# Spec Delta

## ADDED Requirements

### Requirement: Los power-ups viajan en el snapshot
El snapshot del mundo que el servidor manda a los clientes SHALL incluir cada power-up de la carrera con su id, su
tipo, su posición en píxeles y si está activo. Al serializarlo y deserializarlo por el protocolo, el cliente MUST
obtener exactamente los mismos valores.

#### Scenario: Round-trip del DTO de power-up (3.1)
- **GIVEN** un snapshot que solo contiene el power-up de ejemplo del Background: id 1, Nitro, posición (100, 200),
  activo
- **WHEN** el servidor lo serializa y el cliente lo deserializa
- **THEN** el cliente obtiene un único power-up idéntico: id 1, Nitro, (100, 200), activo
- Test: `PowerUpProtocolTest.PowerUpDTORoundTrip` (`tests/test_common.cpp`)

### Requirement: Los efectos activos viajan en el snapshot
El snapshot SHALL incluir, por cada efecto con duración activo (Nitro o Escudo) de cada jugador, el id del jugador,
el tipo de efecto y el tiempo restante. Al serializarlo y deserializarlo, el cliente MUST obtener el mismo tipo y el
mismo tiempo restante, con precisión de milisegundos.

#### Scenario: Round-trip de un efecto en el jugador 1 (3.2)
- **GIVEN** un snapshot con el jugador 1 y el efecto de ejemplo del Background en ese jugador: Escudo con 2.5 s
  restantes
- **WHEN** el servidor lo serializa y el cliente lo deserializa
- **THEN** el cliente obtiene un único efecto idéntico: jugador 1, Escudo, 2.5 s restantes
- Test: `PowerUpProtocolTest.SnapshotEffectRoundTrip` (`tests/test_common.cpp`)

### Requirement: La carrera en curso tiene sus power-ups en el snapshot
Al empezar cada carrera, el servidor SHALL crear los power-ups de sus checkpoints, y cada snapshot de la carrera MUST
listarlos con su estado actual.

#### Scenario: Primer tick de la carrera (3.3)
- **GIVEN** la config rápida y los power-ups del Background, la carrera de test de 4 checkpoints, el generador falso
  que siempre da Nitro y el jugador 1 en carrera
- **WHEN** el loop avanza un tick
- **THEN** el snapshot trae 2 power-ups, los dos activos y de tipo Nitro
- Test: `GameLoopPowerUpTest.SnapshotHasTwoActiveNitroPowerUps` (`tests/test_game_loop.cpp`)

### Requirement: Recoger un power-up en carrera se refleja en el snapshot
Cuando el auto de un jugador recoge un power-up durante la carrera, el snapshot de ese tick SHALL mostrar el power-up
inactivo y, si el efecto tiene duración, el efecto en ese jugador con la duración completa como tiempo restante.

#### Scenario: El jugador 1 recoge un Nitro (3.4)
- **GIVEN** la config y la carrera del Background (`nitro_duration` 1 s), el generador que siempre da Nitro y el
  jugador 1 acelerando hacia el primer power-up
- **WHEN** el auto del jugador 1 recoge el power-up
- **THEN** el siguiente snapshot lo marca inactivo y muestra Nitro con 1 s restante en el jugador 1
- Test: `GameLoopPowerUpTest.CollectedPowerUpShowsInactiveAndNitroEffect` (`tests/test_game_loop.cpp`)

### Requirement: Los efectos avanzan con los ticks del loop
En cada tick de carrera, el servidor SHALL descontar `1 / game_fps` segundos de los efectos activos y de los cooldowns
de reaparición, sin usar el reloj de pared. Un efecto MUST seguir activo hasta que se descuenta su duración completa.

#### Scenario: El nitro expira al tick 200 (3.5)
- **GIVEN** la config del Background (`game_fps` 200, `nitro_duration` 1 s = 200 ticks) y el jugador 1 con un Nitro
  recién recogido
- **WHEN** el loop avanza 199 ticks
- **THEN** el snapshot sigue mostrando el Nitro activo en el jugador 1
- **WHEN** el loop avanza el tick 200
- **THEN** el snapshot ya no muestra efectos en el jugador 1
- Test: `GameLoopPowerUpTest.NitroExpiresAtTick200` (`tests/test_game_loop.cpp`)

### Requirement: Cada carrera arranca con los power-ups limpios
Al terminar una carrera, el servidor SHALL borrar sus power-ups y los efectos activos de los jugadores; la carrera
siguiente MUST empezar con todos sus power-ups activos y sin efectos en ningún jugador.

#### Scenario: Segunda carrera después de recoger un power-up (3.6)
- **GIVEN** la config del Background, dos carreras de test de 4 checkpoints, el generador que siempre da Nitro y el
  jugador 1 con un Nitro activo y un power-up recogido en la primera carrera
- **WHEN** la primera carrera termina y empieza la siguiente
- **THEN** el snapshot de la segunda carrera trae 2 power-ups activos y ningún efecto en el jugador 1
- Test: `GameLoopPowerUpTest.NextRaceStartsWithActivePowerUpsAndNoEffects` (`tests/test_game_loop.cpp`)
- Tests complementarios (la limpieza al terminar la carrera, también por timeout):
  `PowerUpEffectsTest.FinishRaceClearsEffects` y `PowerUpEffectsTest.ClearEffectsRemovesEffectsAndRestoresValues`
  (`tests/test_player.cpp`)

### Requirement: El cheat de vida infinita sigue activo en el loop
Dentro del loop, el cheat de vida infinita SHALL seguir protegiendo al jugador después de que expira un Escudo
recogido en la carrera.

#### Scenario: Cheat y escudo expirado en carrera (3.7)
- **GIVEN** la config del Background (`shield_duration` 1 s = 200 ticks), el generador que siempre da Escudo, el
  jugador 1 con el cheat de vida infinita activado y un Escudo recogido
- **WHEN** pasan 200 ticks y el auto del jugador 1 choca
- **THEN** la salud del jugador 1 en el snapshot no cambia
- Test: `GameLoopPowerUpTest.InfiniteHealthSurvivesShieldExpiryInLoop` (`tests/test_game_loop.cpp`)
- Test de control (la misma maniobra sin cheat baja la salud):
  `GameLoopPowerUpTest.CrashIntoWallWithoutCheatTakesDamage` (`tests/test_game_loop.cpp`)

### Requirement: Un cliente real recibe los power-ups recogidos
Un cliente conectado a un servidor real SHALL recibir por la red los snapshots con el estado de los power-ups y los
efectos de su jugador.

#### Scenario: El auto del cliente recoge un power-up (3.8)
- **GIVEN** un servidor real (en el rango de puertos propio del archivo) con la config del Background y el generador
  que siempre da Nitro, y un cliente conectado jugando la carrera de test de 4 checkpoints
- **WHEN** el auto del cliente recoge un power-up
- **THEN** el cliente recibe un snapshot con ese power-up inactivo y el efecto Nitro activo en su jugador
- Test: `ServerIntegrationPowerUpTest.ClientReceivesCollectedPowerUpInSnapshot` (`tests/test_server_integration.cpp`)
