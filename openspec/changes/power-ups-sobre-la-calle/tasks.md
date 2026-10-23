# Tasks

## 1. Tests de aceptación en rojo

- [x] 1.1 Copiar las 6 carreras de `server/races/` a `tests/data/races/` (sin cambios) y verificar con
      `diff -r server/races tests/data/races` que son idénticas
- [x] 1.2 Agregar `tests/streets_test_utils.{h,cpp}` con `make_straight_street_map()`, `make_l_street_map()`,
      `make_two_streets_map()` y `make_test_crossing_map()` (celdas de 10 px, cruce del Background, design D9) y
      registrarlo en `tests/CMakeLists.txt`; verificar que `nfs_tests` compila
- [x] 1.3 Escribir `tests/test_streets_grid.cpp` con `StreetsGridTest.LoadsCellSizeAndStreetCells` (5.1),
      `MalformedJsonThrows`, `HeightMismatchThrows`, `WidthMismatchThrows` (5.2) (JSON temporales escritos por el
      test), `StreetPathTest.OverBridgeGoesStraightOnTop` (5.6), `UnderBridgeCrossesBelowTheRails` (5.7),
      `FromTopToBottomTakesTheDetour`, `FromBottomToTopTakesTheDetour` (5.8), `StartsFromNearestStreetCell` (5.9),
      `StreetsGridInstallTest.ConfigPointsToTheCityGrids` y `GridsAreInstalled` (5.13, `GTEST_SKIP` si no existe
      `/var/nfs/world`); registrarlo en `tests/CMakeLists.txt`
- [x] 1.4 Agregar en `tests/test_power_up.cpp` `StreetPowerUpTest.StraightStreetKeepsMidpoint` (5.4),
      `LShapedStreetUsesPathMidpoint` (5.5), `NoPathFallsBackToMidpointAndReportsStretch` (5.10, captura
      `std::cerr`), `ArbitraryCheckpointsNeverFail` (5.12) y `RealRacesPowerUpTest.AllPowerUpsLandOnStreets` (5.11,
      `RaceLoader` sobre `tests/data/races/` y rutas de `make_test_server_config()`, que informa carrera y tramo de
      cada falla), con la config `make_power_up_test_server_config()` y el generador de secuencia del Hito 2;
      `PowerUpManagerTest.PowerUpSpawnsAtMidpointOfCheckpoints` (2.3 / 5.3) queda sin cambios
- [x] 1.5 Escribir `tests/client/test_streets_generator.cpp` con `StreetsGeneratorTest.ClassifiesStreetPixels`
      (`QImage` de 30 × 20 armada en el test) y registrarlo en `nfs_client_tests`
- [x] 1.6 Agregar solo las declaraciones mínimas para compilar: `server/streets_grid.{h,cpp}` (D2 – D5 como stubs que
      devuelven vacío o `nullopt`), el overload `PowerUpManager::load(checkpoints, const StreetMap*)` que ignora la
      grilla, `CityLoaderConfig::streets_file`, `server/tools/streets/streets_grid_generator.{h,cpp}` con stubs, y los
      `.cpp` en `server/CMakeLists.txt` / `tests/CMakeLists.txt`; compilar con
      `cmake --build build --target nfs_tests nfs_client_tests -j16` y verificar con
      `./build/nfs_tests --gtest_filter='StreetsGridTest.*:StreetPathTest.*:StreetPowerUpTest.*:RealRacesPowerUpTest.*:StreetsGridInstallTest.*'`
      y `./build/nfs_client_tests --gtest_filter='StreetsGeneratorTest.*'` que **todos** los tests nuevos fallan
      (salvo 5.4, que coincide con (P + Q) / 2: anotar que pasa por el respaldo y que la verdadera verificación es
      con la grilla, 2.3 que no cambia, y `GridsAreInstalled` si se saltea) y que los existentes siguen en verde

## 2. Grilla de calles

- [x] 2.1 Implementar `StreetsGrid` (constructor desde filas, `is_street`, `cell_center`, convención de centros de
      D1) y `load_streets_grid` con `WorldCreationException` que nombra la ruta y el problema (D2); verificar que
      pasan `StreetsGridTest.*` (5.1, 5.2)
- [x] 2.2 Implementar `nearest_street_cell` (distancia euclídea al centro, desempate fila/columna, puntos fuera de la
      grilla) (D2); verificar con un test de unidad en `tests/test_streets_grid.cpp` que un punto fuera de la grilla
      va a la celda de calle más cercana

## 3. Camino con capas

