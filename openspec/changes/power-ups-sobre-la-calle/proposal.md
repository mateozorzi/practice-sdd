# Proposal

## Why

Hoy cada power-up se ubica en (P + Q) / 2, el punto medio en línea recta entre los dos checkpoints de su tramo. En
un circuito con curvas ese punto cae muchas veces fuera de la calle (23 de las 108 posiciones de las carreras de
`server/races/` quedan sobre edificios, agua o pasto), así que nadie lo puede agarrar. Este change implementa el
**Hito 5 de `roadmap.md`** ("Power-ups sobre la calle", criterios 5.1 – 5.13): el power-up pasa a ubicarse en el
punto medio del **camino por la calle** entre los dos checkpoints, recorriendo los puentes como lo haría un auto. Es
el gate para arrancar el Hito 6 (power-ups por capas en los puentes), que usa la capa de este camino.

## What Changes

- **Grilla de calles por ciudad**: un generador que corre una vez, fuera del juego, convierte las máscaras
  `B_STREETS` (copiadas de `editor/EditorMapas/imagenes/` a `server/tools/streets/`) en
  `server/world/streets/streets_<ciudad>.json`, una grilla de celdas transitables. Los tres JSON se generan, se
  versionan y se instalan en `/var/nfs/world/streets/` con el resto de `server/world/`.
- **Carga de la grilla**: el servidor lee el JSON de la ciudad (ruta nueva `streets:` por ciudad en
  `world_config.yaml`). Un archivo mal formado o con dimensiones que no coinciden falla con un error claro.
- **Camino por la calle con capas**: una búsqueda en anchura (BFS) sobre estados `(celda, capa)` entre los dos
  checkpoints del tramo, que reproduce la máquina de estados de `CollisionListener` con los sensores `ON_BRIDGE` /
  `UNDER_BRIDGE` de la ciudad: el camino que entra a un puente por arriba lo cruza por arriba y no atraviesa las
  barandas; el que pasa por debajo sigue por debajo; ninguno dobla dentro del rectángulo de un puente. Si un
  checkpoint no cae sobre la calle, el camino arranca (o termina) en la celda de calle más cercana.
- **Ubicación del power-up** (**modifica el criterio 2.3**): con grilla, el power-up va en el punto del camino que
  deja la mitad de su largo de cada lado. Sin grilla, o si no hay camino entre los checkpoints, sigue en
  (P + Q) / 2; en el segundo caso el servidor informa el tramo. Cargar una carrera nunca falla por esto, así que
  las carreras nuevas del editor (con checkpoints arbitrarios) siguen funcionando.
- **Datos fijos de test**: copia de las carreras de `server/races/` en `tests/data/races/`, para que crear o editar
  carreras después no cambie el resultado de los tests.

### Fuera de alcance

- Capa del power-up, recolección por capa, borde del puente y orden de dibujo respecto del puente (**Hito 6**). En
  este hito el power-up sigue siendo un sensor que recoge cualquier auto, como en el Hito 2, aunque quede dentro de
  un puente.
- Cambios en quién agarra un power-up, en el protocolo, en los DTOs, en el snapshot o en el cliente.
- Cualquier cambio en `editor/` (las máscaras se copian, no se mueven ni se editan) y en el formato de los
  circuitos (`server/races/*.csv`).
- Correr el generador desde el servidor o en la instalación: el servidor solo lee el JSON ya generado.
- Corregir la física de los puentes del juego: el camino modela cómo debe cruzar un auto, no cambia cómo se mueven.

## Capabilities

### New Capabilities
<!-- Ninguna: la grilla y el camino existen solo para ubicar power-ups, así que viven en la capability existente. -->

### Modified Capabilities
- `power-ups`: se modifica el requirement "El power-up aparece en el punto medio del tramo" (2.3: ahora es el punto
  medio del camino por la calle, con (P + Q) / 2 como respaldo) y se suman los requirements de la grilla de calles
  (carga y errores), del camino con capas (puentes por arriba y por debajo, sin doblar en un cruce, checkpoints fuera
  de la calle), del respaldo sin camino, de las carreras reales y de la instalación de las grillas. El resto de los
  requirements de los Hitos 1 – 4 no cambia.

## Impact

- **Servidor**: nuevo `server/streets_grid.*` (grilla, carga del JSON, puentes de la ciudad, camino y su punto
  medio); `server/power_up.*` (ubicación con grilla y respaldo); `server/city_loader.*` (carga y cachea la grilla y
  los puentes de la ciudad de la carrera); `server/game_loop.cpp` (pasa la grilla al cargar los power-ups);
  `server/server_config.h` y `server/server_config_loader.cpp` (clave `streets:`); `server/config/world_config.yaml`;
  `server/CMakeLists.txt`.
- **Datos y herramientas**: nuevos `server/tools/streets/` (máscaras copiadas y generador en C++ con `QImage`, target
  fuera de `all`) y `server/world/streets/streets_<ciudad>.json` (generados y versionados); `Makefile` (target para
  regenerar las grillas). `installer.sh` ya copia `server/world/*`, así que no cambia.
- **Tests**: nuevo `tests/test_streets_grid.cpp`, casos nuevos en `tests/test_power_up.cpp`, `tests/data/races/`
  (copia de las carreras), helpers en `tests/server_test_utils.*`, `tests/CMakeLists.txt`; un test del generador en
  `nfs_client_tests` (único ejecutable de tests con Qt).
- Sin dependencias nuevas (Qt ya está en el stack). Sin cambios en protocolo, cliente ni `editor/`.
