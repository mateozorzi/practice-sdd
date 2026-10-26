# Design

## Context

Ver proposal.md (Why) para la motivación. Las 23 issues salen del análisis de SonarCloud del commit `54b529f`,
hecho con `sonar-project.properties` (fuentes `client,server,common,editor`, `build/compile_commands.json`,
cobertura `build/coverage.xml` de `make test`, `cpp:S2245` ignorada). Esa misma configuración, hoy sin
commitear junto con el target `coverage` del `Makefile`, es la que tiene que usar el tercer análisis.

Restricciones que dan forma al enfoque:
- No hay comportamiento nuevo, así que no hay scenarios nuevos ni tests de aceptación que deban fallar primero. La
  red de seguridad son los tests existentes. Cada arreglo se apoya en un test que ya pasa (tabla de abajo).
- No se agrega API para avanzar el tiempo ni inyección de config: ningún arreglo toca la física, los ticks ni los
  valores de `server/config/*.yaml`.
- Los `main` de los tests silencian `std::cerr`, así que cambiar cómo se loguea en los destructores no rompe la
  salida de los tests. Ningún test captura `EXCEPTION_IN_DESTRUCTOR` ni "Stopping.".

| Regla | Archivos | Test que lo cubre |
|-------|----------|-------------------|
| S5404 | `common/queue.h` | `QueueTest.*` en `tests/test_common.cpp` (`ClosedQueueRejectsPushesAndDrainsBeforeFailing`, `PushWaitsUntilThereIsRoom`, `PopWaitsForAPush`, `CloseWakesUpABlockedPop`, `PointerSpecializationBehavesTheSame`) |
| S1048 | 10 destructores | Todos los tests que crean y destruyen servidor, lobby, partidas y clientes (`tests/test_server*.cpp`, `test_lobby.cpp`, `test_game_loop.cpp`, `test_game_monitor.cpp`, `tests/client/test_client_threads.cpp`) |
| S5018 | `common/socket.*`, `common/resolver.*` | `SocketTest.MovedSocketKeepsTheConnection` y todos los tests de red |
| S998 | `common/thread.h` | `ThreadTest.*` y todos los hilos del servidor y el cliente |
| S5020 | `client/sdl_manager.cpp` | `SDLManager` en `tests/client/test_sdl_client.cpp` (el constructor elige la sala de espera) |
| S5020 | `client/animations.cpp` | Test nuevo que fija el comportamiento actual (ver D5) |
| S5502 | `client/Lobby/joinlobby.cpp` | `tests/client/test_lobby_windows.cpp` (tabla `lobbytable`, botón Join en la columna 4) y `tests/client/test_menus.cpp` |

## Goals / Non-Goals

**Goals:**
- Cerrar cada issue de la forma que Sonar reconoce como resuelta, no solo silenciarla. No se usan `// NOSONAR`
  ni exclusiones.
- Que cada regla quede en un commit atómico y se pueda revertir sola.

**Non-Goals:**
- Modernizar el código de la cátedra más allá de la línea marcada (por ejemplo, cambiar `queue.h` a un
  `std::jthread`/`stop_token`, o unificar la plantilla con la especialización `void*`).
- Inyectar el RNG del cliente para tests deterministas: no hay test que dependa de los valores del temblor ni de
  la sala de espera.

## Decisions

### D1. `queue.h`: predicado equivalente al `while`, sin `closed` en el push

- `push`: `is_not_full.wait(lck, [this] { return q.size() < this->max_size; });`
- `pop`: `is_not_empty.wait(lck, [this] { return !q.empty() || closed; });` y después
  `if (q.empty()) throw ClosedQueue();`

El `pop` actual chequea `closed` dentro del loop solo cuando la cola está vacía. La versión con predicado sale
del `wait` cuando hay elementos o cuando está cerrada, y lanza solo si está vacía. Así sigue drenando los elementos
antes de fallar, y como `close()` hace `is_not_empty.notify_all()`, sigue despertando al pop bloqueado. Se aplica
igual en la plantilla y en `Queue<void*>`.

