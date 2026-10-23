# Design

## Context

Ver `proposal.md` (Why) y `specs/power-ups/spec.md` (scenarios 2.3 y 5.1 – 5.13). Estado actual relevante:

- `PowerUpManager::load(checkpoints)` (`server/power_up.cpp`) pone cada power-up en (P + Q) / 2 de los tramos
  1→2, 3→4, … y crea su sensor kinematic. Lo llama `GameLoop::set_race` justo después de
  `CityLoader::load_city(race.city, race.checkpoints)`.
- La capa de un auto la maneja `CollisionListener::handle_sensor_contact`: con capa `ON_GROUND`, al **entrar** a un
  sensor `ON_BRIDGE` pasa a `ON_BRIDGE` y a uno `UNDER_BRIDGE` pasa a `UNDER_BRIDGE`; con otra capa, entrar a un
  sensor no cambia nada. Al **salir** del sensor de su misma capa vuelve a `ON_GROUND`. Las barandas
  (`on_bridge_collidables_<ciudad>.json`) solo chocan con autos `ON_BRIDGE` (`MASK_CAR_BRIDGE`).
- Los archivos de la ciudad (sensores, barandas, rectángulos de puentes en `object_rendering/bridges_<ciudad>.json`)
  son listas de polígonos de 4 vértices alineados a los ejes, en píxeles; `CityUtils::load_polygons_from_file` ya
  los lee. Las rutas salen de `CityLoaderConfig` (`world_config.yaml`), y `make_test_server_config()` las redirige
  de `/var/nfs/` al repositorio.
- Datos reales (Liberty City, puente 0): rectángulo (1584..1679, 2648..2791), sensor `ON_BRIDGE` (1592..1671,
  2630..2809), sensor `UNDER_BRIDGE` (1554..1709, 2648..2791). El sensor de abajo **contiene** todo el rectángulo del
  puente, así que la máquina de estados sola no impide que un auto de abajo doble hacia la calle de arriba dentro del
  cruce (al salir del sensor de abajo queda `ON_GROUND` sobre la rampa). Vice City tiene 18 puentes y solo 11
  sensores `UNDER_BRIDGE` (puentes sobre el agua).
- Máscaras `editor/EditorMapas/imagenes/00X - B_STREETS - <Ciudad>.png`: 4640 × 4672 RGBA, el mismo sistema de
  píxeles que los checkpoints. El editor considera calle el color (255, 36, 250) con tolerancia 30 por canal
  (`Creador::is_street`).
- El target `nfs_server` no linkea Qt; `nfs_client_tests` sí (Core, Widgets, Multimedia, Test).

## Goals / Non-Goals

**Goals:**
- Un camino determinista y probado con grillas y cruces chicos armados en código, sin física, sin red y sin leer
  `server/world/streets/` (salvo 5.11 y 5.13).
- Que el servidor siga sin depender de Qt: la imagen se procesa una sola vez, fuera del juego.
- Que la carga de una carrera nunca falle por la grilla (archivo faltante, mal formado, tramo sin camino).

**Non-Goals:**
- Guardar la capa en el power-up o filtrar la recolección por capa (Hito 6). `PathStep::layer` queda calculada y
  probada para que el Hito 6 la use.
- Caminos óptimos en distancia euclídea: el BFS minimiza la cantidad de pasos entre celdas vecinas (4 vecinos).

## Decisions

### D1. Formato de la grilla y convención de celdas
`server/world/streets/streets_<ciudad>.json`:
```json
{ "cell_size": 16, "width": 290, "height": 292, "rows": ["0011...", "..."] }
```
`rows` tiene `height` strings de `width` caracteres `'0'`/`'1'` (1 = calle). La celda (col, row) tiene su **centro**
en (col · s, row · s) y cubre [c − s/2, c + s/2) en cada eje; un punto cae en la celda (round(x / s), round(y / s)).
Así, en las grillas de test (s = 10) los puntos del Background (múltiplos de 10) son centros exactos, y los valores
esperados de 5.4, 5.5 y 5.9 son exactos.

