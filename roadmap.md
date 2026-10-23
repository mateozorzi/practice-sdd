## Roadmap y tests de aceptación

Cada hito tiene un **Background** y criterios Given/When/Then.

**Para avanzar de hito:** (a) todos los tests del hito existen y pasan, (b) `make run-tests`
en verde sin regresiones, (c) hooks de lint/cppcheck sin errores, (d) `/opsx:verify` sin CRITICAL.

### Background: datos de test independientes del servidor
Hoy `make_test_server_config()` (`tests/server_test_utils.cpp`) lee `server/config/*.yaml` del repo.
Si los tests de power-ups usaran esos valores, cambiar un default del juego rompería los tests.
Por eso:
- El Background de cada hito define **todos** los valores que usan sus escenarios. Los números de
  las tablas salen del Background, no del YAML.
- El `SetUp()` del fixture **pisa** los valores del config con los del Background (por ejemplo, con un
  helper `make_test_power_up_config()` en `tests/server_test_utils.*`), aunque coincidan con los del YAML.
- Los valores del Background son **distintos a propósito** de los defaults del juego
  (1.25 / 3 / 50 / 5 / 20). Si un test lee el YAML por error, falla.
- El tiempo de los efectos avanza solo por llamadas explícitas del test (pasos de física o ticks del
  loop), nunca por reloj de pared. El nombre de la API se define en `design.md`.
- El sorteo del tipo usa un RNG falso con una secuencia fija, salvo en el test que verifica que salen los 3 tipos.

### Hito 1 — Configuración y modelo de efectos en el servidor (sin física ni red)
Archivos esperados: `server/player.*`, `server/car.*`, config loader, `tests/test_player.cpp`,
`tests/test_server_config_loader.cpp`, `tests/server_test_utils.*`.

**Background**
```yaml
power_ups:
  nitro_multiplier: 2.0
  nitro_duration: 2        # s
  repair_amount: 30
  shield_duration: 4       # s
  respawn_cooldown: 10     # s
upgrades:
  health_upgrade: 50
  speed_upgrade: 3
auto_de_test:              # CarConfig armado en el test, no el de physics_config.yaml
  velocidad_maxima: 10
  aceleracion: 5
  salud_maxima: 100
```

| #    | Given / When / Then |
|------|---------------------|
| 1.1  | Dado un YAML temporal escrito por el test con los `power_ups:` del Background, cuando se carga, entonces el config tiene exactamente esos 5 valores |
| 1.2  | Dado un YAML temporal con un valor de `power_ups:` faltante o negativo, cuando se carga, entonces falla con un error claro |
| 1.3  | Dado el auto de test (vel. 10, acel. 5), cuando toma nitro, entonces vel. = 20 y acel. = 10 |
| 1.4  | Dado el auto con un upgrade de velocidad (vel. 13), cuando toma nitro, entonces vel. = 26 |
| 1.5  | Dado nitro activo, cuando pasan 1.9 s, entonces vel. sigue en 20; cuando llegan a 2 s, entonces vuelve exactamente a 10 y acel. a 5 |
| 1.6  | Dado nitro activo hace 1 s (le queda 1 s), cuando toma otro nitro, entonces le quedan 2 s y vel. = 20 |
| 1.7  | Dados nitro y escudo tomados en t = 0, entonces en t = 2 s el nitro expiró y el escudo sigue; en t = 4 s ninguno está activo |
| 1.8  | Dada salud 50/100, cuando toma reparación, entonces queda en 80 |
| 1.9  | Dada salud 90/100, cuando toma reparación, entonces queda en 100 |
| 1.10 | Dado un upgrade de salud (máx. 150) y salud 130, cuando toma reparación, entonces queda en 150 |
| 1.11 | Dada salud 100 y escudo activo, cuando recibe 20 de daño de choque, entonces la salud sigue en 100 |
| 1.12 | Dado escudo activo, cuando pasan 4 s y recibe 20 de daño, entonces la salud baja a 80 |
| 1.13 | Dado cheat de vida infinita, cuando toma escudo, pasan 4 s y recibe 20 de daño, entonces la salud no cambia |
| 1.14 | Dado cheat de vida infinita, cuando toma reparación y recibe 20 de daño, entonces la salud no cambia |
| 1.15 | Dado un auto muerto (salud 0), cuando toma cualquiera de los 3 power-ups, entonces no tiene efectos y la salud sigue en 0 |
| 1.16 | Dado un auto con nitro y escudo activos, cuando muere, entonces no tiene efectos |
| 1.17 | Dados nitro y escudo activos, cuando empieza otra carrera, entonces no hay efectos, vel. = 10 y acel. = 5 |