*Alternativa descartada:* el predicado `q.size() < max_size || closed` que sugiere el informe. Para que tenga
efecto, `close()` también tendría que notificar `is_not_full`, y el push bloqueado pasaría a lanzar `ClosedQueue`.
Eso es un cambio de comportamiento (fuera de alcance, ver proposal).

### D2. Destructores: helper de log `noexcept` y `catch (...)`

Sonar marca el `std::cerr << ...` que está **dentro** del `catch`. Un `catch (...)` agregado al mismo `try` no lo
cubre, porque un handler no atrapa lo que lanza otro handler del mismo `try`. Por eso:

- Header nuevo `common/destructor_log.h` con dos funciones `inline ... noexcept`, cada una con su `std::cerr`
  dentro de su propio `try { ... } catch (...) {}`:
  - `log_stopping(std::string_view component)`: el `"[" << component << "] Stopping."` que hoy escriben los 9
    destructores del servidor antes del `try`.
  - `log_destructor_error(std::string_view component, std::string_view what)`: el mismo mensaje de error que hoy
    (`"[" << component << "] " << EXCEPTION_IN_DESTRUCTOR << ": " << what`).

  Se lista en `common/CMakeLists.txt`, junto a `thread.h`.
- Cada destructor queda:
  ```cpp
  log_stopping(X_NAME);  // solo los del servidor, como hoy
  try {
      X::stop();  // lo que haga hoy: stop/join/close
  } catch (const std::exception& e) {
      log_destructor_error(X_NAME, e.what());
  } catch (...) {
      log_destructor_error(X_NAME, "<unknown>");
  }
  ```
- El "Stopping." **no** pasa adentro del `try`: si el stream lanzara ahí, se saltearía el `stop()`. Con
  `log_stopping` `noexcept` el log no puede escapar del destructor y el `stop()` corre siempre.
- Se mantiene el texto actual de cada destructor. Hoy algunos ponen `": "` antes del `what()` y `Lobby`/`Game` no.
  Se unifica con `": "`, que es lo que usan 8 de los 10 (solo cambia la salida de `std::cerr`, que ningún test lee).

*Alternativas descartadas:* un `try` anidado alrededor de cada `std::cerr` (repite el mismo bloque 10 veces), o
marcar los destructores como `noexcept(false)` / usar `// NOSONAR` (no arregla nada y empeora la semántica).

### D3. `Socket` y `Resolver`: `noexcept` en los move

Se agrega `noexcept` en la declaración (`socket.h`, `resolver.h`) y en la definición (`socket.cpp`,
`resolver.cpp`) de los move constructor y move assignment. Solo copian enteros y punteros y llaman funciones de C
(`::shutdown`, `::close`, `freeaddrinfo`) que no lanzan, así que la promesa es verdadera. De paso, los contenedores
estándar pasan a mover en vez de copiar/fallar al crecer, sin efecto observable porque los tipos no son copiables.

### D4. `Thread::main()` → `Thread::thread_main()` privado

Se renombra y se mueve a la sección `private`, porque solo lo usa `start()` (`std::thread(&Thread::thread_main, this)`).
No hay otras referencias en el repo (solo `common/thread.h`). Las subclases siguen implementando `run()`.

### D5. `<random>` en el cliente con un engine por objeto

- `Animations` tiene un miembro `std::mt19937 rng{std::random_device{}()}`. `get_camera_shake()` usa
  `std::uniform_int_distribution<int>(-magnitude, magnitude)`, que da el mismo rango que
  `(rand() % (2m+1)) - m`, sin el sesgo del módulo.
- `SDLManager`: en el constructor, un engine local sembrado con `std::random_device` y
  `std::uniform_int_distribution<size_t>(0, WAITING_ROOMS.size() - 1)`, como en `Sounds::play_background_music`
  (`client/sounds.cpp:116`).

