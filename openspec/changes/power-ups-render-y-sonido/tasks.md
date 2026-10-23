# Tasks

## 1. Tests de aceptación en rojo

- [x] 1.1 Actualizar los tests de servidor y protocolo por `collected_by` (design D1): `PowerUpProtocolTest.PowerUpDTORoundTrip`
      (`tests/test_common.cpp`, segundo power-up inactivo con recolector 1), `GameLoopPowerUpTest.CollectedPowerUpShowsInactiveAndNitroEffect`
      (recolector 1) y el nuevo `GameLoopPowerUpTest.RespawnedPowerUpHasNoCollector` (`tests/test_game_loop.cpp`, 400
      ticks con `make_fast_power_up_game_config()`); ajustar los bytes esperados de `ServerProtocolTest.SendWorldSnapshot`
      (`tests/test_server.cpp`) y `ClientProtocolTest.ReceiveUpdatePositions_WithMock` (`tests/test_client.cpp`) solo
      si sus snapshots traen power-ups (hoy van vacíos)
- [x] 1.2 Escribir en `tests/client/test_game_logic.cpp` el fixture `PowerUpGameLogicTest` (helpers con el snapshot
      del Background del Hito 4, `FakeView` con contador de `play_power_up_sound`, design D3, D6) y los tests
      `ExpiredEffectRemovesBadge` (4.5), `RenewedEffectShowsFullDuration` (4.6), `RepairShowsNoBadge` (4.7),
      `OtherPlayersEffectsAreNotShown` (4.8), `OwnPickupPlaysUpgradeSoundOnce` (4.9) y
      `OtherPlayerPickupPlaysNoSound` (4.10)
- [x] 1.3 Escribir en `tests/client/test_sdl_client.cpp` `PowerUpHudTest.NitroBadgeShowsCeiledSeconds` (4.3) y
      `PowerUpHudTest.TwoBadgesSideBySide` (4.4) sobre `effect_badges`, `PowerUpRenderTest.DrawsTheThreePowerUps`
      (4.1, incluido `size == CAR_RENDER_SIZE`), `PowerUpRenderTest.InactivePowerUpIsNotDrawn` (4.2) y `PowerUpRenderTest.UnknownTypeIsIgnored` (4.11)
      sobre `power_ups_to_draw` + `SDLManager::render` con el fixture `SdlTest`, y `PowerUpAssetsTest.SpritesAreInstalled`
      (4.12) (design D4, D7)
- [x] 1.4 Agregar solo los stubs mínimos para compilar: `PowerUpDTO::collected_by` sin serializar, `LocalInfo::power_ups`
      / `effects`, `GameView::play_power_up_sound()` (vacío en `Game`), `client/power_up_view.{h,cpp}` con funciones
      que devuelven listas vacías (en `client/CMakeLists.txt` y en las fuentes de `nfs_client_tests` del
      `CMakeLists.txt` raíz) y las constantes de sprites en `client_constants.h`; compilar con
      `cmake --build build --target nfs_tests nfs_client_tests -j16` y verificar con
      `./build/nfs_tests --gtest_filter='PowerUpProtocolTest.*:GameLoopPowerUpTest.*'` y
      `./build/nfs_client_tests --gtest_filter='PowerUpGameLogicTest.*:PowerUpHudTest.*:PowerUpRenderTest.*:PowerUpAssetsTest.*'`
      que **todos** los tests nuevos o modificados fallan (salvo 4.12, que ya pasa porque los sprites están instalados,
      y los que hoy pasan por vacío, como 4.5, 4.7, 4.8 y 4.10: documentar cuáles en el commit) y que los existentes
      siguen en verde

## 2. Quién recogió el power-up (servidor y protocolo)

- [x] 2.1 Guardar el recolector en `PowerUp` (`collect(respawn_cooldown, player_id)`, `respawn` lo pone en 0,
      `get_collected_by()`), pasarlo desde `PowerUpManager::try_collect` y mandarlo en
      `GameLoop::add_power_ups_to_snapshot` (design D1); verificar que pasan 3.4 y `RespawnedPowerUpHasNoCollector` y
      que `PowerUpManagerTest.*`, `CollisionListenerTest.*` y `GameLoopPowerUpTest.*` siguen en verde
- [x] 2.2 Serializar `collected_by` como `uint32` después de `active` en `ServerProtocol::send_world_snapshot` y
      `ClientProtocol::receive_update_positions` (design D1); verificar que pasa 3.1 y que `ServerProtocolTest.*`,
      `ClientProtocolTest.*` y `ServerIntegrationPowerUpTest.*` siguen en verde

## 3. Estado de power-ups y efectos en el cliente

- [x] 3.1 En `GameLogic::update_game_state_from_snapshot`, copiar `power_ups` y los efectos del jugador propio a
      `LocalInfo` (design D2) e implementar `effect_badges` en `client/power_up_view.cpp` (texto `(ms + 999) / 1000`
      + `s`, fila con separación fija, design D4); verificar que pasan 4.3 – 4.8 y que `GameLogicTest.*` sigue en verde

## 4. Sonido al recoger

- [x] 4.1 Detectar el flanco de recolección propia en `update_game_state_from_snapshot` y llamar a
      `view.play_power_up_sound()`; implementarlo en `Game` con `sdl.play_upgrade_sound()` (design D3); verificar que
      pasan 4.9 y 4.10

## 5. Power-ups en la pista y HUD

- [x] 5.1 Implementar `power_up_sprite` y `power_ups_to_draw` (activos con tipo conocido, tamaño cuadrado
      `POWER_UP_RENDER_SIZE = CAR_RENDER_SIZE`, design D4, D5); verificar que pasan las aserciones de lista y tamaño
      de 4.1, 4.2 y 4.11
- [x] 5.2 Dibujar en `SDLManager`: `render_power_ups` dentro de `render_players` (antes de los autos de suelo, con
      `camera.on_screen`) y `render_hud_effects` después de `render_hud_health`, copiando los PNG con su transparencia y sin fondo detrás
      (design D5); verificar que pasan
      4.1, 4.2, 4.11 completos (render sin errores) y que todos los `SdlTest.*` y `ConnectedSdlTest.*` siguen en verde

## 6. Gate del Hito 4

- [x] 6.1 Refactor final y hooks: formato, cpplint y cppcheck sin errores sobre los archivos tocados; verificar que
      ningún test nuevo del cliente lee config del servidor (todo sale del snapshot del Background) y que
      `git diff --stat HEAD -- editor/` está vacío
- [ ] 6.2 Correr `make run-tests` en verde, sin regresiones, y chequear el gate del Hito 4: (a) los 12 tests de
      4.1 – 4.12 (más 3.1, 3.4 y `RespawnedPowerUpHasNoCollector`) existen y pasan, (b) `make run-tests` en verde,
      (c) hooks sin errores, (d) `/opsx:verify` sin CRITICAL

## Workflow follow-up

- `/opsx:verify` y después `/opsx:archive`, que actualiza el estado del Hito 4 en `openspec/constitution/roadmap.md`.
- Al archivar: documentar `collected_by` en el formato del snapshot y los power-ups / carteles en README y manuales,
  porque cambian el protocolo y lo que ve el jugador.
- Prueba manual (DoD): correr una carrera, ver los power-ups en la pista, los carteles de Nitro y Escudo con la
  cuenta regresiva y escuchar el sonido al recogerlos; ajustar medidas de los carteles si hace falta.
