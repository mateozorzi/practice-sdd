# Proposal

## Why

El Hito 1 dejó en el servidor la configuración de power-ups y el modelo de efectos sobre el jugador, pero todavía no
hay power-ups en la pista: nada los ubica, nada decide su tipo y ningún auto los puede recoger. Este change implementa
el **Hito 2 de `roadmap.md`** ("Power-ups en el mundo físico", criterios 2.1 – 2.13): los power-ups pasan a existir
en el mundo Box2D como sensores, y es el gate para arrancar el Hito 3 (game loop y protocolo).

## What Changes

- **Spawn**: al cargar los checkpoints de una carrera se crea un power-up en el punto medio de cada tramo impar
  (checkpoints 1→2, 3→4, …). Un circuito de 4 o de 5 checkpoints tiene 2 power-ups. Las posiciones se calculan, no se
  leen del mapa.
- **Tipo aleatorio**: el tipo (Nitro, Reparación, Escudo) se sortea de forma uniforme cada vez que un power-up
  aparece (al cargarse y al reaparecer), con un generador **inyectable** para que los tests sean deterministas.
- **Sensor**: cada power-up es un fixture sensor de Box2D; atravesarlo no frena ni daña al auto.
- **Recolección**: el primer auto **vivo** que entra al sensor de un power-up activo recibe su efecto (vía
  `Player::apply_power_up` del Hito 1) y el power-up queda inactivo. Si dos autos entran en el mismo paso de física,
  lo recoge uno solo. Un auto muerto no lo recoge y el power-up sigue activo.
- **Reaparición**: un power-up recogido vuelve a estar activo, en la misma posición y con un tipo nuevo sorteado,
  después de `respawn_cooldown` segundos de tiempo avanzado explícitamente (sin reloj de pared).
- Helpers de test: generador de tipos con una secuencia fija.

### Fuera de alcance

- Integración con el game loop (crear los power-ups al empezar cada carrera, avanzar el cooldown por tick, limpiarlos
  al terminar), DTOs, snapshots y protocolo (**Hito 3**).
- Render, HUD y sonido en el cliente (**Hito 4**).
- Power-ups sobre puentes: los sensores se tratan como en el suelo, igual que los checkpoints; no hay lógica por capa.
- Recoger un power-up que reaparece con un auto ya parado encima: el auto tiene que salir y volver a entrar.
- Cualquier cambio en `editor/` y en el formato de los circuitos (`server/races/*.csv`).

## Capabilities

### New Capabilities
<!-- Ninguna: se extiende la capability existente. -->

### Modified Capabilities
- `power-ups`: suma los requirements del mundo físico: ubicación de spawn, sorteo del tipo con generador inyectable,
  sensor sin respuesta física, recolección por el primer auto vivo y reaparición tras `respawn_cooldown`. Los
  requirements del Hito 1 no cambian.

## Impact

- **Servidor**: nuevo `server/power_up.*` (power-up, colección de power-ups de la carrera y generador de tipos),
  `server/city_utils.h` (nuevo `SensorType::POWER_UP`), `server/collision_listener.*` (recolección al entrar al
  sensor), `server/server_constants.h` (tamaño del sensor), `server/CMakeLists.txt`.
- **Tests**: nuevo `tests/test_power_up.cpp`, casos nuevos en `tests/test_collision_listener.cpp`,
  `tests/server_test_utils.*` (generador con secuencia fija), `tests/CMakeLists.txt`.
- `server/race_loader.*` no cambia: el spawn se calcula a partir de los checkpoints del `RaceDTO` que ya produce.
- Sin dependencias nuevas. Sin cambios en protocolo, cliente, configuración YAML ni `editor/`.
