# Design

## Context

Ver `proposal.md` (Why) y `specs/power-ups/spec.md` (scenarios 6.1 – 6.15). Estado actual relevante:

- `PowerUpManager::load(checkpoints, const StreetMap*)` (`server/power_up.cpp`) ubica cada power-up con
  `spawn_point`: sin grilla o sin camino, (P + Q) / 2; con camino, `path_midpoint(grid, path)`. El `PathStep` de
  cada celda del camino ya trae su `CarLayer` (Hito 5, D4), pero `PowerUp` no guarda capa.
- `CollisionListener::handle_sensor_contact`: al **empezar** el contacto con un sensor `POWER_UP` llama a
  `PowerUpManager::try_collect(index, player)` y retorna; las transiciones de capa del auto son otros
  `BeginContact`/`EndContact` contra los sensores `ON_BRIDGE` / `UNDER_BRIDGE`. `try_collect` hoy solo mira si el
  power-up está activo y el auto vivo.
- `BridgeLayout::bridges` (`server/streets_grid.h`) ya tiene los rectángulos de los puentes de la ciudad como
  `StreetRect` (bordes incluidos); `make_test_crossing_map()` (`tests/streets_test_utils.*`) arma el cruce del
  Background.
- El snapshot serializa cada power-up como `id, type, x, y, active, collected_by` (`server_protocol.cpp`,
  `client_protocol.cpp`); `GameLoop::add_power_ups_to_snapshot` arma los `PowerUpDTO`.
- El cliente dibuja en `SDLManager::render_players`: NPCs, `render_power_ups` (todos), autos no `ON_BRIDGE`,
  `render_bridges` (recortes de la textura del mapa sobre los rectángulos de `bridges_<ciudad>.json`), autos
  `ON_BRIDGE`, decoraciones. `power_ups_to_draw(state)` (`client/power_up_view.*`) arma la lista sin SDL.
- `POWER_UP_HALF_SIZE_PIXELS = 15` (`server/server_constants.h`), el medio lado del sensor.

## Goals / Non-Goals

**Goals:**
- Capa y corrimiento por el borde calculados una sola vez en `load`, deterministas y probados con el cruce de test,
  sin física ni red.
- Que los Hitos 2 – 5 no cambien: sin grilla, sin camino o fuera de los puentes, el power-up es `ON_GROUND` y lo
  recoge cualquiera, como hoy.
- Que el orden de dibujo sea comprobable sin mirar píxeles.

**Non-Goals:**
- Recolección continua mientras el auto sigue dentro del sensor (ver proposal, Fuera de alcance).
- Cambiar la máquina de capas de los autos o el camino del Hito 5.

## Decisions

### D1. Capa en `PowerUp`
`PowerUp(PowerUpType, float x, float y, b2Body*, CarLayer layer = CarLayer::ON_GROUND)` y `CarLayer get_layer()`.
`respawn` no toca `x`, `y` ni `layer` (6.11). El valor por defecto deja intactos los usos actuales.

### D2. Recorrer el camino por distancia (`server/streets_grid.*`)
```cpp
float path_length(const StreetsGrid& grid, const std::vector<PathStep>& path);
// Punto de la polilínea por los centros de las celdas a `distance` px del inicio (acotado a [0, largo])
StreetPoint path_point_at(const StreetsGrid& grid, const std::vector<PathStep>& path, float distance);
```
`path_midpoint` pasa a ser `path_point_at(grid, path, path_length(...) / 2)`, así el Hito 5 no cambia.

### D3. Capa y borde del puente al ubicar (`server/streets_grid.*`)
```cpp
struct PathSpot { StreetPoint point; CarLayer layer; };
// Posición del power-up sobre el camino con su capa: el punto medio, o el más cercano a él cuyo
// cuadrado de medio lado `half_size` queda entero dentro o entero fuera de cada puente
std::optional<PathSpot> place_on_path(const StreetMap& map, const std::vector<PathStep>& path,
                                      float half_size);
```
- **Válido**: para cada rectángulo de `map.bridges.bridges`, el cuadrado queda entero dentro (intervalos cerrados)
  o no se superpone (intervalos abiertos: tocar el borde cuenta como fuera). Así, con varios puentes superpuestos
  (Vice City) también se exige no cruzar ningún borde.
