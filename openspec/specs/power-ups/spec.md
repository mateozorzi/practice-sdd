# power-ups Specification

## Purpose

Power-ups en pista de Need4Speed: su configuración y los efectos (Nitro, Reparación, Escudo) que el servidor,
autoritativo, aplica al jugador que los recoge. Los hitos siguientes suman la aparición en el mundo físico, el
protocolo y la presentación en el cliente.

## Requirements

### Requirement: Configuración de power-ups
El servidor SHALL leer la configuración de power-ups de la sección obligatoria `power_ups:` dentro de `game:` en
`game_config.yaml`, con exactamente cinco valores: `nitro_multiplier`, `nitro_duration` (s), `repair_amount`,
`shield_duration` (s) y `respawn_cooldown` (s).

#### Scenario: Se cargan los valores de power-ups (1.1)
- **GIVEN** un `game_config.yaml` temporal escrito por el test con `power_ups:` = 2.0 / 2 / 30 / 4 / 10
- **WHEN** se carga la configuración
- **THEN** el config tiene `nitro_multiplier` 2.0, `nitro_duration` 2, `repair_amount` 30, `shield_duration` 4 y
  `respawn_cooldown` 10
- Test: `ServerConfigLoaderTest.LoadsPowerUpsSection` (`tests/test_server_config_loader.cpp`)

### Requirement: Configuración de power-ups inválida
El servidor MUST rechazar una configuración de power-ups incompleta o con valores negativos: la carga falla con un
error cuyo mensaje nombra la clave problemática. También falla si falta la sección `power_ups:`.

#### Scenario: Falta un valor de power-ups (1.2)
- **GIVEN** un `game_config.yaml` temporal con la sección `power_ups:` sin `shield_duration`
- **WHEN** se carga la configuración
- **THEN** la carga falla con un error que menciona `shield_duration`
- Test: `ServerConfigLoaderTest.MissingPowerUpValueThrows` (`tests/test_server_config_loader.cpp`)

#### Scenario: Un valor de power-ups es negativo (1.2)
- **GIVEN** un `game_config.yaml` temporal con `repair_amount: -30`
- **WHEN** se carga la configuración
- **THEN** la carga falla con un error que menciona `repair_amount`
- Test: `ServerConfigLoaderTest.NegativePowerUpValueThrows` (`tests/test_server_config_loader.cpp`)

#### Scenario: Falta la sección de power-ups
- **GIVEN** un `game_config.yaml` temporal con `game:` válido pero sin `power_ups:`
- **WHEN** se carga la configuración
- **THEN** la carga falla con un error que menciona `power_ups`
- Test: `ServerConfigLoaderTest.MissingPowerUpsSectionThrows` (`tests/test_server_config_loader.cpp`)

### Requirement: Nitro multiplica velocidad máxima y aceleración
Mientras el Nitro está activo, la velocidad máxima y la aceleración del auto SHALL ser sus valores actuales (con
upgrades) multiplicados por `nitro_multiplier`.

#### Scenario: Nitro sobre el auto de test (1.3)
- **GIVEN** el auto de test (velocidad máxima 10, aceleración 5)
- **WHEN** toma nitro
- **THEN** la velocidad máxima es 20 y la aceleración es 10
- Test: `PowerUpEffectsTest.NitroDoublesSpeedAndAcceleration` (`tests/test_player.cpp`)

#### Scenario: Nitro sobre un auto con upgrade de velocidad (1.4)
- **GIVEN** el auto de test con un upgrade de velocidad (velocidad máxima 13)
- **WHEN** toma nitro
- **THEN** la velocidad máxima es 26
- Test: `PowerUpEffectsTest.NitroAppliesOverSpeedUpgrade` (`tests/test_player.cpp`)

### Requirement: Los efectos con duración expiran
Nitro y Escudo SHALL durar `nitro_duration` y `shield_duration` segundos respectivamente, medidos solo con el tiempo
que avanza el servidor (o el test). Al expirar el Nitro, la velocidad máxima y la aceleración MUST volver exactamente
a sus valores previos. Cada efecto expira de forma independiente.

#### Scenario: El nitro expira y restaura los valores (1.5)
- **GIVEN** el auto de test con nitro activo
- **WHEN** avanzan 1.9 s
- **THEN** la velocidad máxima sigue en 20
- **WHEN** el tiempo llega a 2 s
- **THEN** el nitro no está activo, la velocidad máxima es exactamente 10 y la aceleración exactamente 5
- Test: `PowerUpEffectsTest.NitroExpiresAfterDurationRestoringValues` (`tests/test_player.cpp`)

