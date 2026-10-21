# Tech stack

El stack está **cerrado**: viene fijado por la cátedra y el proyecto existente. No se agregan dependencias nuevas ni
se cambian versiones.

## Lenguaje y build
- **C++20** (`CMAKE_CXX_STANDARD 20`), con warnings como errores (`NFS_MAKE_WARNINGS_AS_ERRORS=ON`).
- **CMake ≥ 3.24**; las dependencias se bajan con `FetchContent`. `make` envuelve los targets más comunes.
- Opciones: `NFS_TESTS`, `NFS_CLIENT`, `NFS_SERVER`, `NFS_EDITOR`, `NFS_COVERAGE`.

## Dependencias

| Uso | Librería | Versión |
|-----|----------|---------|
| Física | Box2D | v2.4.1 |
| Render, input y audio del juego | SDL2 / SDL2_image / SDL2_mixer / SDL2_ttf | 2.30.8 / 2.8.2 / 2.8.0 / 2.22.0 |
| Wrapper C++ de SDL | libSDL2pp | commit `cc198c9` |
| Menús del cliente y editor | Qt6 (Core, Widgets, Multimedia, Test) | sistema |
| Configuración | yaml-cpp | 0.8.0 |
| JSON | nlohmann/json | v3.11.3 |
| Tests | GoogleTest | commit `03597a0` |
| Cobertura | gcovr | sistema o local (`make gcovr`) |
| Socket, thread y queue | código de la cátedra ([eldipa](https://github.com/eldipa)) | GPL v2 |

## Arquitectura
- **`server/`**: acceptor, lobby, partidas, game loop y física Box2D. Es **autoritativo**: toda la lógica de juego
  (incluidos los efectos de los power-ups) se calcula en el servidor.
- **`client/`**: menús en Qt (conexión, lobby, elección de auto) y juego en SDL2 (render, HUD, sonido, input). Recibe
  snapshots del servidor y solo dibuja y reproduce sonido.
- **`editor/EditorMapas/`**: editor de circuitos en Qt.
- **`common/`**: DTOs, protocolo común, socket, thread, queue (lib `nfs_common`).
- Configuración del juego en YAML (`server/config/game_config.yaml`, `physics_config.yaml`, `world_config.yaml`).
- Instalado: la configuración se lee de `/etc/nfs` y los assets de `/var/nfs`.

## Calidad
- **Formato y lint:** clang-format, cpplint (`CPPLINT.cfg`, `linelength=100`) y cppcheck.
- **Hooks de Claude Code** (`.claude/settings.json`): al editar un archivo → formato + cppcheck; al commitear →
  linters + tests. Si un hook falla, se arregla el problema, no se saltea el hook.
- **Análisis estático:** SonarCloud (`sonar-project.properties`, usa `build/compile_commands.json`).

## Tests
- `nfs_tests` (`tests/`): servidor, `common/` y protocolos. `nfs_client_tests` (`tests/client/`): cliente y editor.
- Comandos:
  - `make run-tests`: compila y corre todo.
  - `cmake --build build --target nfs_tests nfs_client_tests -j16`: compila solo los tests.
  - `make test`: corre los tests con cobertura y genera `build/report/index.html`.
- Reglas:
  - Nunca incluir headers del cliente y del servidor en la misma unidad de compilación: chocan `Receiver`, `Sender`,
    `enum class Car` y las macros `HORIZONTAL`/`VERTICAL` del editor. En los tests del cliente, el servidor se usa vía
    `RunningServer` (`tests/server_test_utils.h`).
  - Cada archivo de tests de red usa su propio rango de puertos.
  - SDL corre offscreen (no hace falta Xvfb); para Qt, `QT_QPA_PLATFORM=offscreen` y `tests/client/qt_test_utils.h`.
  - Utilidades: `make_test_server_config()` / `make_fast_game_config()`.
  - Los `main` de los tests silencian `std::cerr` y los warnings de Qt/FFmpeg; para verificar un mensaje, el test lo
    captura.

## Plataforma
- Linux: instalación con `installer.sh`, rutas `/etc/nfs` y `/var/nfs`.
