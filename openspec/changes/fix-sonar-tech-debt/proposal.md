# Proposal

## Why

El análisis de SonarCloud posterior a los hitos (commit `54b529f`, `doc/sonar/Resumen_SonarCloud_practice-sdd_postSDD.pdf`)
deja **23 issues abiertas**: 14 de confiabilidad (rating **E**, con 4 Blocker en `common/queue.h`) y 9 de
mantenibilidad (53 min de deuda). La comparación con el commit pre-SDD (`8ae6d5d`) muestra que son las mismas 23:
el flujo SDD no introdujo ninguna, pero tampoco arregló ninguna, porque ningún change las tenía en su alcance, ni
siquiera en los archivos que tocó (`game_loop.cpp`, `lobby.cpp`, `socket.h`, `thread.h`, ...). Para que el flujo
baje estos números, la remediación tiene que ser un change propio.

**Hito:** este change no implementa un hito del roadmap de power-ups (`roadmap.md`). Es remediación de deuda
técnica detectada por Sonar después de terminar los hitos, y no agrega funcionalidad al juego.

## What Changes

Refactor sin cambios de comportamiento: los tests existentes (`nfs_tests` y `nfs_client_tests`) tienen que seguir
pasando sin modificarlos.

- **cpp:S5404 (4, Blocker)**: los `wait()` de `common/queue.h` (plantilla y especialización `void*`) pasan a la
  sobrecarga con predicado, con la misma semántica que los `while` actuales.
- **cpp:S1048 (10, High)**: los destructores de `Lobby`, `Game`, `ClientHandler`, `GameLoop`, `GameMonitor`,
  `Receiver`, `Sender`, `Acceptor`, `Server` (servidor) y `Receiver` (cliente) dejan de poder propagar
  excepciones, incluidas las que no heredan de `std::exception` y las que podría lanzar el propio log.
- **cpp:S5018 (4, Blocker)**: los move constructor y move assignment de `Socket` y `Resolver` pasan a ser
  `noexcept`.
- **cpp:S998 (1, Blocker)**: `Thread::main()` se renombra para no confundirse con el punto de entrada global.
- **cpp:S5020 (3, Blocker)**: `std::rand()` en `client/sdl_manager.cpp` y `client/animations.cpp` se reemplaza por
  `<random>`, como ya hacen `client/sounds.cpp` y `server/power_up.cpp`.
- **cpp:S5502 (1, Blocker)**: en `client/Lobby/joinlobby.cpp` cada `QTableWidgetItem` se crea en su propia
  sentencia.
- **Nuevo informe de Sonar**: al terminar, se analiza el resultado con la **misma configuración** que los dos
  análisis anteriores y se generan dos PDFs nuevos en `doc/sonar/`: el resumen del análisis post-remediación y una
  comparación entre los tres análisis (pre-SDD, post-SDD y post-remediación). Los PDFs existentes no se borran ni
  se modifican.

Resultado esperado: 0 issues abiertas, confiabilidad E → A, deuda de mantenibilidad 53 min → 0, y la cobertura sin
bajar respecto de 90,2 %.

**Fuera de alcance:**
- Cualquier funcionalidad o cambio de comportamiento del juego, el servidor, el cliente o el editor.
- Cambios en la configuración del análisis: `sonar-project.properties` y el target de cobertura del `Makefile` se
  usan tal como están, para que el tercer análisis sea comparable con los dos anteriores. En particular, no se
  agrega `sonar.coverage.exclusions` (punto 4 del plan del informe) ni se reactiva `cpp:S2245`.
- Que `Queue::close()` despierte a un `push` bloqueado en una cola llena. Hoy queda bloqueado y el predicado del
  `wait()` mantiene ese comportamiento. Cambiarlo sería un cambio de comportamiento con su propio test.
- Issues de Sonar que aparezcan en `tests/` (no son código fuente del análisis).

## Capabilities

### New Capabilities

Ninguna.

### Modified Capabilities

Ninguna. No cambia ningún requirement de `power-ups`, `race-catalogue` ni `race-editor`: el comportamiento
observable es el mismo. Por eso el change declara `skip_specs: true` en `.openspec.yaml`.

## Impact

- **`common/`**: `queue.h`, `thread.h`, `socket.{h,cpp}`, `resolver.{h,cpp}` y un header nuevo con el helper de log
  para destructores. Es código de la cátedra (eldipa), pero se modifica igual porque es donde están las issues.
- **`server/`**: los `.cpp` de `lobby`, `game`, `client_handler`, `game_loop`, `game_monitor`, `receiver`, `sender`,
  `acceptor` y `server` (solo los destructores).
- **`client/`**: `receiver.cpp` (destructor), `sdl_manager.cpp`, `animations.{h,cpp}` y `Lobby/joinlobby.cpp`.
- **`doc/sonar/`**: dos PDFs nuevos.
- Sin cambios de protocolo, de configuración YAML, de dependencias ni de `editor/`.
- Tests: no se modifica ningún test existente. Cada arreglo queda cubierto por los tests actuales (ver design.md),
  salvo el temblor de cámara de `Animations`, que hoy no tiene test. Para ese caso se agrega un test que fija el
  comportamiento actual: pasa antes del refactor y tiene que seguir pasando después.