#### Scenario: Nitro y escudo expiran por separado (1.7)
- **GIVEN** el auto de test que toma nitro y escudo en t = 0
- **WHEN** el tiempo llega a 2 s
- **THEN** el nitro expiró y el escudo sigue activo
- **WHEN** el tiempo llega a 4 s
- **THEN** ningún efecto está activo
- Test: `PowerUpEffectsTest.NitroAndShieldExpireIndependently` (`tests/test_player.cpp`)

### Requirement: Un solo efecto activo por tipo
Si un jugador toma un power-up cuyo efecto ya tiene activo, el efecto SHALL renovar su duración completa sin
acumular tiempo ni volver a multiplicar. Efectos de distinto tipo conviven.

#### Scenario: Renovar el nitro (1.6)
- **GIVEN** el auto de test con nitro tomado hace 1 s (le queda 1 s)
- **WHEN** toma otro nitro
- **THEN** le quedan 2 s de nitro y la velocidad máxima es 20 (no 40)
- Test: `PowerUpEffectsTest.NitroRenewsDurationWithoutStacking` (`tests/test_player.cpp`)

### Requirement: Reparación suma salud hasta la máxima
Al tomar Reparación, la salud del jugador SHALL aumentar en `repair_amount`, sin superar su salud máxima (que incluye
los upgrades de salud). La Reparación es instantánea: no queda un efecto activo.

#### Scenario: Reparación desde salud media (1.8)
- **GIVEN** el auto de test con salud 50/100
- **WHEN** toma reparación
- **THEN** la salud queda en 80
- Test: `PowerUpEffectsTest.RepairAddsHealth` (`tests/test_player.cpp`)

#### Scenario: Reparación tope en la salud máxima (1.9)
- **GIVEN** el auto de test con salud 90/100
- **WHEN** toma reparación
- **THEN** la salud queda en 100
- Test: `PowerUpEffectsTest.RepairCapsAtMaxHealth` (`tests/test_player.cpp`)

#### Scenario: Reparación con upgrade de salud (1.10)
- **GIVEN** el auto de test con un upgrade de salud (máxima 150) y salud 130
- **WHEN** toma reparación
- **THEN** la salud queda en 150
- Test: `PowerUpEffectsTest.RepairCapsAtUpgradedMaxHealth` (`tests/test_player.cpp`)

### Requirement: Escudo ignora el daño por choques
Mientras el Escudo está activo, el daño por choques MUST NOT reducir la salud del jugador. Al expirar, el daño vuelve
a aplicarse normalmente.

#### Scenario: Choque con escudo activo (1.11)
- **GIVEN** el auto de test con salud 100 y escudo activo
- **WHEN** recibe 20 de daño de choque
- **THEN** la salud sigue en 100
- Test: `PowerUpEffectsTest.ShieldIgnoresCollisionDamage` (`tests/test_player.cpp`)

#### Scenario: Choque después de que expira el escudo (1.12)
- **GIVEN** el auto de test con salud 100 y escudo activo
- **WHEN** avanzan 4 s y recibe 20 de daño de choque
- **THEN** la salud baja a 80
- Test: `PowerUpEffectsTest.ShieldExpiredTakesDamage` (`tests/test_player.cpp`)

### Requirement: El cheat de vida infinita es independiente de los power-ups
El cheat de vida infinita SHALL seguir activo después de usar Escudo o Reparación: el Escudo usa su propio estado y
su expiración no desactiva el cheat.

#### Scenario: Cheat y escudo expirado (1.13)
- **GIVEN** el auto de test con salud 100/100 y el cheat de vida infinita activado
- **WHEN** toma escudo, avanzan 4 s y recibe 20 de daño de choque
- **THEN** la salud sigue en 100
- Test: `PowerUpEffectsTest.InfiniteHealthSurvivesShieldExpiry` (`tests/test_player.cpp`)

#### Scenario: Cheat y reparación (1.14)
- **GIVEN** el auto de test con salud 100/100 y el cheat de vida infinita activado
- **WHEN** toma reparación y recibe 20 de daño de choque
- **THEN** la salud sigue en 100
- Test: `PowerUpEffectsTest.InfiniteHealthSurvivesRepair` (`tests/test_player.cpp`)

### Requirement: Los autos muertos no tienen efectos
Un jugador con salud 0 MUST NOT recibir efectos de ningún power-up, y al morir SHALL perder todos sus efectos activos
(la velocidad máxima y la aceleración vuelven a sus valores sin nitro).

