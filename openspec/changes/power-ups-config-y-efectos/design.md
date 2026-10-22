# Design

## Context

Ver `proposal.md` (Why) y `specs/power-ups/spec.md` (requirements y scenarios 1.1 – 1.17). Estado actual relevante:

- `ServerConfigLoader::load_game_config` (`server/server_config_loader.cpp`) lee `game:` y la subsección opcional
  `upgrades:` con `as<T>()`. Cualquier excepción termina envuelta en `std::runtime_error("Configuration loading
  error: ...")`, con el mensaje original adentro.
- `Car` (`server/car.*`) copia su `CarConfig` del modelo y **lo muta** con los upgrades (`set_upgrade` suma a
  `max_speed`/`acceleration`). `detect_linear_movement` usa `car_config.acceleration` y `car_config.max_speed`. `Car`
  guarda una **referencia** `const ServerConfig&`.
- `Player` (`server/player.*`) tiene `health`, `base_health` (salud máxima con upgrades) y `infinite_health` (cheat,
  que `start_race()` resetea a `false`). `apply_collision_damage` es el único camino que baja la salud: lo llama
  `collision_listener` en choques. `is_dead()` es `health == 0`; `update_car_position` marca `dead` y llama a
  `finish_race()`.
- `make_test_server_config()` (`tests/server_test_utils.cpp`) lee `server/config/*.yaml` del repo. Lo usan casi todos
  los tests del servidor, así que el YAML del repo **tiene** que incluir la nueva sección obligatoria.

## Goals / Non-Goals

**Goals:**
- Modelo de efectos en el servidor que se pueda probar sin Box2D stepping, sin game loop y sin red.
- Restaurar los valores exactos al expirar el nitro, sin errores acumulados de punto flotante.
- Una única API de tiempo que el Hito 3 pueda llamar por tick.

**Non-Goals:**
- Detectar contactos, spawnear o sortear el tipo (Hito 2). Serializar el tipo o los efectos (Hito 3).
- Cambiar la semántica del cheat de vida infinita o de los upgrades existentes.

## Decisions

### D1. `PowerUpConfig` dentro de `GameConfig`, sección obligatoria
```cpp
struct PowerUpConfig {
    float nitro_multiplier;
    double nitro_duration;      // s
    unsigned int repair_amount;
    double shield_duration;     // s
    double respawn_cooldown;    // s (se usa en el Hito 2)
};
// GameConfig { ...; UpgradeConfig upgrades; PowerUpConfig power_ups; }
```
`load_game_config` llama a un `load_power_ups(const YAML::Node& game, PowerUpConfig&)` nuevo, que:
- tira `std::runtime_error("Missing 'power_ups' node in game configuration file")` si falta `game.power_ups`;
- para cada clave tira `"Missing 'power_ups.<clave>' ..."` si falta y `"Invalid 'power_ups.<clave>': must be >= 0"`
  si es negativa (`nitro_multiplier` tiene que ser `> 0`). Lee con `as<double>()` para detectar negativos antes de
  convertir `repair_amount` a entero sin signo (`as<unsigned>` sobre `-30` no da un error claro).

Los tests chequean que el mensaje contenga la clave (`testing::HasSubstr`). `server/config/game_config.yaml` suma
`power_ups:` con los defaults 1.25 / 3 / 50 / 5 / 20. `installer.sh` ya copia `server/config/*` a `/etc/nfs`.

*Alternativa descartada:* sección opcional con defaults en código. Esconde errores de configuración y suma una segunda
fuente de defaults.

### D2. `PowerUpType` en `common/`
`enum class PowerUpType : uint8_t { Nitro = 0, Repair = 1, Shield = 2 };` en `common/power_up_type.h`. Solo el enum,
sin serialización. Va en `common/` porque el protocolo del Hito 3 lo necesita; así no hay que moverlo después. El
nombre no choca con nada del cliente (`Car`, `Receiver`, `Sender`).

### D3. Nitro como multiplicador, sin mutar `CarConfig`
`Car` gana `float power_multiplier = 1.0f` y:
- `void set_power_multiplier(float multiplier);`
- `float get_max_speed() const { return car_config.max_speed * power_multiplier; }`
- `float get_acceleration() const { return car_config.acceleration * power_multiplier; }`

`detect_linear_movement` pasa a usar esos getters. Como `car_config` (con upgrades) nunca se toca, al volver a
`1.0f` los valores son **exactamente** los previos (1.5, 1.17), y un upgrade comprado antes queda multiplicado (1.4:
(10 + 3) × 2 = 26). Renovar el nitro vuelve a poner el mismo multiplicador, así que no se multiplica dos veces (1.6).

*Alternativa descartada:* multiplicar `car_config` al activar y dividir al expirar. Acumula error de float y se rompe
si en el medio entra un upgrade.

### D4. Efectos en `Player`
Estado: el tiempo restante por tipo con duración (`double nitro_remaining = 0`, `double shield_remaining = 0`). Un
efecto está activo si su tiempo restante es `> 0`. API pública nueva:

```cpp
void apply_power_up(PowerUpType type, const PowerUpConfig& config);  // mismo estilo que set_upgrade
void advance_effects(double elapsed_seconds);
bool has_effect(PowerUpType type) const;          // Repair siempre false (instantáneo)
double get_effect_remaining(PowerUpType type) const;
float get_max_speed() const;                      // delega en Car (0 sin auto)
float get_acceleration() const;
```

Reglas (cada una mapea a un requirement de la spec):
- `apply_power_up` no hace nada sin auto o si `is_dead()` (1.15).
- **Nitro**: `nitro_remaining = nitro_duration` y `car->set_power_multiplier(nitro_multiplier)`.
- **Escudo**: `shield_remaining = shield_duration`. Es un estado propio, separado de `infinite_health` (1.13).
- **Reparación**: `health = min(health + repair_amount, base_health)` (1.8 – 1.10). Con el cheat, la salud ya está
  en el máximo y el daño se sigue ignorando (1.14).
- `apply_collision_damage`: sale temprano si `infinite_health` **o** el escudo está activo (1.11, 1.13, 1.16a). Si
  la salud llega a 0, llama a `clear_effects()` (1.16b).
- `clear_effects()` (privado): pone los tiempos en 0 y el multiplicador en 1. Lo llaman `start_race()` (1.17), la
  muerte, y `advance_effects` cuando expira el nitro.

### D5. API de tiempo explícita: `Player::advance_effects(double elapsed_seconds)`
Es el único lugar donde pasa el tiempo de los efectos; no hay reloj de pared ni `std::chrono::steady_clock`. Resta
`elapsed_seconds` a cada tiempo restante activo, y si queda `<= EFFECT_TIME_EPSILON` (`1e-6`, en
`server/server_constants.h`) lo pone en 0 y desactiva el efecto (con el nitro, el multiplicador vuelve a 1).
- En los tests: `advance_effects(1.9)`, `advance_effects(0.1)`, etc. (los segundos del Background).
- En el Hito 3, el game loop lo va a llamar una vez por tick con `1.0 / game_fps` (200 llamadas de 0.005 s = 1 s
  exacto gracias al epsilon).

*Alternativas descartadas:* contar ticks enteros (obliga a que `Player` conozca los fps y complica los tests en
segundos); `std::chrono::microseconds` (1/60 s no es exacto en µs, y se arrastra el mismo problema de redondeo).

### D6. RNG
No hace falta en este hito: el test elige el tipo y lo pasa a `apply_power_up`. El sorteo del tipo y la inyección del
RNG se definen en el `design.md` del Hito 2.

### D7. Config de test (valores del Background)
Los scenarios de la spec usan los valores del Background del Hito 1 (`roadmap.md`): power-ups 2.0 / 2 s / 30 / 4 s /
10 s; upgrades `health_upgrade` 50 y `speed_upgrade` 3; auto de test con velocidad máxima 10, aceleración 5 y salud
máxima 100. Son distintos a propósito de los defaults del juego, y cada test los fija explícitamente.

En `tests/server_test_utils.*`:
- `PowerUpConfig make_test_power_up_config();` → 2.0 / 2 / 30 / 4 / 10.
- `ServerConfig make_power_up_test_server_config();` → parte de `make_test_server_config()` y **pisa** lo que usan
  los scenarios, aunque coincida con el YAML: `physics.compact` con `max_speed` 10, `acceleration` 5 y `health` 100;
  `game.upgrades.health_upgrade` 50 y `speed_upgrade` 3; `game.power_ups = make_test_power_up_config()`.

Fixture `PowerUpEffectsTest` en `tests/test_player.cpp`: `ServerConfig config = make_power_up_test_server_config();`
declarado **antes** de `b2World` y `Player` (porque `Car` guarda una referencia a `config`), y `SetUp()` crea el auto
`"Compact"`. Las salidas intermedias (50, 90, 130) se arman con `apply_collision_damage` y upgrades de salud. Para los
tests 1.1 – 1.2 del loader, el YAML temporal se escribe con los valores del Background, extendiendo `MINIMAL_GAME`.

## Risks / Trade-offs

- [Romper tests y configs existentes por la sección obligatoria] → Se actualiza `server/config/game_config.yaml` y
  `MINIMAL_GAME` en el mismo commit que la validación; `make run-tests` lo detecta.
- [Precisión de punto flotante en las duraciones] → El tiempo restante es `double` y se compara con un epsilon de 1e-6.
  1.5 verifica la frontera 1.9 s / 2 s.
- [`Car` guarda una referencia a `ServerConfig`] → El fixture declara la config antes del `Player`; se documenta en el
  test.
- [Se cambia `detect_linear_movement`, que es lógica de física existente] → Con multiplicador 1, el comportamiento es
  idéntico; los `CarTest.*` existentes (límite de velocidad, upgrades) sirven de regresión.
- [`roadmap.md` tiene texto desactualizado] → El Background dice que 1.2 lee el YAML real, y la redacción original de
  1.16 no se puede cumplir con el escudo. La spec sigue la tabla y la aclaración del usuario sobre 1.16. Corregir
  `roadmap.md` queda para el usuario.

## Migration Plan

Sin migración de datos. Un `/etc/nfs/game_config.yaml` instalado sin `power_ups:` hace fallar al servidor al arrancar
con un mensaje claro; se arregla reinstalando la configuración con `installer.sh` o agregando la sección a mano.
