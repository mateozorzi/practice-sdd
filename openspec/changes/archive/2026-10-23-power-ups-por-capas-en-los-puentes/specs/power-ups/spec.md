# Spec Delta

## MODIFIED Requirements

### Requirement: Lo recoge el primer auto vivo que lo toca
Cuando un auto vivo que puede recogerlo por su capa entra al sensor de un power-up activo, el servidor SHALL
aplicarle el efecto de ese tipo y dejar el power-up inactivo para todos. Un power-up inactivo MUST NOT dar efectos.
Si varios autos entran en el mismo paso de física, MUST recogerlo uno solo.

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

### Requirement: El power-up reaparece tras el cooldown
Un power-up recogido SHALL volver a estar activo, en la misma posición, con la misma capa y con un tipo nuevo
sorteado, cuando pasan `respawn_cooldown` segundos de tiempo avanzado explícitamente por el servidor o el test. Antes
de ese plazo MUST seguir inactivo.

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

#### Scenario: Reaparece en su capa (6.11)
- **GIVEN** el cruce de test del Hito 5 (puente en (100..160, 100..160)), los valores de power-ups del Hito 2
  (`respawn_cooldown` 10 s), un power-up `UNDER_BRIDGE` en (130, 130) cargado con P = (130, 0) y Q = (130, 260) por
  el camino de abajo, y recogido por un auto `UNDER_BRIDGE`
- **WHEN** pasan 10 s (600 pasos de 1/60 s)
- **THEN** el power-up está activo, sigue en (130, 130) y su capa sigue siendo `UNDER_BRIDGE`
- Test: `BridgePowerUpTest.RespawnKeepsPositionAndLayer` (`tests/test_power_up.cpp`)

### Requirement: Los power-ups viajan en el snapshot
El snapshot del mundo que el servidor manda a los clientes SHALL incluir cada power-up de la carrera con su id, su
tipo, su posición en píxeles, si está activo, el id del jugador que lo recogió (0 si nadie lo recogió desde que
apareció) y su capa. Al serializarlo y deserializarlo por el protocolo, el cliente MUST obtener exactamente los mismos
valores.

#### Scenario: Round-trip del DTO de power-up (3.1)
- **GIVEN** un snapshot que contiene el power-up de ejemplo del Background (id 1, Nitro, posición (100, 200), activo,
  sin recolector) y un segundo power-up id 2, Escudo, posición (300, 200), inactivo, recogido por el jugador 1, los
  dos con capa `ON_GROUND`
- **WHEN** el servidor lo serializa y el cliente lo deserializa
- **THEN** el cliente obtiene dos power-ups idénticos a los originales: id 1, Nitro, (100, 200), activo, recolector
  0; e id 2, Escudo, (300, 200), inactivo, recolector 1; los dos `ON_GROUND`
- Test: `PowerUpProtocolTest.PowerUpDTORoundTrip` (`tests/test_common.cpp`)

#### Scenario: Round-trip con la capa (6.12)
- **GIVEN** un snapshot con el DTO de ejemplo del Hito 6: power-up {id 1, Nitro, (130, 130), activo, `ON_BRIDGE`}
- **WHEN** el servidor lo serializa y el cliente lo deserializa
- **THEN** el cliente obtiene un power-up idéntico, capa `ON_BRIDGE` incluida
- Test: `PowerUpProtocolTest.PowerUpDTOLayerRoundTrip` (`tests/test_common.cpp`)

#### Scenario: Una capa desconocida se lee como suelo (soporte de 6.12)
- **GIVEN** un snapshot con un power-up {id 1, Nitro, (130, 130), activo} cuya capa tiene un valor que el cliente no
  conoce (7)
- **WHEN** el servidor lo serializa y el cliente lo deserializa
- **THEN** el cliente obtiene ese power-up con capa `ON_GROUND`, así se dibuja debajo de los puentes
- Test: `PowerUpProtocolTest.UnknownLayerIsReadAsOnGround` (`tests/test_common.cpp`)

## ADDED Requirements

