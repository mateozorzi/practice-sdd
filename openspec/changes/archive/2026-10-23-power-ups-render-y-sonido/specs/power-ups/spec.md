# Spec Delta

## MODIFIED Requirements

### Requirement: Los power-ups viajan en el snapshot
El snapshot del mundo que el servidor manda a los clientes SHALL incluir cada power-up de la carrera con su id, su
tipo, su posición en píxeles, si está activo y el id del jugador que lo recogió (0 si nadie lo recogió desde que
apareció). Al serializarlo y deserializarlo por el protocolo, el cliente MUST obtener exactamente los mismos valores.

#### Scenario: Round-trip del DTO de power-up (3.1)
- **GIVEN** un snapshot que contiene el power-up de ejemplo del Background (id 1, Nitro, posición (100, 200), activo,
  sin recolector) y un segundo power-up id 2, Escudo, posición (300, 200), inactivo, recogido por el jugador 1
- **WHEN** el servidor lo serializa y el cliente lo deserializa
- **THEN** el cliente obtiene dos power-ups idénticos a los originales: id 1, Nitro, (100, 200), activo, recolector
  0; e id 2, Escudo, (300, 200), inactivo, recolector 1
- Test: `PowerUpProtocolTest.PowerUpDTORoundTrip` (`tests/test_common.cpp`)

### Requirement: Recoger un power-up en carrera se refleja en el snapshot
Cuando el auto de un jugador recoge un power-up durante la carrera, el snapshot de ese tick SHALL mostrar el power-up
inactivo con ese jugador como recolector y, si el efecto tiene duración, el efecto en ese jugador con la duración
completa como tiempo restante.

#### Scenario: El jugador 1 recoge un Nitro (3.4)
- **GIVEN** la config y la carrera del Background (`nitro_duration` 1 s), el generador que siempre da Nitro y el
  jugador 1 acelerando hacia el primer power-up
- **WHEN** el auto del jugador 1 recoge el power-up
- **THEN** el siguiente snapshot lo marca inactivo con recolector 1 y muestra Nitro con 1 s restante en el jugador 1
- Test: `GameLoopPowerUpTest.CollectedPowerUpShowsInactiveAndNitroEffect` (`tests/test_game_loop.cpp`)

## ADDED Requirements

### Requirement: Un power-up que reaparece no tiene recolector
Cuando un power-up recogido reaparece después del cooldown, el snapshot SHALL mostrarlo activo y sin recolector
(id 0).

#### Scenario: Reaparición después del cooldown en el loop
- **GIVEN** la config del Background del Hito 3 (`game_fps` 200, `respawn_cooldown` 2 s = 400 ticks), el generador
  que siempre da Nitro y el jugador 1 que acaba de recoger el primer power-up
- **WHEN** el loop avanza 400 ticks
- **THEN** el snapshot muestra ese power-up activo y con recolector 0
- Test: `GameLoopPowerUpTest.RespawnedPowerUpHasNoCollector` (`tests/test_game_loop.cpp`)

### Requirement: El cliente dibuja los power-ups activos
Durante la carrera, el cliente SHALL dibujar cada power-up activo del último snapshot en su posición, con el sprite
de su tipo: `nitro.png` para Nitro, `life.png` para Reparación y `shield.png` para Escudo. Los power-ups inactivos
MUST NOT dibujarse.

#### Scenario: Se dibujan los 3 power-ups del snapshot (4.1)
- **GIVEN** SDL offscreen, el jugador propio id 1 en (200, 100) y el snapshot del Background del Hito 4 con los
  power-ups {1, Nitro, (100, 100), activo}, {2, Reparación, (200, 100), activo} y {3, Escudo, (300, 100), activo}
- **WHEN** el cliente renderiza el frame
- **THEN** la lista de power-ups a dibujar contiene los ids 1, 2 y 3 con los sprites `nitro.png`, `life.png` y
  `shield.png`, cada uno cuadrado y del mismo ancho con que se dibuja un auto, y el render del frame termina sin
  errores
- Test: `PowerUpRenderTest.DrawsTheThreePowerUps` (`tests/client/test_sdl_client.cpp`)

#### Scenario: Un power-up inactivo no se dibuja (4.2)
- **GIVEN** el snapshot del Background del Hito 4 con el power-up 2 inactivo
- **WHEN** el cliente renderiza el frame
- **THEN** la lista de power-ups a dibujar contiene solo los ids 1 y 3
- Test: `PowerUpRenderTest.InactivePowerUpIsNotDrawn` (`tests/client/test_sdl_client.cpp`)

### Requirement: Los power-ups de tipo desconocido se ignoran
Si el snapshot trae un power-up con un tipo que el cliente no conoce, el cliente SHALL ignorarlo al dibujar y MUST
seguir renderizando el resto del frame sin fallar.

#### Scenario: Power-up de tipo 99 (4.11)
- **GIVEN** SDL offscreen y el snapshot del Background del Hito 4 más un power-up {4, tipo 99, (150, 100), activo}
- **WHEN** el cliente renderiza el frame
- **THEN** la lista de power-ups a dibujar contiene los ids 1, 2 y 3 pero no el 4, y el render termina sin errores
- Test: `PowerUpRenderTest.UnknownTypeIsIgnored` (`tests/client/test_sdl_client.cpp`)

### Requirement: El HUD muestra un cartel por efecto propio con duración
Mientras el jugador propio tenga un efecto con duración activo (Nitro o Escudo), su HUD SHALL mostrar un cartel por
efecto con el sprite del power-up y los segundos restantes del snapshot redondeados hacia arriba (por ejemplo `3 SEG`).
Con dos efectos, los carteles MUST mostrarse uno al lado del otro sin superponerse.