Alternativas: celdas con esquina en (col · s, row · s) (los puntos del Background quedarían en esquinas y el punto
medio se correría media celda); run-length o binario (más chico pero ilegible y sin necesidad: ~85 KB por ciudad
con s = 16).

### D2. `StreetsGrid` y su carga (`server/streets_grid.*`)
```cpp
class StreetsGrid {
public:
    StreetsGrid(int cell_size, std::vector<std::string> rows);  // valida como el loader
    int get_cell_size() const; int get_width() const; int get_height() const;
    bool is_street(int col, int row) const;                     // false fuera de la grilla
    StreetPoint cell_center(int col, int row) const;
    std::optional<StreetCell> cell_at(StreetPoint point) const;              // nullopt fuera (5.11)
    std::optional<StreetCell> nearest_street_cell(StreetPoint point) const;  // 5.9
};
StreetsGrid load_streets_grid(const std::string& path);         // WorldCreationException
```
`load_streets_grid` usa nlohmann/json y lanza `WorldCreationException` con la ruta y el problema: JSON inválido,
campo faltante o de otro tipo, `cell_size <= 0`, cantidad de filas ≠ `height` (mensaje con `height`), largo de fila ≠
`width` (mensaje con `width`) o un carácter que no es `0`/`1`. El constructor desde filas existe para que los tests
armen grillas en código. `nearest_street_cell` recorre las celdas de calle y elige la de menor distancia euclídea
del centro al punto (empate: menor fila, después menor columna); sirve también para puntos fuera de la grilla.

### D3. Puentes de la ciudad: `BridgeLayout`
```cpp
struct StreetRect { float min_x, max_x, min_y, max_y; };
struct BridgeLayout {
    std::vector<StreetRect> bridges;            // object_rendering/bridges_<ciudad>.json
    std::vector<StreetRect> on_bridge_sensors;  // sensors/on_bridge_...
    std::vector<StreetRect> under_bridge_sensors;
    std::vector<StreetRect> on_bridge_walls;    // collidables/on_bridge_... (barandas)
};
struct StreetMap { StreetsGrid grid; BridgeLayout bridges; };
```
Cada polígono se convierte a su caja (min/max de los 4 vértices). Pertenencia de una celda: a un sensor o a un
puente si su **centro** está dentro de la caja (bordes incluidos); a una baranda si el **cuadrado** de la celda se
superpone con la caja (intervalos abiertos, para que una baranda de alto 0 en y = 100 bloquee solo la fila 100).
Los tests arman el cruce del Background directamente como `BridgeLayout` (barandas = cajas de alto 0).

### D4. Camino con capas: BFS sobre `(celda, capa, rumbo)`
```cpp
struct PathStep { int col, row; CarLayer layer; };
std::optional<std::vector<PathStep>> find_street_path(const StreetMap& map, StreetPoint from,
                                                      StreetPoint to);
```
- Inicio y fin: `nearest_street_cell` de P y de Q (D2). Sin celdas de calle → `nullopt`. Capa inicial según los
  sensores que contienen la celda de inicio: solo `ON_BRIDGE` → `ON_BRIDGE`, solo `UNDER_BRIDGE` → `UNDER_BRIDGE`,
  ninguno → `ON_GROUND`, los dos (checkpoint dentro de un cruce) → dos estados iniciales.
- Vecinos: las 4 celdas adyacentes que son calle.
- Capa al moverse de `c` a `n`, igual que `CollisionListener`: primero las salidas (si la capa es `ON_BRIDGE` y `n`
  sale del sensor `ON_BRIDGE` → `ON_GROUND`; ídem `UNDER_BRIDGE`), después las entradas (con `ON_GROUND`, entrar a un
  sensor `ON_BRIDGE` → `ON_BRIDGE`; si no, entrar a uno `UNDER_BRIDGE` → `UNDER_BRIDGE`). "Entrar" es pasar de una
  celda fuera de la unión de esos sensores a una adentro; con varios sensores superpuestos se usa la unión.
- Barandas: un camino con capa `ON_BRIDGE` antes o después del movimiento no puede entrar a una celda de
  baranda (la baranda frena al auto antes de que salga del sensor; mirando solo la capa resultante, salir del
  sensor hacia la baranda lo dejaría `ON_GROUND` y la atravesaría).
