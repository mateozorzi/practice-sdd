# Design

## Context

Ver `proposal.md` (Why) y `specs/power-ups/spec.md` (requirements y scenarios 2.1 – 2.13). Estado actual relevante:

- Del Hito 1 ya existen `PowerUpConfig` (dentro de `GameConfig`), `PowerUpType` (`common/power_up_type.h`),
  `Player::apply_power_up(type, config)` (ignora autos muertos), `Player::advance_effects(double)`,
  `EFFECT_TIME_EPSILON` (`server/server_constants.h`) y los helpers `make_test_power_up_config()` /
  `make_power_up_test_server_config()` (`tests/server_test_utils.*`), cuyos valores coinciden con el Background del
  Hito 2.
- Los sensores existentes (checkpoints, puentes) se crean en `CityUtils` como cuerpos **estáticos** con
  `userData.pointer = dato << SENSOR_DATA_SHIFT | SensorType`. `CollisionListener::handle_sensor_contact` decodifica
  con `SENSOR_TYPE_MASK` y busca al jugador por cuerpo (`find_player_by_body`).
- `CityUtils::cleanup` destruye **todos** los cuerpos estáticos del mundo cuando `CityLoader` recarga una ciudad.
- `GameLoop` es dueño del `b2World`, del mapa `players` y de `CollisionListener{players}`, y avanza el mundo con
  `Step(GAME_STEP = 1/60 s)`. Conectar los power-ups al loop es del Hito 3.
- `tests/test_collision_listener.cpp` ya tiene un fixture con `add_player`, `square_at` y `step(n)`.
  `make_test_race(n, laps)` pone los checkpoints en x = 1700 + 70·id, y = 2391 (píxeles).
- `Car` crea su cuerpo con `linearDamping` del config: un auto sin input pierde velocidad solo.

## Goals / Non-Goals

**Goals:**
- Power-ups probables en un `b2World` de test con `Step` explícitos, sin game loop, sin red y sin reloj de pared.
- Una API de tiempo que el Hito 3 llame una vez por tick, igual que `Player::advance_effects`.
- No romper el comportamiento ni la API de `CollisionListener` para el código y los tests existentes.

**Non-Goals:**
- Exponer los power-ups en snapshots o DTOs (Hito 3). Solo se dejan getters que el Hito 3 va a leer.
- Volver a recoger un power-up que reaparece con un auto ya dentro del sensor (ver Risks).

## Decisions

### D1. `PowerUp`: estado de un power-up
En `server/power_up.h`:
```cpp
class PowerUp {
    PowerUpType type;
    float x, y;                    // píxeles, centro del sensor (para el DTO del Hito 3)
    bool active = true;
    double respawn_remaining = 0;  // s
    b2Body* body;
public:
    PowerUpType get_type() const; float get_x() const; float get_y() const;
    bool is_active() const; const b2Body* get_body() const;

    void collect(double respawn_cooldown);  // inactivo y arranca el cooldown
    bool advance(double elapsed_seconds);   // true cuando tiene que reaparecer
    void respawn(PowerUpType new_type);     // activo con el tipo nuevo
};
```
El cuerpo es **`b2_kinematicBody`** con un fixture sensor cuadrado de lado `2 * POWER_UP_HALF_SIZE_PIXELS` (nueva
constante en `server_constants.h`), convertido a metros con `config.world.pixels_per_meter`, filtro
`CATEGORY_SENSOR`/`MASK_SENSOR` y `userData.pointer = index << SENSOR_DATA_SHIFT | SensorType::POWER_UP`.

Kinematic y no estático para que `CityUtils::cleanup` no lo destruya por debajo: el dueño de los cuerpos de power-up
es su colección (D2), y así no quedan punteros colgando. Un sensor kinematic genera contactos con los autos
(dinámicos) igual que uno estático, y sin velocidad no se mueve.

*Alternativa descartada:* cuerpo estático como los checkpoints. Obliga a recrear los power-ups después de cada
`load_city` y a no tocar nunca sus cuerpos tras un cleanup; es un acoplamiento de orden frágil.