#### Scenario: Cartel de nitro con segundos redondeados hacia arriba (4.3)
- **GIVEN** el jugador propio id 1 con el efecto Nitro y 2.4 s (2400 ms) restantes en el snapshot
- **WHEN** se renderiza el HUD
- **THEN** se muestra un único cartel con el sprite `nitro.png` y el texto `3 SEG`
- Test: `PowerUpHudTest.NitroBadgeShowsCeiledSeconds` (`tests/client/test_sdl_client.cpp`)

#### Scenario: Dos carteles uno al lado del otro (4.4)
- **GIVEN** el jugador propio id 1 con Nitro (1.0 s restante) y Escudo (4.0 s restantes)
- **WHEN** se renderiza el HUD
- **THEN** se muestran dos carteles, Nitro con `1 SEG` y Escudo con `4 SEG`, y sus rectángulos en pantalla no se
  intersecan
- Test: `PowerUpHudTest.TwoBadgesSideBySide` (`tests/client/test_sdl_client.cpp`)

### Requirement: Los carteles siguen el tiempo que manda el servidor
El cliente SHALL tomar los efectos propios y su tiempo restante del último snapshot recibido, sin llevar un timer
propio: si el snapshot ya no trae el efecto, el cartel MUST desaparecer, y si trae un tiempo mayor (renovación), el
cartel MUST mostrar ese tiempo.

#### Scenario: El efecto expira y el cartel desaparece (4.5)
- **GIVEN** la carrera en curso y un snapshot con el jugador 1 con Nitro en 1.0 s
- **WHEN** llega un snapshot siguiente sin efectos y se actualiza el estado
- **THEN** el estado del cliente no tiene efectos propios y no hay carteles en el HUD
- Test: `PowerUpGameLogicTest.ExpiredEffectRemovesBadge` (`tests/client/test_game_logic.cpp`)

#### Scenario: El efecto se renueva (4.6)
- **GIVEN** la carrera en curso y un snapshot con el jugador 1 con Nitro en 0.5 s
- **WHEN** llega un snapshot siguiente con el jugador 1 con Nitro en 3.0 s
- **THEN** el cartel de Nitro muestra `3 SEG`
- Test: `PowerUpGameLogicTest.RenewedEffectShowsFullDuration` (`tests/client/test_game_logic.cpp`)

### Requirement: La Reparación y los efectos ajenos no muestran cartel
El HUD SHALL mostrar solo los efectos con duración del jugador propio: recoger una Reparación (instantánea) MUST NOT
mostrar cartel, y los efectos de otros jugadores MUST NOT mostrarse.

#### Scenario: El jugador propio recoge una Reparación (4.7)
- **GIVEN** la carrera en curso con el snapshot del Background del Hito 4
- **WHEN** llega un snapshot con el power-up 2 (Reparación) inactivo y recogido por el jugador 1, sin efectos
- **THEN** no hay carteles en el HUD
- Test: `PowerUpGameLogicTest.RepairShowsNoBadge` (`tests/client/test_game_logic.cpp`)

#### Scenario: Otro jugador con escudo (4.8)
- **GIVEN** la carrera en curso con los jugadores 1 (propio) y 2
- **WHEN** llega un snapshot con el jugador 2 con Escudo (4.0 s) y el jugador 1 sin efectos
- **THEN** no hay carteles en el HUD
- Test: `PowerUpGameLogicTest.OtherPlayersEffectsAreNotShown` (`tests/client/test_game_logic.cpp`)

### Requirement: Suena la mejora cuando el jugador propio recoge un power-up
Cuando un snapshot muestra que el jugador propio recogió un power-up, el cliente SHALL reproducir una vez el sonido de
las mejoras (`sfx_upgrade.wav`). Los snapshots siguientes que muestran la misma recolección MUST NOT repetirlo, y una
recolección de otro jugador MUST NOT reproducirlo.

#### Scenario: El jugador propio recoge el power-up 1 (4.9)
- **GIVEN** la carrera en curso con el snapshot del Background del Hito 4 y un fake de sonido que cuenta las
  llamadas a la reproducción del sonido de mejora
- **WHEN** llega un snapshot con el power-up 1 inactivo y recogido por el jugador 1
- **THEN** el contador vale 1
- **WHEN** llegan dos snapshots más con el power-up 1 todavía inactivo y recogido por el jugador 1
- **THEN** el contador sigue en 1
- Test: `PowerUpGameLogicTest.OwnPickupPlaysUpgradeSoundOnce` (`tests/client/test_game_logic.cpp`)

#### Scenario: Otro jugador recoge el power-up 1 (4.10)
- **GIVEN** la carrera en curso con el snapshot del Background del Hito 4 y el fake de sonido
- **WHEN** llega un snapshot con el power-up 1 inactivo y recogido por el jugador 2
- **THEN** el contador vale 0
- Test: `PowerUpGameLogicTest.OtherPlayerPickupPlaysNoSound` (`tests/client/test_game_logic.cpp`)

### Requirement: Los sprites de power-ups se instalan
La instalación del juego SHALL dejar los sprites `nitro.png`, `life.png` y `shield.png` en
`/var/nfs/assets/sprites`, que es donde el cliente los busca.

#### Scenario: Sprites instalados (4.12)
- **GIVEN** el juego instalado con `installer.sh` (los assets en `/var/nfs/assets`)
- **WHEN** se buscan los sprites de power-ups
- **THEN** existen `/var/nfs/assets/sprites/nitro.png`, `/var/nfs/assets/sprites/life.png` y
  `/var/nfs/assets/sprites/shield.png`
- Test: `PowerUpAssetsTest.SpritesAreInstalled` (`tests/client/test_sdl_client.cpp`)