- Rumbo: al entrar al rectángulo de un puente se guarda la dirección del movimiento; mientras las dos celdas estén
  dentro del mismo rectángulo, el movimiento debe tener ese rumbo (no se dobla dentro del cruce, 5.8). Al salir se
  borra. Si el inicio está dentro de un rectángulo, el primer movimiento fija el rumbo.
- Se termina al sacar de la cola un estado en la celda de fin (cualquier capa). Los vecinos se exploran en orden
  fijo (N, S, E, O), así que el camino es determinista.

Esta regla de rumbo es lo que agrega el camino a la máquina de estados del listener: con los datos reales (sensor de
abajo ⊇ rectángulo, ver Context), solo con sensores el camino de abajo podría subir a la rampa. Alternativa
descartada: prohibir dentro del rectángulo las celdas fuera del sensor de la capa; funciona con el cruce de test
pero no con los datos reales.

Estados: `width · height · 3 capas · 5 rumbos` (≈ 1,3 M con s = 16); `parent` en un vector plano indexado por
estado, reservado una vez por búsqueda. La info de cada celda (sensores, baranda, puente) se calcula la primera vez
que la búsqueda la alcanza: describir toda la ciudad de entrada costaba ~75 ms por tramo con s = 8.

**Medición (5.11, build Debug, 108 tramos):** con s = 8, promedio 31 ms y máximo 164 ms (un tramo de 614 celdas);
con s = 16, promedio 7,5 ms y máximo 10 ms, y 5.11 sigue en verde. Se usa s = 16 (Risks).

### D5. Punto medio del camino
`StreetPoint path_midpoint(const StreetsGrid&, const std::vector<PathStep>&)`: polilínea por los centros de las
celdas del camino; se camina la mitad de su largo y se interpola en el segmento. Un camino de una celda devuelve su
centro. Como cada segmento une dos celdas de calle vecinas, el punto queda sobre una celda de calle (5.11). No se
incluyen P y Q en la polilínea: con un checkpoint fuera de la calle, el tramo hasta él podría sacar el punto medio
de la calle.

### D6. `PowerUpManager::load` con grilla y respaldo
```cpp
void load(const std::vector<CheckpointDTO>& checkpoints, const StreetMap* streets = nullptr);
```
`nullptr` → (P + Q) / 2, igual que hoy (2.3 / 5.3, los tests de los Hitos 2 – 4 no cambian). Con grilla: camino y
punto medio (D4, D5); sin camino → (P + Q) / 2 y una línea en `std::cerr`:
`[POWER_UPS] Stretch 1 -> 2 has no street path, using the straight midpoint` (ids de los checkpoints). 5.10 captura
`std::cerr` como `ThreadTest.ExceptionsInRunEndTheThread`. `load` no lanza por la ubicación (5.12).

### D7. Carga de la grilla de la ciudad en el servidor
- `world_config.yaml`: nueva clave `streets:` por ciudad (`/var/nfs/world/streets/streets_<ciudad>.json`).
  `CityLoaderConfig::streets_file` (`std::string`); el loader la lee si está (opcional: vacía = sin grilla), así los
  YAML temporales de `test_server_config_loader.cpp` no cambian.
- `StreetMap load_street_map(const CityLoaderConfig&, CityUtils&)` en `server/streets_grid.cpp` arma la grilla y el
  `BridgeLayout` con los archivos de la ciudad.
- `CityLoader` cachea un `std::optional<StreetMap>` por ciudad normalizada (se carga la primera vez que se corre una
  carrera en esa ciudad) y expone `const StreetMap* get_street_map() const` de la ciudad actual. Si `streets_file`
  está vacía o la carga lanza, loguea el error una vez y devuelve `nullptr`: la carrera sigue con (P + Q) / 2.
- `GameLoop::set_race`: `power_ups.load(race.checkpoints, city_loader.get_street_map())`.

