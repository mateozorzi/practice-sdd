# Proposal

## Why

Desde el Hito 5 los power-ups quedan sobre la calle, y el camino que los ubica ya sabe si pasa por arriba o por
debajo de cada puente. Pero el power-up todavía no usa esa capa: un auto que pasa por debajo de un puente agarra el
power-up que está arriba, el cliente lo dibuja siempre debajo del puente (tapado aunque esté arriba) y un power-up
puede quedar cruzando el borde del puente, medio arriba y medio abajo. Este change implementa el **Hito 6 de
`roadmap.md`** ("Power-ups por capas en los puentes", criterios 6.1 – 6.15) y deja sin efecto la exclusión del
Hito 2 "power-ups sobre puentes: sin lógica por capa". Es el último hito de la feature de power-ups.

## What Changes

- **Capa del power-up**: cada power-up tiene una capa (`ON_GROUND`, `ON_BRIDGE` o `UNDER_BRIDGE`). Dentro del
  rectángulo de un puente toma la capa del camino del Hito 5 en ese punto (arriba o abajo; en un puente sobre el agua
  solo hay camino por arriba, así que queda `ON_BRIDGE`); fuera de todo puente es `ON_GROUND`. Sin grilla o sin
  camino (el respaldo (P + Q) / 2 del Hito 5) es `ON_GROUND`, así que los Hitos 2 – 5 no cambian.
- **Borde del puente**: si el cuadrado del power-up (medio lado 15 px) queda cruzando el borde del rectángulo de un
  puente, se corre por el camino hasta la posición más cercana donde queda entero dentro o entero fuera, y toma la
  capa de esa posición.
- **Recolección por capa** (**modifica** "Lo recoge el primer auto vivo que lo toca"): un power-up `ON_BRIDGE` o
  `UNDER_BRIDGE` lo recoge solo un auto de su misma capa; uno `ON_GROUND` lo recoge un auto de cualquier capa
  (por ejemplo, uno `ON_BRIDGE` en la rampa). Al reaparecer conserva posición y capa.
- **Protocolo** (**BREAKING** en el formato del snapshot, cliente y servidor se actualizan juntos):
  `PowerUpDTO` suma la capa y el snapshot la serializa después del recolector.
- **Render**: el cliente dibuja los power-ups `ON_BRIDGE` después del puente (encima) y el resto antes (debajo),
  como ya hace con los autos.
- **Datos reales**: con las carreras fijas de `tests/data/races/`, ningún power-up cruza el borde de un puente y cada
  uno dentro de un puente tiene la capa del camino en ese punto.

### Fuera de alcance

- Cambios en la física de los puentes o en la máquina de capas de los autos (`CollisionListener` para sensores
  `ON_BRIDGE` / `UNDER_BRIDGE`), en el camino del Hito 5 o en la grilla de calles.
- Cambiar la capa de un power-up durante la carrera: se fija al cargar la carrera y se conserva al reaparecer.
- Que un auto que ya está tocando el sensor de un power-up lo recoja al cambiar de capa sin salir del sensor: la
  recolección se decide al **entrar** al sensor, como en el Hito 2.
- Mostrar la capa en el HUD o en el minimapa, o sonidos distintos por capa.
- Cualquier cambio en `editor/` y en el formato de los circuitos (`server/races/*.csv`).
- Compatibilidad del protocolo con clientes viejos: cliente y servidor se distribuyen juntos.

## Capabilities

### New Capabilities
<!-- Ninguna: la capa es una propiedad del power-up, así que vive en la capability existente. -->

### Modified Capabilities
- `power-ups`: se modifican "Lo recoge el primer auto vivo que lo toca" (solo un auto que puede recogerlo por su
  capa), "El power-up reaparece tras el cooldown" (conserva la capa) y "Los power-ups viajan en el snapshot" (la capa
  en el DTO); se suman los requirements de la capa al cargar (arriba, abajo, puente sobre el agua, fuera de los
  puentes), del borde del puente, de la recolección por capa, de la capa en el snapshot del loop, del orden de
  dibujo respecto del puente y de las carreras reales. El resto de los requirements de los Hitos 1 – 5 no cambia.

## Impact

- **Servidor**: `server/power_up.*` (capa en `PowerUp`, cálculo de capa y corrimiento por el borde en `load`,
  filtro por capa en `try_collect`); `server/streets_grid.*` (helpers para recorrer el camino por distancia y
  consultar los rectángulos de los puentes); `server/game_loop.cpp` (capa en el `PowerUpDTO` del snapshot);
  `server/server_protocol.cpp`.
- **Común y cliente**: `common/power_up_dto.*` (campo `layer`); `client/client_protocol.cpp`;
  `client/power_up_view.*` (capa en `PowerUpSprite` y listas por pasada) y `client/sdl_manager.*` (dos pasadas,
  antes y después de `render_bridges`).
- **Tests**: casos nuevos en `tests/test_power_up.cpp`, `tests/test_collision_listener.cpp`,
  `tests/test_common.cpp`, `tests/test_game_loop.cpp` y `tests/client/test_sdl_client.cpp`; helper del puente sobre
  el agua en `tests/streets_test_utils.*`.
- Sin dependencias nuevas. Sin cambios en `editor/`, en la config YAML ni en los archivos de `server/world/`.
