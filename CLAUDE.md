# Need4Speed

Juego 2D de carreras (TP Taller de Programación) en C++20 con CMake, Box2D, Qt, SDL2 y YAML.
- `server/`: lobby, partidas y física. `client/`: menús en Qt y juego en SDL2. `editor/EditorMapas/`: editor de circuitos (Qt). `common/`: código compartido.
- El programa instalado lee la configuración de `/etc/nfs` y los assets de `/var/nfs`.

## Comandos
- Compilar y testear: `make run-tests`
- Solo tests: `cmake --build build --target nfs_tests nfs_client_tests -j16`
- Tests + cobertura: `make test` (corre `nfs_tests` y `nfs_client_tests`, imprime el resumen y genera `build/report/index.html`)

## Tests
- `nfs_tests` (`tests/`): servidor, `common/` y protocolos. `nfs_client_tests` (`tests/client/`): cliente y editor.
- Nunca incluir headers del cliente y del servidor en la misma unidad de compilación: chocan `Receiver`, `Sender`, `enum class Car` y las macros `HORIZONTAL`/`VERTICAL` del editor. En los tests del cliente, el servidor se usa vía `RunningServer` (`tests/server_test_utils.h`).
- Utilidades: `make_test_server_config()` / `make_fast_game_config()`; para Qt, `tests/client/qt_test_utils.h`.
- SDL corre offscreen (no hace falta Xvfb). Cada archivo de tests de red usa su propio rango de puertos.
- Los `main` de los tests (`tests/main.cpp`, `tests/client/main.cpp`) silencian `std::cerr` del servidor y los warnings de Qt/FFmpeg. Para chequear un mensaje, capturalo en el test (ver `ThreadTest.ExceptionsInRunEndTheThread`).