### D8. Generador fuera del servidor (`server/tools/streets/`)
- Máscaras copiadas a `server/tools/streets/masks/streets_mask_<ciudad>.png` (sin tocar `editor/`).
- `streets_grid_generator.{h,cpp}`: `std::vector<std::string> classify_street_cells(const QImage&, int cell_size)`
  (celda calle ⇔ el píxel del centro (col · s, row · s) está a menos de 30 por canal de (255, 36, 250); el ancho es
  `ceil(w / s)`, ídem alto) y `write_streets_grid_json(path, cell_size, rows)`. No incluye headers del servidor.
- `generate_streets_grid.cpp` (main: máscara, salida, tamaño de celda) → target `nfs_streets_generator`
  (`EXCLUDE_FROM_ALL`, Qt6::Gui), así no cambia el build normal ni el del servidor.
- `make streets`: compila el target y regenera los tres JSON con s = 16 (`STREETS_CELL_SIZE`). Los JSON se
  versionan; `installer.sh` ya copia `server/world/*` a `/var/nfs/world/` (5.13).
- `nfs_client_tests` compila `streets_grid_generator.cpp` para `StreetsGeneratorTest.ClassifiesStreetPixels`
  (`tests/client/test_streets_generator.cpp`), con una `QImage` armada en el test.

Alternativas: script en Python con PIL (dependencia nueva, el stack es cerrado); leer el PNG en el servidor (sumaría
Qt o un decoder al servidor y procesaría 21 M píxeles por carrera).

### D9. Tiempo, RNG y config de test
- Este hito no agrega tiempo: el camino se calcula una vez en `load`. La reaparición sigue con la API explícita del
  Hito 2, `PowerUpManager::advance(double elapsed_seconds)` / `step(n)` del fixture; no hay reloj de pared.
- RNG: los tests usan `make_sequence_power_up_type_generator` con la secuencia del Hito 2 (`tests/server_test_utils.*`).
- Config: `make_power_up_test_server_config()` (valores del Background del Hito 2). Las grillas y el cruce se arman
  con helpers nuevos en `tests/streets_test_utils.{h,cpp}` (`make_straight_street_map()`, `make_l_street_map()`,
  `make_two_streets_map()`, `make_test_crossing_map()`), separados de `server_test_utils.h` porque incluyen headers
  del servidor (ese header evita incluirlos para que lo usen los tests del cliente).
- Cruce de test (s = 10): camino de arriba = filas y ∈ {110..150}, x ∈ [0, 260]; camino de abajo = columnas
  x ∈ {110..150}, y ∈ [0, 260]; rodeo = columna x = 260 (y ∈ [110, 260]) y fila y = 260 (x ∈ [110, 260]); puente,
  sensores y barandas como en el Background (barandas: y = 100 y y = 160, x ∈ [100, 160]).
- 5.11 usa `make_test_server_config()` solo por las **rutas** de `server/world/` (no lee valores de juego) y
  `RaceLoader` con `races_folder = tests/data/races/`.

## Risks / Trade-offs

- [Hay tramos reales sin camino (máscara con huecos, rumbo dentro de un puente en L, sensores raros)] → 5.11 lo
  detecta tramo por tramo con el nombre de la carrera; se corrige la regla o el tamaño de celda en este change, no se
  relaja el test.
- [El BFS tarda en el arranque de cada carrera] → medido en 5.11: con s = 8 un tramo llegaba a 164 ms (Debug), así
  que se pasó a s = 16 (máximo 10 ms) y la info de las celdas se calcula a demanda (ver D4).
- [Los tests de los Hitos 3 y 4 usan `make_test_race` en Liberty City y ahora el loop carga la grilla real] → la
  posición puede correrse algunos píxeles; `collect_first_power_up` lee la posición del snapshot. Si algún test se
  rompe, se revisa por qué (regresión real o dato fijo en el test) antes de tocarlo.
- [La regla de rumbo es más estricta que la física real: un auto podría doblar en un puente en L] → los puentes son
  rectángulos rectos; si 5.11 muestra un puente en L, se ajusta la regla.
- [Las máscaras quedan duplicadas en `editor/` y `server/tools/streets/`] → aceptado: no se toca `editor/`.

## Migration Plan

Sin migración de datos: los JSON nuevos se instalan con el resto de `server/world/` (`installer.sh`). Rollback:
quitar la clave `streets:` de `world_config.yaml` vuelve a (P + Q) / 2 sin recompilar.