#### Scenario: Auto muerto toma power-ups (1.15)
- **GIVEN** el auto de test muerto (salud 0)
- **WHEN** toma nitro, reparación y escudo
- **THEN** no tiene efectos activos, la salud sigue en 0 y la velocidad máxima sigue en 10
- Test: `PowerUpEffectsTest.DeadCarIgnoresPowerUps` (`tests/test_player.cpp`)

#### Scenario: Con escudo, un choque letal no mata (1.16)
- **GIVEN** el auto de test con salud 100 y nitro y escudo activos
- **WHEN** recibe 100 de daño de choque (letal sin escudo)
- **THEN** no muere: la salud sigue en 100 y el nitro y el escudo siguen activos
- Test: `PowerUpEffectsTest.ShieldPreventsLethalDamage` (`tests/test_player.cpp`)

#### Scenario: Muere después de que expira el escudo y pierde los efectos (1.16)
- **GIVEN** el auto de test con salud 100 que toma escudo en t = 0 y nitro en t = 3 s
- **WHEN** el tiempo llega a 4 s (expira el escudo, el nitro sigue activo) y recibe 100 de daño de choque
- **THEN** el auto muere (salud 0), no tiene efectos activos, la velocidad máxima es 10 y la aceleración 5
- Test: `PowerUpEffectsTest.DyingClearsEffects` (`tests/test_player.cpp`)

### Requirement: Cada carrera arranca sin efectos
Al empezar una carrera, el jugador SHALL no tener efectos activos y su auto MUST tener la velocidad máxima y la
aceleración sin nitro.

#### Scenario: Nueva carrera con efectos activos (1.17)
- **GIVEN** el auto de test con nitro y escudo activos
- **WHEN** empieza otra carrera
- **THEN** no hay efectos activos, la velocidad máxima es 10 y la aceleración 5
- Test: `PowerUpEffectsTest.StartRaceClearsEffects` (`tests/test_player.cpp`)

### Requirement: Un power-up cada dos checkpoints
Al cargar los checkpoints de una carrera, el servidor SHALL crear un power-up por cada tramo impar entre checkpoints
consecutivos (1→2, 3→4, …). Un checkpoint final sin pareja no genera power-up. Las posiciones no se leen del mapa.

#### Scenario: Circuito de 4 checkpoints (2.1)
- **GIVEN** un circuito de test con 4 checkpoints
- **WHEN** se cargan los power-ups de la carrera
- **THEN** hay exactamente 2 power-ups
- Test: `PowerUpManagerTest.FourCheckpointsSpawnTwoPowerUps` (`tests/test_power_up.cpp`)

#### Scenario: Circuito de 5 checkpoints (2.2)
- **GIVEN** un circuito de test con 5 checkpoints
- **WHEN** se cargan los power-ups de la carrera
- **THEN** hay exactamente 2 power-ups
- Test: `PowerUpManagerTest.FiveCheckpointsSpawnTwoPowerUps` (`tests/test_power_up.cpp`)

### Requirement: El power-up aparece en el punto medio del tramo
Cada power-up SHALL ubicarse en el punto medio entre los centros de los dos checkpoints de su tramo, en las
coordenadas en píxeles de la carrera.

#### Scenario: Posición del primer power-up (2.3)
- **GIVEN** un circuito de test de 4 checkpoints con los checkpoints 1 y 2 en P y Q (leídos de la carrera)
- **WHEN** se cargan los power-ups de la carrera
- **THEN** el primer power-up está en (P + Q) / 2
- Test: `PowerUpManagerTest.PowerUpSpawnsAtMidpointOfCheckpoints` (`tests/test_power_up.cpp`)

### Requirement: El tipo se sortea con un generador inyectable
El tipo de cada power-up (Nitro, Reparación o Escudo) SHALL sortearse cada vez que aparece, al cargarse y al
reaparecer, usando un generador que se puede inyectar. El generador real MUST producir los tres tipos con
distribución uniforme.

#### Scenario: Tipos según el generador falso (2.4)
- **GIVEN** el generador falso del Background y un circuito de test de 4 checkpoints
- **WHEN** se crean los 2 power-ups
- **THEN** el primero es Escudo y el segundo es Nitro
- Test: `PowerUpManagerTest.TypesFollowInjectedGenerator` (`tests/test_power_up.cpp`)

#### Scenario: El generador real produce los 3 tipos (2.5)
- **GIVEN** el generador real de tipos
- **WHEN** se sortean 1000 tipos
- **THEN** aparecen Nitro, Reparación y Escudo al menos una vez cada uno
- Test: `PowerUpTypeGeneratorTest.RandomGeneratorYieldsAllThreeTypes` (`tests/test_power_up.cpp`)