### Hito 2 — Power-ups en el mundo físico
Archivos esperados: `server/power_up.*` (nuevo), `server/city_utils.*` / `server/race_loader.*`,
`server/collision_listener.*`, `tests/test_power_up.cpp`, `tests/test_collision_listener.cpp`,
`tests/test_race_loader.cpp`.

**Background**
```yaml
power_ups:                 # mismos valores que el Hito 1
  nitro_multiplier: 2.0
  nitro_duration: 2
  repair_amount: 30
  shield_duration: 4
  respawn_cooldown: 10     # s
fisica:
  paso: 1/60 s             # world.Step fijo
circuito: make_test_race(N, 1)   # las posiciones se leen del RaceDTO, no se hardcodean
rng_falso: [Escudo, Nitro, Reparación, Nitro, ...]
autos: dos autos de test vivos (ids 1 y 2), salud 100
```

| #    | Given / When / Then |
|------|---------------------|
| 2.1  | Dado un circuito de 4 checkpoints, cuando se carga la carrera, entonces hay 2 power-ups |
| 2.2  | Dado un circuito de 5 checkpoints, cuando se carga la carrera, entonces hay 2 power-ups |
| 2.3  | Dado un circuito con los checkpoints 1 y 2 en P y Q (leídos del RaceDTO), entonces el primer power-up está en (P + Q) / 2 |
| 2.4  | Dado el RNG falso del Background, cuando se crean 2 power-ups, entonces el primero es Escudo y el segundo Nitro |
| 2.5  | Dado un RNG real, cuando se sortean 1000 tipos, entonces aparecen los 3 tipos |
| 2.6  | Dado un power-up, entonces su fixture es sensor |
| 2.7  | Dado el auto 1 a velocidad constante, cuando atraviesa un power-up, entonces su velocidad no cambia por el contacto y la salud sigue en 100 |
| 2.8  | Dado el auto 1 vivo, cuando entra al sensor de un power-up Escudo, entonces tiene escudo activo y el power-up queda inactivo |
| 2.9  | Dado un power-up inactivo, cuando el auto 2 entra al sensor, entonces el auto 2 no tiene efectos |
| 2.10 | Dados los autos 1 y 2 entrando al mismo sensor en el mismo paso, entonces exactamente uno tiene el efecto |
| 2.11 | Dado el auto 1 muerto, cuando entra al sensor, entonces el power-up sigue activo y el auto no tiene efectos |
| 2.12 | Dado un power-up recogido, cuando pasan 9.9 s (594 pasos), entonces sigue inactivo |
| 2.13 | Dado un power-up recogido, cuando pasan 10 s (600 pasos), entonces está activo en la misma posición y su tipo es el siguiente de la secuencia del RNG falso |

### Hito 3 — Game loop y protocolo
Archivos esperados: `server/game_loop.*`, `server/server_protocol.*`, `client/client_protocol.*`,
DTO nuevo en `common/`, `tests/test_game_loop.cpp`, `tests/test_common.cpp`,
`tests/test_server_integration.cpp`.

