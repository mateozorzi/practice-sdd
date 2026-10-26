# Tasks

## 1. Línea base y configuración del análisis

- [x] 1.1 Correr `make run-tests` sin tocar código y anotar que `nfs_tests` y `nfs_client_tests` pasan en verde (es la línea base del refactor)
- [x] 1.2 Commitear tal como están `sonar-project.properties`, el target `coverage` del `Makefile` y `doc/sonar/` (la configuración de los dos análisis anteriores, sin modificarla); verificar con `git status` que no quedan cambios de configuración sin commitear

## 2. cpp:S5404 – `wait()` con predicado en `common/queue.h`

- [ ] 2.1 Reemplazar los dos `while` + `wait()` de la plantilla `Queue<T>` por la sobrecarga con predicado según D1 (push sin `closed`; pop con `!q.empty() || closed` y `throw ClosedQueue()` si queda vacía); verificar que pasan `QueueTest.*` en `tests/test_common.cpp`
- [ ] 2.2 Aplicar el mismo cambio a la especialización `Queue<void*>`; verificar `QueueTest.PointerSpecializationBehavesTheSame` y `make run-tests` en verde, y commitear

## 3. cpp:S1048 – destructores que no propagan excepciones

- [ ] 3.1 Crear `common/destructor_log.h` con `log_destructor_error(component, what) noexcept` según D2 y agregarlo a `common/CMakeLists.txt`; verificar que `nfs_common` compila sin warnings
- [ ] 3.2 Pasar los destructores de `server/lobby.cpp` (con el "Stopping." adentro del `try`), `game.cpp`, `client_handler.cpp`, `game_loop.cpp` y `game_monitor.cpp` a `catch (const std::exception&)` + `catch (...)` con el helper; verificar `test_lobby`, `test_game_loop`, `test_game_monitor` y `test_server*` en verde
- [ ] 3.3 Hacer lo mismo en `server/receiver.cpp`, `sender.cpp`, `acceptor.cpp`, `server.cpp` y `client/receiver.cpp`; verificar que `grep -rn "EXCEPTION_IN_DESTRUCTOR" server client` solo queda en el helper o en sus llamadas, `make run-tests` en verde, y commitear

## 4. cpp:S5018 y cpp:S998 – `noexcept` en los move y renombrar `Thread::main()`

- [ ] 4.1 Agregar `noexcept` a la declaración y la definición de los move constructor y move assignment de `Socket` (`common/socket.{h,cpp}`) y `Resolver` (`common/resolver.{h,cpp}`) según D3; verificar `SocketTest.*` en verde y commitear
- [ ] 4.2 Renombrar `Thread::main()` a `thread_main()`, moverlo a `private` y actualizar `start()` según D4; verificar que `grep -rn "Thread::main\|::main()" common server client` no encuentra nada, `ThreadTest.*` y `make run-tests` en verde, y commitear

## 5. cpp:S5020 – `<random>` en el cliente

- [ ] 5.1 Agregar `AnimationsTest.CameraShakeStaysWithinItsMagnitude` en `tests/client/test_sdl_client.cpp` según D5 (sin temblor activo devuelve `(0, 0)`; con temblor activo, en muchas muestras, `dx` y `dy` quedan en `[-3, 3]`); verificar que **pasa con el código actual** (`std::rand()`), porque fija el comportamiento existente
- [ ] 5.2 Reemplazar `std::rand()` en `Animations::get_camera_shake()` por un miembro `std::mt19937` y `std::uniform_int_distribution` según D5; verificar que `AnimationsTest.CameraShakeStaysWithinItsMagnitude` sigue en verde
- [ ] 5.3 Reemplazar `std::rand()` en el constructor de `SDLManager` (elección de la sala de espera) según D5; verificar que `grep -rn "std::rand\|srand" client server common editor` no encuentra nada, los tests de `SDLManager` en `tests/client/test_sdl_client.cpp` y `make run-tests` en verde, y commitear

## 6. cpp:S5502 – un `new` por sentencia en `joinlobby.cpp`

- [ ] 6.1 Reemplazar el arreglo de 4 `new QTableWidgetItem` en `client/Lobby/joinlobby.cpp` por un loop que crea cada ítem en su propia sentencia según D6; verificar `tests/client/test_lobby_windows.cpp` y `tests/client/test_menus.cpp` (tabla `lobbytable` y botón Join) en verde y commitear

## 7. Verificación integral

- [ ] 7.1 Correr `make run-tests` en verde sin regresiones respecto de la línea base de 1.1, y los hooks (formato, cpplint, cppcheck) sin errores
- [ ] 7.2 Correr `make test` y verificar que se generan `build/coverage.xml` y `build/report/index.html`, con cobertura total no menor que la del análisis post-SDD (90,2 % en Sonar; comparar el resumen de gcovr antes y después)
- [ ] 7.3 Correr `/opsx:verify` sobre el change y verificar que no hay issues CRITICAL

## 8. Tercer informe de Sonar y comparación de los tres análisis

- [ ] 8.1 Correr `sonar-scanner` sobre el commit final con el `sonar-project.properties` de 1.2, sin cambios, y publicar en `mateozorzi_practice-sdd` según D7; verificar en la API de SonarCloud que el análisis corresponde a ese commit y que las issues de S5404, S1048, S5018, S998, S5020 y S5502 están en 0 (si queda alguna, volver al grupo de su regla antes de seguir)
- [ ] 8.2 Generar `doc/sonar/Resumen_SonarCloud_practice-sdd_postRemediacion.pdf` con datos de la API de SonarCloud, con la misma estructura que `Resumen_SonarCloud_practice-sdd_postSDD.pdf` (resumen ejecutivo, métricas, ratings, Quality Gate, issues por regla, recomendaciones); verificar que el PDF abre y que sus números coinciden con la API
- [ ] 8.3 Generar `doc/sonar/Comparacion_SonarCloud_practice-sdd_pre_post_remediacion.pdf` comparando pre-SDD (`8ae6d5d`), post-SDD (`54b529f`) y post-remediación: ratings, Quality Gate, issues por regla, métricas generales y balance de issues arregladas e introducidas en cada tramo; verificar que el PDF abre y que las columnas pre y post-SDD coinciden con los informes existentes
- [ ] 8.4 Verificar con `git status` que los tres PDFs existentes de `doc/sonar/` siguen sin cambios y commitear solo los dos PDFs nuevos
- [ ] 8.5 Correr `make run-tests` en verde por última vez y chequear el cierre: todos los tests pasan, hooks sin errores, `/opsx:verify` sin CRITICAL y el tercer análisis con 0 issues de las seis reglas

## Workflow follow-up

- Archivar el change con `/opsx:archive` cuando el informe esté revisado.
