# Spec Delta

## ADDED Requirements

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