**Background**
```yaml
config: make_fast_game_config()   # game_fps 200, esperas en 0
power_ups:                        # duraciones cortas para que el loop termine rápido
  nitro_multiplier: 2.0
  nitro_duration: 1        # s = 200 ticks
  repair_amount: 30
  shield_duration: 1       # s = 200 ticks
  respawn_cooldown: 2      # s = 400 ticks
carrera: make_test_race(4, 1)     # 2 power-ups
rng_falso: siempre Nitro (en 3.7, siempre Escudo)
jugadores: 1 jugador (id 1); en 3.8, servidor real con RunningServer en el rango de puertos propio del archivo
dto_de_ejemplo: power-up {id 1, Nitro, (100, 200), activo}; efecto {Escudo, 2.5 s restantes}
```

| #   | Given / When / Then |
|-----|---------------------|
| 3.1 | Dado el DTO de power-up de ejemplo, cuando se serializa y deserializa, entonces es idéntico |
| 3.2 | Dado un snapshot con el efecto de ejemplo en el jugador 1, cuando se serializa y deserializa, entonces es idéntico (tipo y tiempo restante) |
| 3.3 | Dada la carrera en curso, cuando el loop avanza un tick, entonces el snapshot trae 2 power-ups activos de tipo Nitro |
| 3.4 | Dado que el jugador 1 recoge un power-up, entonces el siguiente snapshot lo marca inactivo y muestra Nitro con 1 s restante en el jugador 1 |
| 3.5 | Dado nitro activo, cuando el loop avanza 199 ticks, entonces sigue activo; al tick 200, entonces expiró |
| 3.6 | Dados un efecto activo y un power-up recogido, cuando la carrera termina y empieza la siguiente, entonces los 2 power-ups están activos y el jugador no tiene efectos |
| 3.7 | Dado el cheat de vida infinita activado y un escudo recogido, cuando pasan 200 ticks y el auto choca, entonces la salud no cambia |
| 3.8 | Dado un servidor real y un cliente conectado, cuando el auto del cliente recoge un power-up, entonces el cliente recibe un snapshot con el power-up inactivo y el efecto activo |

### Hito 4 — Render y sonido en el cliente
Archivos esperados: `client/sdl_manager.*`, `client/game.*`, `client/game_state.h`,
`tests/client/test_sdl_client.cpp`, `tests/client/test_game_logic.cpp`.

**Background** (los tests del cliente no usan config del servidor: todo viene del snapshot armado en el test)
```yaml
sdl: offscreen
jugador_propio: id 1
otro_jugador: id 2
power_ups_en_snapshot:
  - {id 1, Nitro,      (100, 100), activo}
  - {id 2, Reparación, (200, 100), activo}
  - {id 3, Escudo,     (300, 100), activo}
efectos_en_snapshot: los indica cada escenario (tipo + segundos restantes)
sonido: fake que cuenta las llamadas a play_upgrade_sound
```

| #    | Given / When / Then |
|------|---------------------|
| 4.1  | Dado el snapshot del Background, cuando se renderiza, entonces se dibujan los 3 sprites sin errores |
| 4.2  | Dado el power-up 2 inactivo, cuando se renderiza, entonces se dibujan solo los power-ups 1 y 3 |
| 4.3  | Dado el jugador 1 con Nitro y 2.4 s restantes, cuando se renderiza el HUD, entonces se muestra el cartel con el sprite de nitro y el texto `3 SEG` |
| 4.4  | Dado el jugador 1 con Nitro (1.0 s) y Escudo (4.0 s), cuando se renderiza el HUD, entonces se muestran los dos carteles (`1 SEG` y `4 SEG`) sin superponerse |
| 4.5  | Dado un snapshot con Nitro en 1.0 s y el siguiente sin efectos, cuando se renderiza el segundo, entonces no hay cartel |
| 4.6  | Dado un snapshot con Nitro en 0.5 s y el siguiente con Nitro en 3.0 s, cuando se renderiza el segundo, entonces el cartel muestra `3 SEG` |
| 4.7  | Dado que el jugador 1 recoge el power-up de Reparación, cuando se renderiza el HUD, entonces no hay cartel |
| 4.8  | Dado el jugador 2 con Escudo (4.0 s) y el jugador 1 sin efectos, cuando se renderiza el HUD, entonces no hay carteles |
| 4.9  | Dado que el jugador 1 recoge el power-up 1, cuando llega el snapshot, entonces el contador de sonido vale 1 (y sigue en 1 con los snapshots siguientes) |
| 4.10 | Dado que el jugador 2 recoge el power-up 1, cuando llega el snapshot, entonces el contador de sonido vale 0 |
| 4.11 | Dado un snapshot con un power-up de tipo desconocido (ej. 99), cuando se renderiza, entonces se ignora sin crashear |
| 4.12 | Los sprites `nitro.png`, `life.png`, `shield.png` se instalan en `/var/nfs` |

