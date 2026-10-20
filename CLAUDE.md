# Need4Speed (TP Taller de Programación)

Juego 2D de carreras en C++20 con tres programas que comparten `common/`:
- `server/`: lobby, partidas y física (Box2D), configuración en YAML (`server/config`).
- `client/`: menús en Qt (`client/Lobby/`, `menu_*.cpp`) y juego en SDL2 (`game*.cpp`, `sdl_manager.cpp`).
- `editor/EditorMapas/`: editor de circuitos en Qt.

Las dependencias se descargan con FetchContent (`build/_deps`).
El programa instalado lee de rutas fijas: configuración en `/etc/nfs`, carreras, mundo y assets en `/var/nfs`.

## Comandos

- Compilar y testear: `make run-tests`
- Cobertura: `make coverage GCOVR=<gcovr>`. El reporte queda en `build/report/index.html`.
  - gcovr no está instalado en el sistema: se instala con `pip3 install --target build/tools gcovr`.
- Compilar un solo target: `cmake --build build --target nfs_tests nfs_client_tests -j16`
- El código de la cátedra (`common/{queue,liberror,resolver,resolvererror,socket,thread}`) está excluido de los linters.

## Tests (dos ejecutables)

- **`nfs_tests`** (`tests/*.cpp`): servidor, `common/` y protocolos.
- **`nfs_client_tests`** (`tests/client/`): cliente (Qt y SDL) y editor.

Están separados porque el cliente y el servidor definen clases globales con el mismo nombre (`Receiver`, `Sender`) y el `upgrade_shop.h` del cliente declara `enum class Car`. Por lo mismo, no se pueden incluir headers del cliente y del servidor en la misma unidad de compilación. `RunningServer` (`tests/server_test_utils.h`) oculta el servidor detrás de un pimpl.

Además, el `Constants.h` del editor define las macros `HORIZONTAL`/`VERTICAL`, que chocan con `common/constants.h`.

### Utilidades

- `make_test_server_config()` / `make_fast_game_config()`: redirigen `/var/nfs` a los archivos del repositorio (`NFS_SOURCE_DIR`); la versión rápida pone las esperas en 0.
- Qt (`tests/client/qt_test_utils.h`):
  - `QtTest` crea una `QApplication` por suite, porque los menús crean la suya.
  - `ModalCloser` cierra los diálogos modales y guarda su texto.
  - `UiRobot`/`Script` recorren menús que bloquean en `QEventLoop` anidados; las acciones se encolan con `QTimer::singleShot(0)`.
- SDL corre con `SDL_VIDEODRIVER=offscreen` y `SDL_AUDIODRIVER=dummy`; no hace falta Xvfb. Los tests de SDL se saltean si no existe `/var/nfs/assets`.
- Los tests de red usan sockets reales en localhost, cada archivo con su rango de puertos (24500, 25500, …, 29500).

### Cuidado al correr y depurar

- SDL convierte SIGTERM/SIGINT en `SDL_QUIT`: para cortar un test colgado hay que usar `timeout -s KILL`.
- gdb no puede adjuntarse a un proceso que ya está corriendo (`ptrace_scope`): hay que lanzarlo con `gdb -batch -ex run ... --args`.
- No usar `pkill -f <patrón>`: mata la propia shell.
- Antes de dar un test por bueno, correrlo muchas veces: hubo condiciones de carrera que solo aparecían en algunas corridas. Para detectarlas sirve compilar aparte con `-fsanitize=address`, usando `FETCHCONTENT_SOURCE_DIR_<DEP>` (no `FETCHCONTENT_BASE_DIR`, que pisa el directorio `build/`).

## Bugs conocidos (tests `DISABLED_` que los documentan)

1. `ClientHandler::stop()` se llama desde el acceptor mientras el thread del handler usa el mismo buffer del protocolo: el servidor puede crashear al apagarse con clientes conectados.
2. Al irse un cliente se cierra dos veces el mismo fd (`CommonProtocol::stop`; `Socket::close` no pone `skt` en -1): puede cortarle la conexión a un cliente nuevo.
3. Con la conexión cerrada, `Socket::recvall`/`sendall` devuelven 0 y `CommonProtocol` lo ignora: el `Receiver` del cliente queda en un loop infinito y el `Sender` nunca se entera de que se cortó.
4. `GameLoop::add_player`/`remove_player` modifican `players` mientras corre el loop de la partida (carrera de datos).
5. `LibError` desborda su buffer con mensajes largos.
6. `Server::run()` queda en un loop infinito si stdin está cerrado.

Ya arreglados:
- `GameLoop::stop()` ya no vacía `players` desde otro thread.
- `~InputManager` cierra la cola del `Sender` en vez de frenarlo, para no perder el `DISCONNECT`.
- `GameLogic` (`client/game_logic.*`) separa la lógica del cliente del dibujo, a través de la interfaz `GameView`.