### D2. `PowerUpManager`: los power-ups de la carrera
```cpp
using PowerUpTypeGenerator = std::function<PowerUpType()>;

class PowerUpManager {
public:
    PowerUpManager(b2World& world, const ServerConfig& config, PowerUpTypeGenerator generator);
    void load(const std::vector<CheckpointDTO>& checkpoints);  // borra los anteriores y crea los nuevos
    bool try_collect(size_t index, Player& player);
    void advance(double elapsed_seconds);
    const std::vector<PowerUp>& get_power_ups() const;
    void clear();                                             // destruye sus cuerpos
    ~PowerUpManager();                                        // llama a clear()
};
```
`load` recorre los checkpoints por pares `(0,1), (2,3), …` del vector (tramos 1→2, 3→4) y crea un `PowerUp` en
`((P.x + Q.x) / 2, (P.y + Q.y) / 2)` con `type = generator()`. Con 4 o 5 checkpoints da 2 (2.1, 2.2). Se calcula
desde el `RaceDTO` que ya produce `RaceLoader`, así que `race_loader.*` no cambia (el roadmap lo lista como posible
archivo; no hace falta tocarlo). No copiable: dueño de cuerpos de Box2D.

### D3. RNG inyectable
- `PowerUpTypeGenerator` es un `std::function<PowerUpType()>`, inyectado por constructor.
- Real: `PowerUpTypeGenerator make_random_power_up_type_generator();` en `server/power_up.*`: captura un
  `std::mt19937` sembrado con `std::random_device` y una `std::uniform_int_distribution<int>(0, 2)` (uniforme sobre
  los 3 tipos). Lo usa el Hito 3 al construir el manager en el game loop; 2.5 lo llama 1000 veces.
- Falso: `std::function<PowerUpType()> make_sequence_power_up_type_generator(std::vector<PowerUpType> sequence);`
  en `tests/server_test_utils.*`. Es el mismo tipo que `PowerUpTypeGenerator`, escrito así porque
  `server_test_utils.h` también lo incluyen los tests del cliente y no puede incluir `server/power_up.h` (arrastra
  `car.h`, que choca con el `Car` del cliente). Devuelve los tipos en orden y vuelve a empezar al terminarse. El Background usa
  `{Shield, Nitro, Repair, Nitro}`.
- El tipo se sortea al crear y **al reaparecer** (no al recoger): así 2.13 ve Reparación, el tercero de la secuencia.

*Alternativa descartada:* una interfaz virtual `Random`. Más código para un único método; `std::function` permite
lambdas en los tests.

### D4. Recolección en `CollisionListener`
- `SensorType` (en `city_utils.h`) suma `POWER_UP`.
- `CollisionListener` suma un constructor `CollisionListener(players, PowerUpManager& power_ups)` y guarda un
  `PowerUpManager*` (nulo con el constructor actual, que no cambia). `GameLoop` lo empieza a usar en el Hito 3.
- En `handle_sensor_contact`, con `is_begin` y `SensorType::POWER_UP`: si hay manager y el cuerpo es de un jugador,
  `power_ups->try_collect(index, player)`. Se procesa antes del chequeo de capa, como los checkpoints. El `EndContact`
  de un power-up no hace nada.
- `try_collect`: si el power-up está inactivo o `player.is_dead()`, devuelve `false` sin cambios (2.9, 2.11). Si no,
  `active = false`, `respawn_remaining = power_ups.respawn_cooldown`, `player.apply_power_up(type, config.game.power_ups)`
  y devuelve `true` (2.8).
- Dos autos en el mismo paso (2.10): Box2D llama a `BeginContact` de a uno durante `Step`; el primero lo desactiva y
  el segundo lo encuentra inactivo. No hace falta lógica extra.
- “Desaparecer para todos” es `active = false`: el cuerpo no se deshabilita, porque durante el callback el mundo está
  bloqueado (`SetEnabled` falla) y no vale la pena diferirlo.