### Requirement: El power-up toma la capa del camino dentro de un puente
Al cargar la carrera, un power-up cuyo cuadrado queda dentro del rectángulo de un puente SHALL tomar la capa del
camino por la calle en su posición: `ON_BRIDGE` si el camino pasa por arriba, `UNDER_BRIDGE` si pasa por debajo. En
un puente sobre el agua (sin sensor `UNDER_BRIDGE`) MUST quedar `ON_BRIDGE`.

#### Scenario: Tramo por el camino de arriba (6.1)
- **GIVEN** el cruce de test del Hito 5 (puente en (100..160, 100..160), camino de arriba horizontal con sensor
  `ON_BRIDGE` (40..220, 110..150)) y los checkpoints 1 y 2 en P = (0, 130) y Q = (260, 130)
- **WHEN** se cargan los power-ups con ese cruce
- **THEN** el primer power-up está en (130, 130), dentro del puente, y su capa es `ON_BRIDGE`
- Test: `BridgePowerUpTest.OverTheBridgeIsOnBridge` (`tests/test_power_up.cpp`)

#### Scenario: Tramo por el camino de abajo (6.2)
- **GIVEN** el cruce de test del Hito 5 (camino de abajo vertical con sensor `UNDER_BRIDGE` (110..150, 40..220)) y
  los checkpoints 1 y 2 en P = (130, 0) y Q = (130, 260)
- **WHEN** se cargan los power-ups con ese cruce
- **THEN** el primer power-up está en (130, 130), dentro del puente, y su capa es `UNDER_BRIDGE`
- Test: `BridgePowerUpTest.UnderTheBridgeIsUnderBridge` (`tests/test_power_up.cpp`)

#### Scenario: Puente sobre el agua (6.3)
- **GIVEN** el puente sobre el agua del Background: el rectángulo (100..160, 100..160) con el camino de arriba y su
  sensor `ON_BRIDGE` (40..220, 110..150), sin sensor `UNDER_BRIDGE` ni camino de abajo, y los checkpoints 1 y 2 en
  P = (0, 130) y Q = (260, 130)
- **WHEN** se cargan los power-ups con ese puente
- **THEN** el primer power-up está en (130, 130) y su capa es `ON_BRIDGE`
- Test: `BridgePowerUpTest.WaterBridgeIsOnBridge` (`tests/test_power_up.cpp`)

### Requirement: Fuera de los puentes el power-up es de suelo
Un power-up cuyo cuadrado queda entero fuera de todo rectángulo de puente SHALL tener capa `ON_GROUND`, aunque el
camino en ese punto tenga otra capa (por ejemplo, en la rampa de un puente). Sin grilla de calles o sin camino entre
los checkpoints (respaldo (P + Q) / 2), el power-up MUST ser `ON_GROUND`.

#### Scenario: Punto medio en la rampa, fuera del puente (6.4)
- **GIVEN** el cruce de test del Hito 5 y los checkpoints 1 y 2 en P = (0, 130) y Q = (140, 130), con el punto medio
  del camino en (70, 130): dentro del sensor `ON_BRIDGE` y con el cuadrado (55..85, 115..145) fuera del rectángulo
  del puente
- **WHEN** se cargan los power-ups con ese cruce
- **THEN** el primer power-up está en (70, 130) y su capa es `ON_GROUND`
- Test: `BridgePowerUpTest.OutsideTheBridgeIsOnGround` (`tests/test_power_up.cpp`)

#### Scenario: Sin grilla es de suelo (soporte de 6.4)
- **GIVEN** un circuito de test de 4 checkpoints, sin grilla de calles
- **WHEN** se cargan los power-ups
- **THEN** los dos power-ups tienen capa `ON_GROUND`
- Test: `BridgePowerUpTest.WithoutGridIsOnGround` (`tests/test_power_up.cpp`)

### Requirement: El power-up nunca cruza el borde de un puente
Si en el punto medio del camino el cuadrado del power-up (medio lado 15 px) queda cruzando el borde del rectángulo de
un puente, el power-up SHALL correrse por el camino hasta la posición más cercana (en distancia recorrida por el
camino) donde su cuadrado queda entero dentro o entero fuera de todo puente, y MUST tomar la capa de esa posición. Si
ningún punto del camino cumple, MUST quedar en el punto medio del camino con capa `ON_GROUND` y el servidor informa
el tramo.