Como hoy no hay test del temblor de cámara, antes del refactor se agrega en `tests/client/test_sdl_client.cpp` un
test que fija el comportamiento actual (`AnimationsTest.CameraShakeStaysWithinItsMagnitude`): sin temblor activo
devuelve `(0, 0)`, y con temblor activo, en muchas muestras, `dx` y `dy` quedan en `[-3, 3]`. Pasa con
`std::rand()` y tiene que seguir pasando con `<random>`.

*Alternativa descartada:* un engine `static`/global compartido. Introduce estado global y, con hilos, una carrera
de datos. Un engine por objeto alcanza porque cada objeto lo usa desde un solo hilo (el de render).

### D6. `joinlobby.cpp`: un `new` por sentencia

Se reemplaza el arreglo con 4 `new` por un arreglo de 4 `QString` (id, nombre, jugadores activos, máximo) y un
loop por columna que crea el `QTableWidgetItem` en su propia sentencia, lo centra y hace `setItem(i, col, item)`.
El ownership pasa a la tabla igual que hoy y el contenido y la alineación de las celdas no cambian.

### D7. Tercer informe de Sonar con la misma configuración

- Antes de analizar, se commitea la configuración que usaron los dos análisis anteriores (`sonar-project.properties`
  y el target `coverage` del `Makefile`, hoy sin commitear), sin modificarla.
- Se corre `make test` (genera `build/coverage.xml` y `build/compile_commands.json`) y `sonar-scanner`
  (`/opt/sonar-scanner`) sobre el commit final, publicado en el proyecto `mateozorzi_practice-sdd` como hizo el
  análisis post-SDD.
- Los datos salen de la API de SonarCloud (measures, qualitygates, issues, hotspots), igual que en los informes
  anteriores. Se generan dos PDFs nuevos en `doc/sonar/`, con el mismo formato y en español:
  - `Resumen_SonarCloud_practice-sdd_postRemediacion.pdf`: resumen del tercer análisis, con la misma estructura que
    `Resumen_SonarCloud_practice-sdd_postSDD.pdf`.
  - `Comparacion_SonarCloud_practice-sdd_pre_post_remediacion.pdf`: comparación de los tres análisis (pre-SDD
    `8ae6d5d`, post-SDD `54b529f`, post-remediación), con ratings, issues por regla, métricas y el balance de issues
    arregladas e introducidas en cada tramo.
- `Comparacion_SonarCloud_practice-sdd_pre_vs_postSDD.pdf`, `Resumen_SonarCloud_practice-sdd_postSDD.pdf` y
  `Resumen_SonarCloud_practice-sdd_preSDD_8ae6d5d.pdf` no se tocan.

## Risks / Trade-offs

- [El predicado del `pop` cambia sutilmente el orden de chequeos] → D1 conserva exactamente cuándo se lanza
  (cola vacía y cerrada). `ClosedQueueRejectsPushesAndDrainsBeforeFailing` y `CloseWakesUpABlockedPop` lo
  verifican, y los tests de red ejercitan las colas del sender/receiver.
- [Sonar sigue marcando S1048 si analiza el helper como una llamada que lanza] → El helper se declara `noexcept`.
  Si el tercer análisis todavía lo marca, se revisa antes de generar el informe, sin recurrir a `NOSONAR`.
- [`noexcept` en los move hace que una excepción futura llame a `std::terminate`] → Las implementaciones actuales
  no lanzan. Si se agrega algo que lance, el `noexcept` lo deja en evidencia en lugar de esconderlo.
- [Los números del tercer análisis no son comparables] → D7 congela la configuración, y el informe aclara cualquier
  diferencia de líneas o archivos que venga del propio change (por ejemplo, el header nuevo).
- [Los hooks de commit (linters + tests) tardan con un commit por regla] → Es el costo de la atomicidad. No se
  saltean.