### Requirement: El power-up es un sensor sin respuesta física
Cada power-up SHALL ser un sensor del mundo físico: atravesarlo MUST NOT cambiar la velocidad del auto ni causar daño.

#### Scenario: El fixture es sensor (2.6)
- **GIVEN** un circuito de test de 4 checkpoints con sus power-ups cargados
- **WHEN** se inspecciona el fixture del primer power-up
- **THEN** es un sensor
- Test: `PowerUpManagerTest.PowerUpFixtureIsSensor` (`tests/test_power_up.cpp`)

#### Scenario: Atravesar un power-up no frena ni daña (2.7)
- **GIVEN** el auto 1 (salud 100) a velocidad constante, sin amortiguación, en línea recta hacia un power-up
- **WHEN** lo atraviesa por completo
- **THEN** su velocidad es la misma que antes del contacto y su salud sigue en 100
- Test: `CollisionListenerTest.CrossingAPowerUpKeepsVelocityAndHealth` (`tests/test_collision_listener.cpp`)

### Requirement: Lo recoge el primer auto vivo que lo toca
Cuando un auto vivo entra al sensor de un power-up activo, el servidor SHALL aplicarle el efecto de ese tipo y dejar
el power-up inactivo para todos. Un power-up inactivo MUST NOT dar efectos. Si varios autos entran en el mismo paso
de física, MUST recogerlo uno solo.

#### Scenario: El auto recoge un Escudo (2.8)
- **GIVEN** el auto 1 vivo y un power-up activo de tipo Escudo
- **WHEN** el auto entra al sensor del power-up
- **THEN** el auto 1 tiene el escudo activo y el power-up queda inactivo
- Test: `CollisionListenerTest.CarCollectsShieldPowerUp` (`tests/test_collision_listener.cpp`)

#### Scenario: Un power-up inactivo no da efectos (2.9)
- **GIVEN** un power-up que ya fue recogido (inactivo)
- **WHEN** el auto 2 entra a su sensor
- **THEN** el auto 2 no tiene efectos activos y el power-up sigue inactivo
- Test: `CollisionListenerTest.InactivePowerUpGivesNoEffect` (`tests/test_collision_listener.cpp`)

#### Scenario: Dos autos en el mismo paso (2.10)
- **GIVEN** los autos 1 y 2 ubicados de modo que entran al mismo sensor de un power-up activo en el mismo paso
- **WHEN** avanza ese paso de física
- **THEN** exactamente uno de los dos autos tiene el efecto y el power-up queda inactivo
- Test: `CollisionListenerTest.TwoCarsSameStepOnlyOneCollects` (`tests/test_collision_listener.cpp`)

### Requirement: Los autos muertos no recogen power-ups
Un auto con salud 0 que entra al sensor de un power-up activo MUST NOT recogerlo: el power-up SHALL seguir activo y
el auto sin efectos.

#### Scenario: Auto muerto entra al sensor (2.11)
- **GIVEN** el auto 1 muerto (salud 0) y un power-up activo
- **WHEN** el auto entra al sensor del power-up
- **THEN** el power-up sigue activo y el auto 1 no tiene efectos activos
- Test: `CollisionListenerTest.DeadCarDoesNotCollect` (`tests/test_collision_listener.cpp`)

### Requirement: El power-up reaparece tras el cooldown
Un power-up recogido SHALL volver a estar activo, en la misma posición y con un tipo nuevo sorteado, cuando pasan
`respawn_cooldown` segundos de tiempo avanzado explícitamente por el servidor o el test. Antes de ese plazo MUST
seguir inactivo.

#### Scenario: Antes del cooldown sigue inactivo (2.12)
- **GIVEN** un power-up recogido
- **WHEN** pasan 9.9 s (594 pasos de 1/60 s)
- **THEN** sigue inactivo
- Test: `PowerUpManagerTest.StaysInactiveBeforeCooldown` (`tests/test_power_up.cpp`)

#### Scenario: Reaparece con el siguiente tipo (2.13)
- **GIVEN** el generador falso del Background, un circuito de test de 4 checkpoints (power-ups Escudo y Nitro) y el
  primer power-up recogido
- **WHEN** pasan 10 s (600 pasos de 1/60 s)
- **THEN** el primer power-up está activo, en la misma posición que antes y su tipo es Reparación (el siguiente de la
  secuencia)
- Test: `PowerUpManagerTest.RespawnsAfterCooldownWithNextType` (`tests/test_power_up.cpp`)