- **Búsqueda**: desde `s* = largo / 2`, para `d = 0, 1, 2, …` px se prueba `s* + d` y después `s* − d` (empate:
  hacia Q), dentro de `[0, largo]`. El primer punto válido gana. En 6.5 da exacto (115, 130) y (85, 130), porque los
  segmentos son ejes de 10 px y `s*` es entero.
- **Capa**: si el cuadrado está dentro de algún puente, la del `PathStep` cuya celda es `grid.cell_at(punto)` (entre
  los dos extremos del segmento donde cae el punto); si esa capa es `ON_GROUND`, queda `ON_GROUND`. Si está fuera de
  todos los puentes, `ON_GROUND` aunque el camino tenga otra capa (rampa, 6.4).
- **Sin posición válida** en todo el camino (tramo muy corto metido en un borde): `nullopt`. `load` deja el punto
  medio con `ON_GROUND` y escribe en `std::cerr`
  `[POWER_UPS] Stretch <p> -> <q> crosses a bridge edge, keeping the path midpoint`. 6.15 detecta si pasa en datos
  reales.

Paso de 1 px: con largos de unos cientos a pocos miles de px son a lo sumo unos miles de evaluaciones por tramo, cada
una contra los ~20 puentes de una ciudad; despreciable al lado del BFS. Alternativa descartada: calcular
analíticamente los intervalos válidos por segmento; más rápido pero con más casos borde y sin necesidad.

Alternativa descartada para el borde: achicar o mover el cuadrado perpendicular al camino. Saca al power-up de la
calle y no lo pide el criterio.

### D4. `PowerUpManager::load` y `try_collect`
- `spawn_point` devuelve `PathSpot`: sin grilla o sin camino, `{(P + Q) / 2, ON_GROUND}` (2.3 / 5.3 / 5.10); con
  camino, `place_on_path(*streets, *path, POWER_UP_HALF_SIZE_PIXELS)` o el respaldo de D3.
- `try_collect`: además de activo y vivo, si `layer != ON_GROUND` exige `player.get_car_layer() == layer`. Devuelve
  `false` sin tocar nada si no coincide (6.6, 6.8). La capa que cuenta es la del auto en el `BeginContact` del sensor
  del power-up; el listener no cambia.

### D5. Protocolo y DTO
- `PowerUpDTO` suma `CarLayer layer` como último campo (y último parámetro del constructor, por defecto
  `ON_GROUND`); `operator==` sigue `= default`, así 3.1 compara también la capa. `common/power_up_dto.h` incluye
  `car_position_dto.h` (ya es `common/`).
- Servidor: `add_uint8_to_buffer(static_cast<uint8_t>(power_up.layer))` después de `collected_by`. Cliente:
  `recv_uint8()` en el mismo orden; un valor > 2 se toma como `ON_GROUND` (se dibuja debajo, como un tipo
  desconocido no rompe el render).
- `GameLoop::add_power_ups_to_snapshot` pasa `power_up.get_layer()`.

### D6. Render en dos pasadas (`client/power_up_view.*`, `client/sdl_manager.*`)
```cpp
enum class PowerUpDrawPass { BELOW_BRIDGES, ABOVE_BRIDGES };
// ON_BRIDGE → ABOVE_BRIDGES; ON_GROUND y UNDER_BRIDGE → BELOW_BRIDGES
std::vector<PowerUpSprite> power_ups_to_draw(const LocalInfo& state, PowerUpDrawPass pass);
```
`power_ups_to_draw(state)` sigue devolviendo todos (4.1, 4.2 no cambian) y `PowerUpSprite` suma `CarLayer layer`.
`render_power_ups` recibe la pasada: `BELOW_BRIDGES` donde se llama hoy (antes de los autos de abajo) y
`ABOVE_BRIDGES` después de `render_bridges` y antes de los autos `ON_BRIDGE`, así un auto de arriba se dibuja sobre
el power-up. 6.14 verifica las dos listas y que el frame renderiza sin errores con un JSON de puentes armado en el
test (un polígono (100..160, 100..160), mismo formato que `bridges_<ciudad>.json`). Alternativa descartada: grabar el
orden de las llamadas de SDL en el `SDLManager`; agrega estado de test al render sin necesidad, porque el orden de
las pasadas es fijo en `render_players`.

