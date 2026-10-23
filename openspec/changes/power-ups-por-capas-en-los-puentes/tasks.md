# Tasks

## 1. Tests de aceptación en rojo

- [x] 1.1 Agregar `make_water_bridge_map()` en `tests/streets_test_utils.{h,cpp}` (rectángulo (100..160, 100..160),
      camino de arriba, sensor `ON_BRIDGE` (40..220, 110..150) y barandas, sin sensor `UNDER_BRIDGE`, design D7) y
      `make_bridge_test_race()` en `tests/server_test_utils.{h,cpp}` (Liberty City, 4 checkpoints, tramo 1 → 2 por
      arriba del puente 0); verificar que `nfs_tests` compila
- [x] 1.2 Escribir en `tests/test_power_up.cpp` `BridgePowerUpTest.OverTheBridgeIsOnBridge` (6.1),
      `UnderTheBridgeIsUnderBridge` (6.2), `WaterBridgeIsOnBridge` (6.3), `OutsideTheBridgeIsOnGround` y
      `WithoutGridIsOnGround` (6.4), `CrossingTheEdgeMovesInside` y `CrossingTheEdgeMovesOutside` (6.5),
      `RespawnKeepsPositionAndLayer` (6.11) y `RealRacesPowerUpTest.BridgePowerUpsHaveThePathLayer` (6.15, informa
      carrera y tramo de cada falla), con `make_power_up_test_server_config()` y el generador de secuencia del Hito 2
- [x] 1.3 Escribir en `tests/test_collision_listener.cpp` el helper `place_bridge_power_up` y
      `CollisionListenerTest.UnderBridgeCarSkipsOnBridgePowerUp` (6.6), `OnBridgeCarCollectsOnBridgePowerUp` (6.7),
      `UnderBridgePowerUpOnlyForUnderBridgeCars` (6.8), `OnGroundPowerUpForAnyLayer` (6.9) y
      `RealSensorsOnlyTheCarOnTopCollects` (6.10, sensores del cruce con `CityUtils::add_sensor` y pasos de física)
- [x] 1.4 Escribir `PowerUpProtocolTest.PowerUpDTOLayerRoundTrip` (6.12) en `tests/test_common.cpp`,
      `GameLoopPowerUpTest.SnapshotHasTheBridgeLayer` (6.13, con `make_bridge_test_race()`, chequeando antes del
      `THEN` que el power-up cae dentro del rectángulo del puente) en `tests/test_game_loop.cpp` y
      `PowerUpRenderTest.OnBridgePowerUpIsDrawnAboveTheBridge` (6.14, JSON de un puente (100..160, 100..160) armado en
      el test) en `tests/client/test_sdl_client.cpp`
- [x] 1.5 Agregar solo las declaraciones mínimas para compilar: `PowerUp::get_layer()` (devuelve `ON_GROUND`),
      `PowerUpDTO::layer` sin serializar, `PathSpot` / `place_on_path` / `path_length` / `path_point_at` como stubs,
      `PowerUpDrawPass` y `power_ups_to_draw(state, pass)` que devuelve todos; compilar con
      `cmake --build build --target nfs_tests nfs_client_tests -j16` y verificar con
      `./build/nfs_tests --gtest_filter='BridgePowerUpTest.*:CollisionListenerTest.*Bridge*:CollisionListenerTest.OnGround*:CollisionListenerTest.RealSensors*:PowerUpProtocolTest.*Layer*:GameLoopPowerUpTest.SnapshotHasTheBridgeLayer:RealRacesPowerUpTest.*'`
      y `./build/nfs_client_tests --gtest_filter='PowerUpRenderTest.*'` que los tests nuevos fallan (anotar los que
      pasan por coincidir con el comportamiento actual: `WithoutGridIsOnGround`, `OutsideTheBridgeIsOnGround`,
      `OnGroundPowerUpForAnyLayer`, `OnBridgeCarCollectsOnBridgePowerUp`) y que los existentes siguen en verde

## 2. Capa y borde del puente al cargar

