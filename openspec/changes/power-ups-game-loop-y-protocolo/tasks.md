# Tasks

## 1. Tests de aceptación en rojo

- [x] 1.1 Agregar `make_fast_power_up_game_config()` a `tests/server_test_utils.*` (design D7: config rápida + power-ups
      2.0 / 1 / 30 / 1 / 2 del Background + `timeout_seconds_race` 60) y el parámetro opcional de factory de generador a `RunningServer`
      (design D6, declarado sin incluir `server/power_up.h`); verificar que `nfs_tests` y `nfs_client_tests` compilan
- [x] 1.2 Escribir en `tests/test_common.cpp` la suite `PowerUpProtocolTest` con un `ISocket` en memoria y los tests
      `PowerUpDTORoundTrip` (3.1) y `SnapshotEffectRoundTrip` (3.2) (design D1, D2, D7)
- [x] 1.3 Escribir en `tests/test_game_loop.cpp` el fixture `GameLoopPowerUpTest` (con `tick(n)` sobre
      `GameLoop::tick()`, design D3, D7) y los tests `SnapshotHasTwoActiveNitroPowerUps` (3.3),
      `CollectedPowerUpShowsInactiveAndNitroEffect` (3.4), `NitroExpiresAtTick200` (3.5),
      `NextRaceStartsWithActivePowerUpsAndNoEffects` (3.6), `InfiniteHealthSurvivesShieldExpiryInLoop` (3.7) y su
      control `CrashIntoWallWithoutCheatTakesDamage` (choque contra la pared x 1968 de
      `on_ground_collidables_liberty_city.json`, design D7)
- [x] 1.4 Escribir en `tests/test_server_integration.cpp` el fixture `ServerIntegrationPowerUpTest` (design D7: su
      propio `RunningServer` en un puerto de `next_port`, la config del helper, la factory "siempre Nitro" y
      `make_test_race(4, 1)`) y el test `ClientReceivesCollectedPowerUpInSnapshot` (3.8)
- [x] 1.5 Agregar solo los stubs mínimos para compilar: `common/power_up_dto.*` (en `common/CMakeLists.txt`), campos
      `power_ups` / `effects` y `add_power_up` / `add_effect` en `WorldSnapshotDTO` sin serializar,
      `GameLoop::tick()` vacío, parámetro de generador en `GameLoop` / `Game` / `Lobby` sin uso; compilar con
      `cmake --build build --target nfs_tests nfs_client_tests -j16` y verificar con
      `./build/nfs_tests --gtest_filter='PowerUpProtocolTest.*:GameLoopPowerUpTest.*:ServerIntegrationPowerUpTest.*'`
      que **todos** los tests nuevos fallan y que los existentes siguen en verde

## 2. DTOs y protocolo

- [x] 2.1 Implementar `PowerUpDTO` / `PowerUpEffectDTO` y la serialización de power-ups y efectos después de los NPCs
      en `ServerProtocol::send_world_snapshot` y `ClientProtocol::receive_update_positions` (design D1, D2);
      verificar que pasan 3.1 y 3.2
- [x] 2.2 Actualizar `ServerProtocolTest.SendWorldSnapshot` (`tests/test_server.cpp`, +2 bytes de contadores) y
      `ClientProtocolTest.ReceiveUpdatePositions_WithMock` (`tests/test_client.cpp`, `0x00, 0x00` al final) al
      formato nuevo; verificar que pasan junto con el resto de `ServerProtocolTest.*` y `ClientProtocolTest.*`

## 3. Tick explícito del game loop

- [x] 3.1 Implementar `GameLoop::tick()` moviendo el cuerpo del `while` de `run()` y sacando `fixed_sleep` de los
      `handle_*_phase` (`run()` = `tick()` + `fixed_sleep`), y agregar `tick_seconds = 1.0 / game_fps` (design D3);
      verificar que todos los `GameLoopTest.*`, `GameTest.*` y `ServerIntegrationTest.*` existentes siguen en verde

## 4. Power-ups en la carrera

- [x] 4.1 Agregar el `PowerUpManager` a `GameLoop` con el generador inyectado, construir el listener con el manager,
      cargar los power-ups en `set_race` y sumar los power-ups al snapshot (design D5, D6, D4 paso 5); verificar que
      pasa 3.3
- [x] 4.2 Reordenar el tick de `IN_RACE` (avance de efectos y cooldowns con `tick_seconds` → `Step` → snapshot) y
      sumar los efectos de Nitro y Escudo de cada jugador al snapshot (design D4); verificar que pasan 3.4, 3.5,
      3.7 y `CrashIntoWallWithoutCheatTakesDamage`, y que los `GameLoopTest.*` existentes siguen en verde
- [x] 4.3 Llamar a `power_ups.clear()` y a `clear_effects()` en todos los jugadores al salir de `IN_RACE` (también
      por timeout; `Player::clear_effects()` pasa a ser público) y hacer que `Player::finish_race()` borre los efectos
      (design D5); verificar que pasan 3.6, `PowerUpEffectsTest.FinishRaceClearsEffects` y
      `PowerUpEffectsTest.ClearEffectsRemovesEffectsAndRestoresValues`, y que los `PowerUpEffectsTest.*` /
      `PlayerTest.*` siguen en verde

## 5. Generador desde el lobby

- [x] 5.1 Pasar la factory de generador `Lobby` → `Game` → `GameLoop` (default: generador real) y conectarla en
      `RunningServer` (design D6); verificar que pasa 3.8 y que los `LobbyTest.*` y `ServerIntegrationTest.*`
      existentes siguen en verde

## 6. Gate del Hito 3

- [x] 6.1 Refactor final y hooks: formato, cpplint y cppcheck sin errores sobre los archivos tocados; verificar que
      ningún test nuevo lee valores de power-ups de `server/config/*.yaml` (solo `make_fast_power_up_game_config()`, que también
      fija el timeout de carrera, y los generadores de secuencia) y que `git diff --stat HEAD -- editor/` está vacío (este change no toca `editor/`)
- [x] 6.2 Correr `make run-tests` en verde, sin regresiones, y chequear el gate del Hito 3: (a) los 8 tests de
      3.1 – 3.8 existen y pasan, (b) `make run-tests` en verde, (c) hooks sin errores, (d) `/opsx:verify` sin CRITICAL

## Workflow follow-up

- `/opsx:verify` y después `/opsx:archive`, que actualiza el estado del Hito 3 en `openspec/constitution/roadmap.md`.
- Al archivar: documentar el formato nuevo del snapshot (power-ups y efectos) en README / manuales, porque cambia el
  protocolo.
- Prueba manual: correr una carrera y confirmar que agarrar un Nitro acelera el auto (los power-ups todavía no se
  dibujan; eso es el Hito 4).