#### Scenario: Se corre hacia adentro del puente (6.5)
- **GIVEN** el cruce de test del Hito 5 y los checkpoints 1 y 2 en P = (0, 130) y Q = (210, 130): el punto medio
  del camino es (105, 130) y su cuadrado (90..120) cruza el borde x = 100; entero dentro queda a 10 px, en
  (115, 130), y entero fuera a 20 px, en (85, 130)
- **WHEN** se cargan los power-ups con ese cruce
- **THEN** el primer power-up está en (115, 130) y su capa es `ON_BRIDGE`
- Test: `BridgePowerUpTest.CrossingTheEdgeMovesInside` (`tests/test_power_up.cpp`)

#### Scenario: Se corre hacia afuera del puente (6.5)
- **GIVEN** el cruce de test del Hito 5 y los checkpoints 1 y 2 en P = (0, 130) y Q = (190, 130): el punto medio
  del camino es (95, 130) y su cuadrado (80..110) cruza el borde x = 100; entero fuera queda a 10 px, en (85, 130), y
  entero dentro a 20 px, en (115, 130)
- **WHEN** se cargan los power-ups con ese cruce
- **THEN** el primer power-up está en (85, 130) y su capa es `ON_GROUND`
- Test: `BridgePowerUpTest.CrossingTheEdgeMovesOutside` (`tests/test_power_up.cpp`)

#### Scenario: Ningún punto del camino sale del borde (soporte de 6.5)
- **GIVEN** el cruce de test del Hito 5 y los checkpoints 1 y 2 los dos en (100, 130), sobre el borde x = 100: el
  camino es una sola celda y el cuadrado del power-up (85..115) cruza el borde en cualquier punto del camino
- **WHEN** se cargan los power-ups con ese cruce
- **THEN** el primer power-up queda en (100, 130) con capa `ON_GROUND` y el servidor informa el tramo `1 -> 2`
- Test: `BridgePowerUpTest.NoPlaceOffTheEdgeKeepsMidpointAndReports` (`tests/test_power_up.cpp`)

### Requirement: Un power-up de puente lo recoge solo un auto de su capa
Un power-up `ON_BRIDGE` o `UNDER_BRIDGE` MUST NOT darle efectos a un auto de otra capa que entra a su sensor: SHALL
seguir activo. Lo recoge un auto de su misma capa. Un power-up `ON_GROUND` lo recoge un auto de cualquier capa. La
capa del auto que cuenta es la que tiene al entrar al sensor.

#### Scenario: Un auto de abajo no recoge el power-up de arriba (6.6)
- **GIVEN** los autos de test vivos con salud 100, un power-up activo `ON_BRIDGE` cargado en el cruce del Hito 5 y
  el auto 1 con capa `UNDER_BRIDGE`
- **WHEN** el auto 1 entra al sensor del power-up
- **THEN** el power-up sigue activo y el auto 1 no tiene efectos activos
- Test: `CollisionListenerTest.UnderBridgeCarSkipsOnBridgePowerUp` (`tests/test_collision_listener.cpp`)

#### Scenario: Un auto de arriba recoge el power-up de arriba (6.7)
- **GIVEN** un power-up activo `ON_BRIDGE` cargado en el cruce del Hito 5 y el auto 1 con capa `ON_BRIDGE`
- **WHEN** el auto 1 entra al sensor del power-up
- **THEN** el auto 1 tiene el efecto del power-up y el power-up queda inactivo
- Test: `CollisionListenerTest.OnBridgeCarCollectsOnBridgePowerUp` (`tests/test_collision_listener.cpp`)

#### Scenario: El power-up de abajo es solo para los de abajo (6.8)
- **GIVEN** un power-up activo `UNDER_BRIDGE` cargado en el cruce del Hito 5, el auto 1 con capa `ON_BRIDGE` y el
  auto 2 con capa `UNDER_BRIDGE`
- **WHEN** entra al sensor el auto 1 y después el auto 2
- **THEN** después del auto 1 el power-up sigue activo y el auto 1 no tiene efectos; después del auto 2 el auto 2
  tiene el efecto y el power-up queda inactivo
- Test: `CollisionListenerTest.UnderBridgePowerUpOnlyForUnderBridgeCars` (`tests/test_collision_listener.cpp`)

