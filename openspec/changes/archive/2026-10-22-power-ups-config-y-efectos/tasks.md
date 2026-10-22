# Tasks

## 1. Tests de aceptación en rojo

- [x] 1.1 Agregar `make_test_power_up_config()` y `make_power_up_test_server_config()` a `tests/server_test_utils.*`
      con los valores del Background (design D7); verificar que `nfs_tests` compila
- [x] 1.2 Escribir en `tests/test_server_config_loader.cpp` los tests `ServerConfigLoaderTest.LoadsPowerUpsSection`
      (1.1), `MissingPowerUpValueThrows`, `NegativePowerUpValueThrows` (1.2) y `MissingPowerUpsSectionThrows`, con
      YAML temporales que extienden `MINIMAL_GAME`; verificar que los de error fallan (hoy no tiran) y que
      `LoadsPowerUpsSection` falla
- [x] 1.3 Escribir en `tests/test_player.cpp` el fixture `PowerUpEffectsTest` y los 16 tests de la spec (1.3 – 1.17:
      `NitroDoublesSpeedAndAcceleration` … `StartRaceClearsEffects`, incluidos `ShieldPreventsLethalDamage` y
      `DyingClearsEffects`)
- [x] 1.4 Agregar solo las declaraciones mínimas para que compile (`PowerUpConfig`, `common/power_up_type.h`, y los
      métodos de `Player`/`Car` de design D3–D5 como stubs que no hacen nada); compilar con
      `cmake --build build --target nfs_tests nfs_client_tests -j16` y verificar con
      `./build/nfs_tests --gtest_filter='PowerUpEffectsTest.*:ServerConfigLoaderTest.*PowerUp*'` que **todos** los
      tests nuevos fallan y que los existentes siguen en verde

## 2. Configuración de power-ups

- [x] 2.1 Implementar `load_power_ups` en `server/server_config_loader.*` con validación de sección faltante, clave
      faltante, valores negativos y `nitro_multiplier > 0` (design D1); verificar que 1.1 – 1.2 y
      `MissingPowerUpsSectionThrows` pasan
- [x] 2.2 Agregar `power_ups:` (1.25 / 3 / 50 / 5 / 20) bajo `game:` en `server/config/game_config.yaml` y la sección
      a `MINIMAL_GAME`; verificar que todos los `ServerConfigLoaderTest.*` y el resto de `nfs_tests` pasan

## 3. Nitro en el auto

- [x] 3.1 Implementar `Car::set_power_multiplier`, `get_max_speed` y `get_acceleration` y usarlos en
      `detect_linear_movement` (design D3); verificar que los `CarTest.*` existentes siguen en verde
- [x] 3.2 Implementar `Player::apply_power_up` para Nitro, más `has_effect`, `get_effect_remaining`,
      `get_max_speed` y `get_acceleration`; verificar que 1.3 y 1.4 pasan

## 4. Tiempo de los efectos

- [x] 4.1 Agregar `EFFECT_TIME_EPSILON` a `server/server_constants.h` e implementar `Player::advance_effects`
      (design D5), con el escudo como estado propio; verificar que 1.5, 1.6 y 1.7 pasan

## 5. Reparación y escudo

- [x] 5.1 Implementar Reparación con tope en `base_health` (design D4); verificar que 1.8, 1.9 y 1.10 pasan
- [x] 5.2 Hacer que `apply_collision_damage` ignore el daño con escudo activo, sin tocar `infinite_health`;
      verificar que 1.11, 1.12, 1.13, 1.14 y `ShieldPreventsLethalDamage` pasan, junto con
      `PlayerTest.InfiniteHealthIgnoresDamage`

## 6. Muerte y nueva carrera

- [x] 6.1 Implementar `clear_effects()`: ignorar power-ups con el auto muerto, limpiar al llegar a salud 0 y en
      `start_race()`; verificar que 1.15, `DyingClearsEffects` y 1.17 pasan, junto con
      `PlayerTest.StartRaceResetsTheRaceState`

## 7. Cierre y gate del Hito 1

- [x] 7.1 Refactor sin cambiar comportamiento (nombres y duplicación en `Player`), con hooks de formato, cpplint y
      cppcheck sin errores; verificar que `PowerUpEffectsTest.*` sigue en verde
- [x] 7.2 Correr `make run-tests` en verde sin regresiones y chequear el gate del Hito 1: los 1.1 – 1.17 existen y
      pasan, ningún test de power-ups lee los valores de `server/config/*.yaml`, hooks sin errores, sin cambios en
      `editor/`, y `/opsx:verify` sin CRITICAL

## Workflow follow-up

- Al archivar: actualizar el estado del Hito 1 en `openspec/constitution/roadmap.md`.
- Si `README.md` o los manuales documentan `game_config.yaml`, sumar la sección `power_ups`.