### D5. API de tiempo explícita: `PowerUpManager::advance(double elapsed_seconds)`
Único lugar donde corre el cooldown; sin `std::chrono` ni reloj de pared. A cada power-up inactivo le resta
`elapsed_seconds`; si queda `<= EFFECT_TIME_EPSILON`, lo reactiva con `respawn_remaining = 0` y `type = generator()`.
La posición y el cuerpo no cambian (2.13).
- En los tests: un helper `step(n)` del fixture hace, por cada paso, `world.Step(GAME_STEP, …)` y
  `power_ups.advance(GAME_STEP)`. 594 pasos dejan 0.1 s restantes (2.12); 600 pasos llegan a 0 dentro del epsilon
  (2.13). `GAME_STEP` es `float` (`1.0f/60` ≈ 0.016666668, un poco más que 1/60): 600 pasos suman ≈ 10.000001 s,
  así que el cooldown llega a 0 en el paso 600 y no antes (en el 599 quedan ≈ 0.0167 s). 2.12 y 2.13 cubren la
  frontera.
- En el Hito 3, el game loop lo llama una vez por tick junto con `Player::advance_effects`.

### D6. Config de test
Valores del Background del Hito 2 (`roadmap.md`) que usan los scenarios: power-ups 2.0 / 2 s / 30 / 4 s / 10 s;
paso de física fijo de 1/60 s; circuito `make_test_race(N, 1)` con posiciones leídas del `RaceDTO`; generador falso
`{Shield, Nitro, Repair, Nitro}`; autos de test 1 y 2 vivos con salud 100.

Se reutiliza `make_power_up_test_server_config()` (Compact con velocidad 10, aceleración 5, salud 100; power-ups
2.0 / 2 / 30 / 4 / 10), que pisa los valores del YAML aunque coincidan. No hace falta un helper nuevo de config.
- `tests/test_power_up.cpp`, fixture `PowerUpManagerTest`: `ServerConfig config` declarado antes del `b2World` y del
  manager (que guardan referencias), manager construido con el generador de secuencia y
  `make_test_race(4, 1)` / `make_test_race(5, 1)`. Las posiciones esperadas salen de `race.checkpoints`.
- `tests/test_collision_listener.cpp`: el fixture existente pasa a usar `make_power_up_test_server_config()` (el
  auto sigue siendo Compact; los tests de choque comparan contra la salud base, no contra números del YAML) y suma un
  `PowerUpManager`; el único listener del fixture se construye con el manager (`CollisionListener{players,
  power_ups}`), así que los tests existentes corren con un manager vacío. Los casos de power-ups ubican autos y power-ups en
  coordenadas propias del test (un “circuito” de 2 checkpoints armado en el test), no en las de una ciudad.
- 2.7: el test pone `SetLinearDamping(0)` en el cuerpo del auto, para que cualquier cambio de velocidad venga del
  contacto, y compara la velocidad antes y después de cruzar el sensor.
- 2.10: los dos autos se ubican a la misma distancia del sensor, a la misma velocidad y separados para no chocar
  entre sí.
- 2.11: el auto se mata con `apply_collision_damage(100)` antes de entrar al sensor.

## Risks / Trade-offs

- [Un auto parado dentro del sensor cuando reaparece no lo recoge] → Box2D solo dispara `BeginContact` al empezar el
  contacto. Se acepta y queda fuera de alcance (proposal); en carrera los autos se mueven.
- [Cuerpos kinematic en un mundo donde el resto de los sensores es estático] → Comentado en el código; `cleanup` no
  los toca y el manager los destruye en `clear()` / destructor. Riesgo de doble destrucción nulo porque nadie más los
  borra.
- [El destructor del manager toca el `b2World`] → El manager se declara **después** del mundo en fixtures y (Hito 3)
  en `GameLoop`, así se destruye antes.
- [Cambiar el fixture de `CollisionListenerTest`] → Los tests existentes son la regresión; si alguno depende del YAML,
  se mantiene un fixture aparte para los casos nuevos.
- [2.5 usa un RNG real] → La probabilidad de que en 1000 sorteos uniformes falte un tipo es ~3·(2/3)^1000, nula en
  la práctica. Es la excepción documentada al principio II.

## Migration Plan

Sin migración: no cambia la configuración, el protocolo ni los circuitos. El servidor en producción no crea
power-ups hasta que el Hito 3 conecte el manager al game loop.