#### Scenario: El power-up de suelo lo recoge cualquier capa (6.9)
- **GIVEN** un power-up activo `ON_GROUND` y el auto 1 con capa `ON_BRIDGE` (por ejemplo, en la rampa)
- **WHEN** el auto 1 entra al sensor del power-up
- **THEN** el auto 1 tiene el efecto del power-up y el power-up queda inactivo
- Test: `CollisionListenerTest.OnGroundPowerUpForAnyLayer` (`tests/test_collision_listener.cpp`)

#### Scenario: Con sensores reales lo recoge el auto que pasa por arriba (6.10)
- **GIVEN** el cruce del Hito 5 con sus sensores `ON_BRIDGE` (40..220, 110..150) y `UNDER_BRIDGE`
  (110..150, 40..220) creados en el mundo físico, un power-up activo `ON_BRIDGE` en (130, 130) y los autos 1 y 2 con
  capa `ON_GROUND` fuera de los sensores
- **WHEN** con pasos de física el auto 1 pasa por debajo (de (130, 0) a (130, 260)) y después el auto 2 pasa por
  arriba (de (0, 130) a (260, 130))
- **THEN** después del auto 1 el power-up sigue activo y el auto 1 no tiene efectos; después del auto 2 el auto 2
  tiene el efecto y el power-up queda inactivo
- Test: `CollisionListenerTest.RealSensorsOnlyTheCarOnTopCollects` (`tests/test_collision_listener.cpp`)

### Requirement: La carrera en curso manda la capa de sus power-ups
Durante una carrera, el snapshot que manda el game loop SHALL traer cada power-up con la capa que se le calculó al
cargar la carrera.

#### Scenario: Power-up de puente en el snapshot del loop (6.13)
- **GIVEN** una carrera de test en Liberty City cuyo primer tramo cruza por arriba un puente de la ciudad, con el
  punto medio del camino dentro del rectángulo del puente, y el loop con un jugador en carrera
- **WHEN** el loop avanza un tick
- **THEN** el snapshot trae el primer power-up con capa `ON_BRIDGE`
- Test: `GameLoopPowerUpTest.SnapshotHasTheBridgeLayer` (`tests/test_game_loop.cpp`)

### Requirement: El cliente dibuja cada power-up en su capa respecto del puente
El cliente SHALL dibujar los power-ups `ON_BRIDGE` después de los puentes (encima) y los `UNDER_BRIDGE` y `ON_GROUND`
antes de los puentes (debajo), igual que a los autos de cada capa.

#### Scenario: Uno arriba y otro abajo del mismo puente (6.14)
- **GIVEN** SDL offscreen, un snapshot armado en el test con un rectángulo de puente en (100..160, 100..160) y los
  power-ups activos {1, Nitro, (130, 130), `ON_BRIDGE`} y {2, Escudo, (130, 130), `UNDER_BRIDGE`}
- **WHEN** el cliente renderiza el frame
- **THEN** la lista de power-ups que se dibujan antes de los puentes contiene solo el id 2, la que se dibuja después
  contiene solo el id 1, y el render del frame termina sin errores
- Test: `PowerUpRenderTest.OnBridgePowerUpIsDrawnAboveTheBridge` (`tests/client/test_sdl_client.cpp`)

### Requirement: Las carreras del juego respetan las capas de los puentes
Con las grillas de calles y los puentes de las tres ciudades, ningún power-up de las carreras incluidas en el juego
SHALL cruzar el borde de un puente, y cada power-up dentro de un puente MUST tener la capa del camino en ese punto.

#### Scenario: Carreras fijas de test (6.15)
- **GIVEN** las carreras fijas de `tests/data/races/` y las grillas de `server/world/streets/` con los sensores y
  puentes de `server/world/`
- **WHEN** se cargan los power-ups de cada carrera con la grilla y los puentes de su ciudad
- **THEN** ningún power-up tiene su cuadrado cruzando el borde de un rectángulo de puente, cada uno entero dentro de
  un puente tiene la capa del camino en su posición (`ON_BRIDGE` o `UNDER_BRIDGE`) y cada uno fuera de los puentes es
  `ON_GROUND`; una falla informa carrera y tramo
- Test: `RealRacesPowerUpTest.BridgePowerUpsHaveThePathLayer` (`tests/test_power_up.cpp`)
