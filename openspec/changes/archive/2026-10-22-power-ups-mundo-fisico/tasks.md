# Tasks

## 1. Tests de aceptación en rojo

- [x] 1.1 Agregar `make_sequence_power_up_type_generator(std::vector<PowerUpType>)` a `tests/server_test_utils.*`
      (cicla la secuencia, design D3); verificar que `nfs_tests` compila
- [x] 1.2 Escribir `tests/test_power_up.cpp` con el fixture `PowerUpManagerTest` (design D6) y los tests
      `FourCheckpointsSpawnTwoPowerUps` (2.1), `FiveCheckpointsSpawnTwoPowerUps` (2.2),
      `PowerUpSpawnsAtMidpointOfCheckpoints` (2.3), `TypesFollowInjectedGenerator` (2.4), `PowerUpFixtureIsSensor`
      (2.6), `StaysInactiveBeforeCooldown` (2.12), `RespawnsAfterCooldownWithNextType` (2.13) y
      `PowerUpTypeGeneratorTest.RandomGeneratorYieldsAllThreeTypes` (2.5); registrarlo en `tests/CMakeLists.txt`
- [x] 1.3 Escribir en `tests/test_collision_listener.cpp` los tests `CrossingAPowerUpKeepsVelocityAndHealth` (2.7),
      `CarCollectsShieldPowerUp` (2.8), `InactivePowerUpGivesNoEffect` (2.9), `TwoCarsSameStepOnlyOneCollects`
      (2.10) y `DeadCarDoesNotCollect` (2.11), con un `step(n)` que avanza el mundo y el manager (design D5)
- [x] 1.4 Agregar solo las declaraciones mínimas para que compile (`server/power_up.h` con `PowerUp`,
      `PowerUpManager` y `make_random_power_up_type_generator` como stubs vacíos, `server/power_up.cpp` en
      `server/CMakeLists.txt`, `SensorType::POWER_UP` y el constructor nuevo de `CollisionListener` sin lógica);
      compilar con `cmake --build build --target nfs_tests nfs_client_tests -j16` y verificar con
      `./build/nfs_tests --gtest_filter='PowerUpManagerTest.*:PowerUpTypeGeneratorTest.*:CollisionListenerTest.*PowerUp*:CollisionListenerTest.*Collect*'`
      que **todos** los tests nuevos fallan y que los existentes siguen en verde

## 2. Spawn y tipo de los power-ups

- [x] 2.1 Implementar `PowerUpManager::load` / `clear` / destructor y la creación del sensor kinematic con su
      `userData` (design D1, D2); verificar que pasan 2.1, 2.2, 2.3 y 2.6
- [x] 2.2 Usar el generador inyectado al crear cada power-up e implementar `make_random_power_up_type_generator`
      (design D3); verificar que pasan 2.4 y 2.5

## 3. Recolección por contacto

- [x] 3.1 Implementar `PowerUpManager::try_collect` (inactivo o auto muerto → sin cambios; si no, desactiva, arranca
      el cooldown y aplica el efecto) y el manejo de `SensorType::POWER_UP` en
      `CollisionListener::handle_sensor_contact` (design D4); verificar que pasan 2.7 – 2.11 y que todos los
      `CollisionListenerTest.*` existentes siguen en verde

## 4. Reaparición

- [x] 4.1 Implementar `PowerUpManager::advance(double elapsed_seconds)` con `EFFECT_TIME_EPSILON` y nuevo sorteo del
      tipo al reaparecer (design D5); verificar que pasan 2.12 y 2.13

## 5. Gate del Hito 2

- [x] 5.1 Refactor final y hooks: formato, cpplint y cppcheck sin errores sobre los archivos tocados; verificar que
      ningún test nuevo lee valores de `server/config/*.yaml` (solo `make_power_up_test_server_config()` y el
      generador de secuencia) y que no hay cambios en `editor/`
- [x] 5.2 Correr `make run-tests` en verde, sin regresiones, y chequear el gate del Hito 2: (a) los 13 tests de
      2.1 – 2.13 existen y pasan, (b) `make run-tests` en verde, (c) hooks sin errores, (d) `/opsx:verify` sin CRITICAL

## Workflow follow-up

- `/opsx:verify` y después `/opsx:archive`, que actualiza el estado del Hito 2 en `openspec/constitution/roadmap.md`.
- Prueba manual: queda para el Hito 3, cuando los power-ups estén conectados al game loop.