- [x] 2.1 Implementar `path_length` y `path_point_at`, y reescribir `path_midpoint` con ellos (D2); verificar que
      `StreetsGridTest.*`, `StreetPathTest.*` y `StreetPowerUpTest.*` (Hito 5) siguen en verde
- [x] 2.2 Agregar la capa a `PowerUp` (D1) e implementar `place_on_path` (validez por puente, búsqueda de a 1 px con
      empate hacia Q, capa por `cell_at`, D3) y su uso en `PowerUpManager::load` con el respaldo y el log
      `crosses a bridge edge` (D4); verificar que pasan 6.1 – 6.5 y `RespawnKeepsPositionAndLayer` (6.11), y que
      `PowerUpManagerTest.*` y `StreetPowerUpTest.*` siguen en verde
- [x] 2.3 Hacer pasar `RealRacesPowerUpTest.BridgePowerUpsHaveThePathLayer` (6.15): investigar cada tramo que falle
      y corregirlo según los Risks de design.md, sin relajar el test; verificar que
      `RealRacesPowerUpTest.AllPowerUpsLandOnStreets` (5.11) sigue en verde

## 3. Recolección por capa

- [x] 3.1 Agregar el filtro por capa en `PowerUpManager::try_collect` (D4); verificar que pasan
      `CollisionListenerTest` 6.6 – 6.10 y que el resto de `CollisionListenerTest.*` (Hito 2) sigue en verde

## 4. Protocolo y snapshot

- [x] 4.1 Agregar `layer` a `PowerUpDTO` y serializarla en `server_protocol.cpp` / `client_protocol.cpp` después de
      `collected_by` (valor > 2 → `ON_GROUND`, D5); verificar que pasan `PowerUpProtocolTest.*` (3.1 y 6.12)
- [x] 4.2 Pasar `get_layer()` en `GameLoop::add_power_ups_to_snapshot`; elegir los checkpoints de
      `make_bridge_test_race()` con el puente 0 de Liberty City y verificar que pasa
      `GameLoopPowerUpTest.SnapshotHasTheBridgeLayer` (6.13) y que `GameLoopPowerUpTest.*` y
      `ServerIntegrationPowerUpTest.*` siguen en verde

## 5. Render por capa

- [x] 5.1 Agregar `layer` a `PowerUpSprite`, implementar `power_ups_to_draw(state, pass)` y las dos pasadas en
      `SDLManager::render_players` (`BELOW_BRIDGES` antes de los autos de abajo, `ABOVE_BRIDGES` después de
      `render_bridges`, D6); verificar que pasa `PowerUpRenderTest.OnBridgePowerUpIsDrawnAboveTheBridge` (6.14) y que
      `PowerUpRenderTest.*` y `SdlTest.*` siguen en verde

## 6. Gate del Hito 6

- [x] 6.1 Refactor final y hooks: formato, cpplint y cppcheck sin errores sobre los archivos tocados; verificar que
      ningún test nuevo lee valores de juego de `server/config/*.yaml` (6.13 y 6.15 solo usan rutas de
      `server/world/`) y que `git status editor/` no muestra cambios
- [ ] 6.2 Correr `make run-tests` en verde, sin regresiones, y chequear el gate del Hito 6: (a) los tests de
      6.1 – 6.15 existen y pasan, (b) `make run-tests` en verde, (c) hooks sin errores, (d) `/opsx:verify` sin
      CRITICAL

## Workflow follow-up

- `/opsx:verify` y después `/opsx:archive`, que actualiza el estado del Hito 6 en `openspec/constitution/roadmap.md`
  y los criterios del DoD de la feature.
- Al archivar: el protocolo cambia (capa en el power-up del snapshot), así que actualizar la descripción del protocolo
  en README y manuales, y documentar que los power-ups de los puentes solo los agarra un auto de su capa.
- Prueba manual (DoD): correr una carrera que cruce un puente por arriba y otra por debajo (por ejemplo
  `bridge_tour`) y ver que el power-up de arriba se dibuja sobre el puente y solo lo agarra el auto que pasa por
  arriba.