### Hito 5 — Power-ups sobre la calle
Modifica el criterio 2.3: el power-up ya no va en (P + Q) / 2 sino en el punto medio del **camino por la calle**
entre los dos checkpoints del tramo. El camino respeta los puentes igual que los autos: si entra a un puente por
arriba lo cruza por arriba y no puede bajar por la baranda; si pasa por debajo, sigue por debajo y no puede doblar
hacia la calle de arriba. Para eso el BFS recorre estados `(celda, capa)` y reproduce la máquina de estados de
`CollisionListener` con los sensores `ON_BRIDGE` / `UNDER_BRIDGE` de la ciudad. Este hito no cambia quién agarra un
power-up, ni el protocolo, ni el cliente.

Archivos esperados: `server/streets_grid.*` (nuevo: carga de la grilla y camino), `server/power_up.*`,
`server/race_loader.*` / config loader (ruta de la grilla en `world_config.yaml`), `server/tools/streets/`
(copia de las máscaras `B_STREETS` y el generador de la grilla), `server/world/streets/streets_<ciudad>.json`
(generado y versionado; se instala en `/var/nfs/world/streets/` con el resto de `server/world/`),
`tests/test_streets_grid.cpp`, `tests/test_power_up.cpp`.

Las máscaras se **copian** de `editor/EditorMapas/imagenes/` a `server/tools/streets/` (sin cambios en `editor/`).
El generador corre una vez, fuera del juego, y no forma parte del servidor: el servidor solo lee el JSON. El stack es
cerrado, así que el generador se escribe con lo que ya hay (por ejemplo C++ con `QImage` de Qt); el `design.md` lo
define.

**Background** (los tests arman una grilla y un cruce propios; no leen `server/world/streets/`, salvo 5.11)

Las carreras de 5.11 y 6.15 son una copia fija en `tests/data/races/`: las carreras que se creen o editen después
(el editor escribe en `/var/nfs/races`) no cambian el resultado de los tests.
```yaml
grilla_de_test:            # armada en el test, celdas de 10 px
  calle_recta: horizontal, de (0, 0) a (200, 0)
  calle_en_L: de (0, 0) a (200, 0) y de (200, 0) a (200, 200); (P + Q) / 2 = (100, 100) queda fuera de la calle
cruce_de_test:             # rectángulo de puente en (100..160, 100..160)
  camino_de_arriba: horizontal, con sensor ON_BRIDGE (40..220, 110..150) y barandas en y = 100 y y = 160
  camino_de_abajo: vertical, con sensor UNDER_BRIDGE (110..150, 40..220)
  rodeo: una calle que une el camino de arriba con el de abajo por fuera del cruce
power_ups: mismos valores que el Hito 2
rng_falso: el del Hito 2
```