- [x] 3.1 Implementar `BridgeLayout` (pertenencia por centro a sensores y puentes, por cuadrado a barandas, D3) y
      `find_street_path` con el BFS sobre `(celda, capa, rumbo)`: vecinos, transiciones de capa del listener y
      barandas para `ON_BRIDGE` (D4); verificar que pasan `StreetPathTest.OverBridgeGoesStraightOnTop` (5.6),
      `UnderBridgeCrossesBelowTheRails` (5.7) y `StartsFromNearestStreetCell` (5.9, sin el punto medio)
- [x] 3.2 Agregar la regla de rumbo dentro del rectángulo de un puente y los dos estados iniciales dentro de un cruce
      (D4); verificar que pasan `FromTopToBottomTakesTheDetour` y `FromBottomToTopTakesTheDetour` (5.8) y que 5.6 y
      5.7 siguen en verde
- [x] 3.3 Implementar `path_midpoint` (D5); verificar que pasa `StreetsGridTest`/`StreetPathTest` completo, incluido el
      punto medio (125, 0) de 5.9

## 4. Ubicación de los power-ups

- [x] 4.1 Implementar `PowerUpManager::load(checkpoints, const StreetMap*)`: camino + punto medio, respaldo
      (P + Q) / 2 con el mensaje `Stretch <p> -> <q>` en `std::cerr`, sin lanzar (D6); verificar que pasan
      `StreetPowerUpTest.*` (5.4, 5.5, 5.10, 5.12) y que `PowerUpManagerTest.*` (Hito 2, incluido 2.3 / 5.3) sigue en
      verde

## 5. Grillas reales e integración en el servidor

- [x] 5.1 Implementar `classify_street_cells` y `write_streets_grid_json` (D8); verificar que pasa
      `StreetsGeneratorTest.ClassifiesStreetPixels`
- [x] 5.2 Copiar las 3 máscaras a `server/tools/streets/masks/` (sin cambios en `editor/`), agregar
      `generate_streets_grid.cpp`, el target `nfs_streets_generator` (`EXCLUDE_FROM_ALL`, Qt6::Gui) y `make streets`;
      generar los 3 `server/world/streets/streets_<ciudad>.json` con s = 8 y verificar que cargan con
      `load_streets_grid` (580 × 584 celdas) y que `git status editor/` no muestra cambios
- [x] 5.3 Agregar `streets:` a las 3 ciudades de `server/config/world_config.yaml`, leer la clave opcional en
      `server_config_loader.cpp` y implementar `load_street_map` (D7); verificar que pasa
      `StreetsGridInstallTest.ConfigPointsToTheCityGrids` (5.13) y que `ServerConfigLoaderTest.*` sigue en verde
- [x] 5.4 Hacer pasar `RealRacesPowerUpTest.AllPowerUpsLandOnStreets` (5.11): investigar cada tramo que falle (máscara,
      regla de rumbo, tamaño de celda) y corregirlo según los Risks de design.md, sin relajar el test; medir el tiempo
      de `load` por carrera y anotarlo (si un tramo pasa de ~100 ms, regenerar con s = 16)
- [x] 5.5 Cachear el `StreetMap` por ciudad en `CityLoader` con `get_street_map()` (`nullptr` y log si falta o falla)
      y pasarlo en `GameLoop::set_race` (D7); verificar que `GameLoopPowerUpTest.*`,
      `ServerIntegrationPowerUpTest.*` y `CityLoaderTest.*` siguen en verde (si alguno se rompe por la posición,
      analizar la causa antes de tocar el test, ver Risks)
- [x] 5.6 Instalar el juego con `installer.sh` y verificar que existen los 3 archivos en `/var/nfs/world/streets/`
      y que `StreetsGridInstallTest.GridsAreInstalled` (5.13) pasa sin saltearse

## 6. Gate del Hito 5

- [x] 6.1 Refactor final y hooks: formato, cpplint y cppcheck sin errores sobre los archivos tocados; verificar que
      ningún test nuevo lee valores de juego de `server/config/*.yaml` (5.11 y 5.13 solo usan rutas, excepción
      documentada) y que no hay cambios en `editor/`
- [x] 6.2 Correr `make run-tests` en verde, sin regresiones, y chequear el gate del Hito 5: (a) los tests de
      5.1 – 5.13 existen y pasan, (b) `make run-tests` en verde, (c) hooks sin errores, (d) `/opsx:verify` sin
      CRITICAL

## Workflow follow-up

- `/opsx:verify` y después `/opsx:archive`, que actualiza el estado del Hito 5 en `openspec/constitution/roadmap.md`.
- Al archivar: documentar en README y manuales `make streets` (cómo regenerar las grillas si cambia una máscara) y que
  los power-ups ahora aparecen sobre la calle. El protocolo no cambia.
- Prueba manual (DoD): correr una carrera en cada ciudad (incluida una que cruce un puente) y ver que los power-ups
  aparecen sobre la calle, en el camino entre los checkpoints.