### D7. Tiempo, RNG y config de test
- **Tiempo**: no se agrega API. La reaparición (6.11) usa `PowerUpManager::advance(double elapsed_seconds)` a través
  del `step(n)` del fixture (`GAME_STEP` = 1/60 s, 600 pasos = 10 s); 6.10 avanza la física con `world.Step` en el
  `step(n)` de `CollisionListenerTest`; 6.13 usa `GameLoop::tick()` desde el hilo del test (`GameLoopPowerUpTest`).
  Nunca reloj de pared.
- **RNG**: `make_sequence_power_up_type_generator` con la secuencia del Hito 2 (`tests/server_test_utils.*`); 6.13
  usa la secuencia de un tipo del fixture del loop.
- **Config**: `make_power_up_test_server_config()` (valores del Hito 2: `respawn_cooldown` 10 s) y
  `make_fast_power_up_game_config()` en el loop. Las posiciones de 6.10 se pasan de px a metros con
  `config.world.pixels_per_meter` del propio test, como hace `square_at`.
- **Mapas**: `make_test_crossing_map()` (Hito 5) y un helper nuevo `make_water_bridge_map()` en
  `tests/streets_test_utils.*` (solo el camino de arriba, su sensor `ON_BRIDGE` y las barandas; sin sensor
  `UNDER_BRIDGE` ni camino de abajo).
- **Power-ups de puente en `CollisionListenerTest`**: helper `place_bridge_power_up(P, Q)` que llama a
  `power_ups.load(checkpoints, &crossing)` con el cruce de test; 6.6 – 6.9 fijan la capa del auto con
  `set_car_layer` (sin sensores de puente en el mundo, la capa no cambia sola) y lo mueven hacia el sensor. 6.10 crea
  los dos sensores del cruce con `CityUtils::add_sensor` y no fija capas.
- **Carrera del loop (6.13)**: `make_bridge_test_race()` en `tests/server_test_utils.*`: Liberty City, 4
  checkpoints, el tramo 1 → 2 sobre el camino de arriba del puente 0 (rectángulo (1584..1679, 2648..2791); su sensor
  `ON_BRIDGE` (1592..1671, 2630..2809) es largo en y, así que el camino de arriba es vertical). Los checkpoints
  exactos se eligen al implementar y el test chequea, antes del `THEN`, que el power-up cae dentro del rectángulo.
- **6.15**: `RaceLoader` sobre `tests/data/races/` y las rutas de `make_test_server_config()` (solo rutas de
  `server/world/`, excepción ya documentada en el Hito 5). Para cada power-up recalcula el camino del tramo y toma la
  capa del paso cuya celda es `cell_at(posición)`.

## Risks / Trade-offs

- [Un auto entra en el mismo paso al sensor `ON_BRIDGE` y al del power-up, y el orden de los `BeginContact` deja la
  capa vieja] → el power-up queda entero dentro del rectángulo del puente, a ≥ 15 px del borde, y los sensores de
  rampa empiezan antes del rectángulo; 6.10 lo cubre con sensores reales. Si aparece en la prueba manual, se revisa
  el orden en el listener.
- [Un auto que está tocando el sensor con la capa equivocada no lo recoge aunque cambie de capa adentro] → aceptado
  (Fuera de alcance); con el power-up entero dentro del puente, cambiar de capa ahí no es un movimiento posible.
- [Un tramo real no tiene posición válida en todo el camino] → `nullopt` y log (D3); 6.15 falla con carrera y tramo
  y se corrige en este change sin relajar el test.
- [Los tests de los Hitos 3 y 4 usan `make_test_race` en Liberty City con la grilla real y el corrimiento podría
  mover un power-up] → los checkpoints de `make_test_race` (y = 2391) no cruzan puentes; si alguno se rompe, se
  analiza la causa antes de tocarlo.
- [Cambio de formato del snapshot] → cliente y servidor se compilan y distribuyen juntos; 3.1 y 6.12 cubren el
  round-trip.

## Migration Plan

Sin migración de datos ni de config. El protocolo cambia en ambos extremos a la vez (se instalan juntos con
`installer.sh`). Rollback: revertir el change; no hay datos persistidos que dependan de la capa.