| #    | Given / When / Then |
|------|---------------------|
| 5.1  | Dado un JSON temporal de grilla escrito por el test, cuando se carga, entonces la grilla tiene el tamaño de celda y las celdas transitables del archivo |
| 5.2  | Dado un JSON de grilla mal formado o con dimensiones que no coinciden con los datos, cuando se carga, entonces falla con un error claro |
| 5.3  | Dado un `PowerUpManager` sin grilla de calles, cuando se cargan los power-ups, entonces el primero sigue en (P + Q) / 2 (el criterio 2.3 y los tests del Hito 2 no cambian) |
| 5.4  | Dada la calle recta y P, Q sobre ella, cuando se cargan los power-ups, entonces el power-up está en (P + Q) / 2 |
| 5.5  | Dada la calle en L con P = (0, 0) y Q = (200, 200), cuando se cargan los power-ups, entonces el power-up está sobre la calle, en el punto que deja la mitad del largo del camino de cada lado: (200, 0) |
| 5.6  | Dado el cruce y P, Q a los dos lados del camino de arriba, cuando se calcula el camino, entonces sigue derecho por arriba, nunca entra al camino de abajo y su capa dentro del rectángulo del puente es `ON_BRIDGE` |
| 5.7  | Dado el cruce y P, Q a los dos lados del camino de abajo, cuando se calcula el camino, entonces atraviesa las barandas por debajo y su capa dentro del rectángulo del puente es `UNDER_BRIDGE` |
| 5.8  | Dado el cruce con P en el camino de arriba y Q en el de abajo, cuando se calcula el camino, entonces no dobla dentro del cruce: va por el rodeo |
| 5.9  | Dado P en una celda que no es calle, cuando se calcula el camino, entonces arranca desde la celda de calle más cercana a P |
| 5.10 | Dados P y Q sin camino entre ellos, cuando se cargan los power-ups, entonces el power-up queda en (P + Q) / 2 y se informa el tramo |
| 5.11 | Dadas las carreras fijas de `tests/data/races/` (copia de las de `server/races/` al crear el hito) y las grillas de `server/world/streets/`, cuando se cargan los power-ups, entonces todos quedan sobre una celda de calle y ningún tramo cae en el caso 5.10 |
| 5.12 | Dada una carrera nueva como las que crea el editor (checkpoints arbitrarios, incluso con tramos sin camino), cuando se carga, entonces no falla: cada power-up queda en el camino o, si no lo hay, en (P + Q) / 2 |
| 5.13 | Los archivos `streets_<ciudad>.json` de las 3 ciudades se instalan en `/var/nfs/world/streets/` y `world_config.yaml` apunta a ellos |

### Hito 6 — Power-ups por capas en los puentes
Un power-up que queda dentro del rectángulo de un puente toma la capa del camino del Hito 5 en ese punto. Si la carrera
pasa por arriba, el power-up está arriba; si pasa por debajo, está abajo. En un puente sobre el agua solo hay camino
por arriba, así que queda arriba. Nunca queda cruzando el borde del puente. Lo agarra solo un auto de su misma capa.
Deja sin efecto la exclusión del Hito 2 "power-ups sobre puentes: sin lógica por capa".

Archivos esperados: `server/power_up.*`, `server/collision_listener.*`, `common/power_up_dto.*`,
`server/server_protocol.*`, `client/client_protocol.*`, `client/sdl_manager.*`, `client/power_up_view.*`,
`tests/test_power_up.cpp`, `tests/test_collision_listener.cpp`, `tests/test_common.cpp`,
`tests/client/test_sdl_client.cpp`.

