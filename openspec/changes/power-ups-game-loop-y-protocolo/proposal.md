# Proposal

## Why

Los Hitos 1 y 2 dejaron en el servidor la configuración, el modelo de efectos y los power-ups como sensores Box2D,
pero nada de eso está conectado a la partida: `GameLoop` no crea power-ups, no avanza los efectos ni los cooldowns, y
los snapshots no los mandan al cliente. Este change implementa el **Hito 3 de `roadmap.md`** ("Game loop y
protocolo", criterios 3.1 – 3.8): los power-ups pasan a funcionar en una carrera real y viajan por la red, que es lo
que necesita el Hito 4 para dibujarlos.

## What Changes

- **DTOs nuevos en `common/`**: un DTO de power-up (id, tipo, posición en píxeles, activo) y un DTO de efecto activo
  de un jugador (jugador, tipo, tiempo restante).
- **Snapshot extendido** (**BREAKING** de protocolo): `WorldSnapshotDTO` suma la lista de power-ups de la carrera y
  la de efectos activos de los jugadores; el servidor los serializa y el cliente los deserializa. Un cliente viejo no
  entiende el snapshot nuevo (cliente y servidor se distribuyen juntos).
- **Power-ups en el game loop**: al empezar cada carrera se crean los power-ups de sus checkpoints; durante la carrera
  los autos los recogen al pasar; al terminar se borran y la siguiente carrera arranca con todos activos y sin
  efectos.
- **Tiempo por tick**: en cada tick de carrera el loop avanza los efectos de los jugadores y los cooldowns de
  reaparición en `1 / game_fps` segundos, sin reloj de pared.
- **API de tick explícita** en `GameLoop` para que los tests avancen el loop de a un tick, sin arrancar el hilo.
- **RNG inyectable** en `GameLoop` y, para el test de integración, desde el lobby del servidor; por defecto se usa el
  generador uniforme real del Hito 2.
- Helper de test con la config rápida del juego y los power-ups del Background del Hito 3.

### Fuera de alcance

- Render de sprites, HUD con los carteles de efecto y sonido al recoger (**Hito 4**). El cliente solo deserializa los
  datos nuevos; no los usa todavía.
- Cambios en la lógica de efectos (Hito 1) o en spawn, sensor y reaparición (Hito 2), salvo conectarlos al loop.
- Recoger un power-up que reaparece con un auto parado encima (ya fuera de alcance en el Hito 2).
- Cambios en `editor/`, en el formato de los circuitos y en los valores de `server/config/*.yaml`.
- Compatibilidad con clientes de versiones anteriores del protocolo.

## Capabilities

### New Capabilities
<!-- Ninguna: se extiende la capability existente. -->

### Modified Capabilities
- `power-ups`: suma los requirements de game loop y protocolo: power-ups y efectos en el snapshot (y su
  serialización), recolección dentro del loop, avance del tiempo por tick, limpieza entre carreras, cheat de vida
  infinita en el loop y entrega a un cliente real. Los requirements de los Hitos 1 y 2 no cambian.

## Impact

- **Common**: nuevo `common/power_up_dto.*`, `common/world_snapshot_dto.*`, `common/CMakeLists.txt`.
- **Servidor**: `server/game_loop.*` (manager de power-ups, tick explícito, avance de efectos, snapshot),
  `server/server_protocol.cpp`, `server/game.*` y `server/lobby.*` (paso del generador de tipos),
  `server/player.*` (los efectos se borran al terminar la carrera; `clear_effects()` pasa a ser público).
- **Cliente**: `client/client_protocol.cpp` (deserializa los campos nuevos; sin cambios de render).
- **Tests**: `tests/test_common.cpp`, `tests/test_game_loop.cpp`, `tests/test_server_integration.cpp`,
  `tests/server_test_utils.*`; se actualizan `tests/test_server.cpp` y `tests/test_client.cpp`, que verifican los
  bytes exactos del snapshot.
- Documentación del protocolo (README / manuales) al archivar. Sin dependencias nuevas ni cambios en `editor/`.