**Background**
```yaml
cruce_de_test: el del Hito 5 (rectángulo de puente en (100..160, 100..160))
puente_sobre_agua: rectángulo de puente con sensor ON_BRIDGE y sin sensor UNDER_BRIDGE
power_up: medio lado 15 px (POWER_UP_HALF_SIZE_PIXELS)
power_ups: mismos valores que el Hito 2
autos: dos autos de test vivos (ids 1 y 2), salud 100; la capa la fija el escenario
capas_del_power_up: ON_GROUND (fuera de todo puente), ON_BRIDGE, UNDER_BRIDGE
cliente: snapshot armado en el test con un rectángulo de puente en (100..160, 100..160), SDL offscreen
dto_de_ejemplo: power-up {id 1, Nitro, (130, 130), activo, ON_BRIDGE}
```

| #    | Given / When / Then |
|------|---------------------|
| 6.1  | Dado el cruce y un tramo por el camino de arriba cuyo punto medio cae dentro del puente, cuando se cargan los power-ups, entonces el power-up es `ON_BRIDGE` |
| 6.2  | Dado el cruce y un tramo por el camino de abajo cuyo punto medio cae dentro del puente, cuando se cargan los power-ups, entonces el power-up es `UNDER_BRIDGE` |
| 6.3  | Dado el puente sobre el agua y un tramo que lo cruza, cuando se cargan los power-ups, entonces el power-up es `ON_BRIDGE` |
| 6.4  | Dado un tramo cuyo punto medio queda fuera de todo puente, cuando se cargan los power-ups, entonces el power-up es `ON_GROUND` |
| 6.5  | Dado un tramo cuyo punto medio deja el cuadrado del power-up cruzando el borde del puente, cuando se cargan los power-ups, entonces se corre por el camino hasta la posición más cercana donde queda entero dentro o entero fuera, y su capa es la de esa posición |
| 6.6  | Dado un power-up `ON_BRIDGE`, cuando entra al sensor un auto `UNDER_BRIDGE`, entonces el power-up sigue activo y el auto no tiene efectos |
| 6.7  | Dado un power-up `ON_BRIDGE`, cuando entra al sensor un auto `ON_BRIDGE`, entonces lo recoge |
| 6.8  | Dado un power-up `UNDER_BRIDGE`, cuando entra al sensor un auto `ON_BRIDGE`, entonces sigue activo; cuando entra un auto `UNDER_BRIDGE`, entonces lo recoge |
| 6.9  | Dado un power-up `ON_GROUND`, cuando entra al sensor un auto con cualquier capa (por ejemplo `ON_BRIDGE` en la rampa), entonces lo recoge |
| 6.10 | Dado el cruce con sensores reales y un power-up `ON_BRIDGE`, cuando el auto 1 pasa por debajo con pasos de física y el auto 2 después por arriba, entonces lo recoge el auto 2 |
| 6.11 | Dado un power-up `UNDER_BRIDGE` recogido, cuando reaparece, entonces conserva su posición y su capa |
| 6.12 | Dado el DTO de ejemplo, cuando se serializa y deserializa, entonces es idéntico, capa incluida |
| 6.13 | Dado un power-up `ON_BRIDGE` en la carrera, cuando el loop avanza un tick, entonces el snapshot lo trae con capa `ON_BRIDGE` |
| 6.14 | Dado un snapshot con un power-up `ON_BRIDGE` y otro `UNDER_BRIDGE` dentro del puente, cuando se renderiza, entonces el `ON_BRIDGE` se dibuja después del puente y el `UNDER_BRIDGE` antes |
| 6.15 | Dadas las carreras fijas de `tests/data/races/`, cuando se cargan los power-ups, entonces ninguno cruza el borde de un puente y cada uno dentro de un puente tiene la capa del camino en ese punto |

### Criterios globales (Definition of Done de la feature)
- [ ] Todos los tests de los 6 hitos pasan; ningún test existente se rompe.
- [ ] Ningún test de power-ups depende de los valores de `server/config/*.yaml`.
- [ ] Cobertura (`make test`) de los archivos nuevos/modificados del servidor ≥ la actual.
- [ ] Sin cambios en `editor/`.
- [ ] `openspec/specs/` contiene la spec de power-ups archivada.
- [ ] Prueba manual
