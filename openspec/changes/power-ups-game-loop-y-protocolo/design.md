# Design

## Context

Ver `proposal.md` (Why) y `specs/power-ups/spec.md` (requirements y scenarios 3.1 – 3.8). Estado actual relevante:

- De los Hitos 1 y 2 ya existen `Player::apply_power_up`, `Player::advance_effects(double)`,
  `Player::has_effect` / `get_effect_remaining`, `EFFECT_TIME_EPSILON`, `PowerUpManager` (`load`, `try_collect`,
  `advance`, `clear`, `get_power_ups`), `PowerUpTypeGenerator`, `make_random_power_up_type_generator()`,
  `CollisionListener(players, power_ups)` y, en tests, `make_sequence_power_up_type_generator()`.
- `GameLoop` (`server/game_loop.*`) es un `Thread`: `run()` fija `phase_start` y despacha a `handle_*_phase`, y cada
  una mide su espera con `steady_clock` y termina en `fixed_sleep`. En `IN_RACE` el orden es `process_commands` →
  `check_alive_players` → `update_positions` (arma y difunde el snapshot) → `box2d_world.Step(GAME_STEP)`. No tiene
  `PowerUpManager`, construye `collision_listener(players)` y nunca llama a `advance_effects`.
- `set_race` hace `load_city` y luego `start_race()` en cada jugador; `start_race` ya hace `clear_effects()`.
- `WorldSnapshotDTO` tiene `car_positions` y `npc_positions` con contadores `uint8`. `ServerProtocol::send_world_snapshot`
  y `ClientProtocol::receive_update_positions` serializan autos y después NPCs. `ServerProtocolTest.SendWorldSnapshot`
  y `ClientProtocolTest.ReceiveUpdatePositions_WithMock` fijan los bytes exactos.
- `Lobby` crea cada `Game`, que crea su `GameLoop`; `RunningServer` (tests) arma `Lobby` + `Acceptor` y esconde los
  headers del servidor.
- `Car::set_position_for_race` pone el auto 1 m antes del checkpoint 1 mirando al 2: acelerando cruza el primer
  power-up (punto medio 1→2 de `make_test_race`).
- Paredes del mundo: `server/world/object_rendering/decorations_*.json` (y `bridges_*.json`) son **solo render**:
  `RaceLoader` los manda como texto en el `RaceDTO` y el servidor no crea cuerpos con ellos. Las paredes físicas salen
  de `server/world/collidables/on_ground_collidables_*.json` (`CityLoader::load_collidables`). En la ruta de
  `make_test_race` (y = 2391, auto desde x ≈ 1740) la primera es el polígono x 1968–1999, y 2248–2447 de
  `on_ground_collidables_liberty_city.json`; los power-ups (x 1805 y 1945) quedan antes.
- El daño de choque contra una pared es `min(velocidad × STATIC_COLLISION_DAMAGE_MULTIPLIER,
  STATIC_COLLISION_DAMAGE_MAX)` truncado a `uint16`: un choque lento puede no hacer daño.
- `game_fps` es 60 en producción (igual que `GAME_STEP` = 1/60) y 200 en `make_fast_game_config()`.

## Goals / Non-Goals

**Goals:**
- Tests del loop (3.3 – 3.7) que avanzan de a un tick, en el hilo del test, sin arrancar el `Thread` ni dormir.
- Una única fuente de tiempo para efectos y cooldowns dentro del loop: el tick.
- Formato de red que el Hito 4 pueda leer sin más cambios de protocolo.
- No cambiar el comportamiento de las fases ni de los tests existentes del loop.

**Non-Goals:**
- Reemplazar el `steady_clock` de las esperas de fase (tienda, cuenta regresiva, timeout de carrera): en los tests
  rápidos esas esperas son 0 y no afectan a los power-ups.
- Unificar `GAME_STEP` con `1 / game_fps` (ver Risks).

## Decisions

### D1. DTOs en `common/power_up_dto.*`
```cpp
struct PowerUpDTO {
    uint8_t id;          // índice en la carrera + 1
    PowerUpType type;
    uint32_t x_position, y_position;  // píxeles, centro del sensor (como NpcDTO)
    bool active;
    bool operator==(const PowerUpDTO&) const = default;
};
struct PowerUpEffectDTO {
    uint32_t player_id;
    PowerUpType type;    // Nitro o Shield
    uint32_t remaining_ms;
    bool operator==(const PowerUpEffectDTO&) const = default;
};
```
El tiempo viaja en **milisegundos enteros** (2.5 s → 2500): el round-trip es exacto (3.2) y el protocolo existente
solo maneja enteros. El servidor convierte con `std::lround(remaining * 1000)`; el cliente (Hito 4) redondea a
segundos hacia arriba. `operator==` permite `EXPECT_EQ` sobre los DTOs completos.

*Alternativa descartada:* `float` de segundos en el wire: no hay helper de floats en `CommonProtocol` y el round-trip
depende de la representación binaria.

*Alternativa descartada:* meter los efectos dentro de `PlayerDTO`: `PlayerDTO` se usa en otros mensajes y cambiaría su
serialización en todos.

### D2. Snapshot y wire
`WorldSnapshotDTO` suma `std::vector<PowerUpDTO> power_ups` y `std::vector<PowerUpEffectDTO> effects`, con
`add_power_up` / `add_effect` (como `add_npc_position`). Se serializan **después de los NPCs**:
```
... autos ... npcs ...
uint8  power_up_count
  { uint8 id, uint8 type, uint32 x, uint32 y, uint8 active } × count
uint8  effect_count
  { uint32 player_id, uint8 type, uint32 remaining_ms } × count
```
Al final para que el prefijo del mensaje no cambie. `ServerProtocol::send_world_snapshot` y
`ClientProtocol::receive_update_positions` se extienden; los dos tests de bytes exactos existentes suman los dos
contadores en 0 (`SendWorldSnapshot`: +2 bytes; `ReceiveUpdatePositions_WithMock`: `0x00, 0x00` al final).

El cliente convierte el byte del tipo a `PowerUpType` con `static_cast` y **no lo valida ni tira excepción** si no es
0, 1 o 2. Así un tipo desconocido (por ejemplo 99) llega intacto al cliente, y filtrarlo es responsabilidad del
render: lo exige el criterio 4.11 del Hito 4 y este hito no debe impedirlo. El tipo de enum entero (`uint8_t`) hace
que el `static_cast` esté definido para cualquier valor de byte.

### D3. API de tiempo explícita: `GameLoop::tick()`
- `void tick();` público: hace lo que hoy hace una vuelta del `while` de `run()` (fijar `phase_start` si falta y
  despachar a la fase) **sin** `fixed_sleep`. `run()` queda `while (should_keep_running()) { inicio = now; tick();
  fixed_sleep(inicio); }`, así que el hilo se comporta igual. Los `fixed_sleep` salen de los `handle_*_phase`.
- Los tests construyen el `GameLoop`, agregan jugadores, mandan comandos a la cola y llaman `tick()` desde el hilo del
  test, sin `start()`. Las difusiones van a las colas de `GameMonitor`, que el test drena.
- Tiempo de efectos por tick: miembro `const double tick_seconds = 1.0 / game_fps`. Con `game_fps` 200, 200 ticks =
  1 s (Background). Ningún efecto ni cooldown usa `steady_clock`.

*Alternativa descartada:* inyectar un reloj falso en el loop. Cambia todas las fases y no hace falta: con
`make_fast_game_config()` las esperas son 0 y las fases avanzan solas en cada tick.

### D4. Orden de un tick de `IN_RACE`
1. `process_commands` (movimiento, cheats).
2. `check_alive_players`.
3. `advance_effects(tick_seconds)` en cada jugador y `power_ups.advance(tick_seconds)`.
4. `box2d_world.Step(...)`: la recolección ocurre acá, vía `CollisionListener{players, power_ups}`.
5. `update_positions`: arma el snapshot (autos, NPCs, power-ups y efectos) y lo difunde.

Avanzar antes del `Step` y difundir después hace que el snapshot del tick en que se recoge muestre la duración
completa (1000 ms, 3.4). Tras 199 ticks más quedan 5 ms (activo) y al tick 200 llega a 0 dentro de
`EFFECT_TIME_EPSILON` (expira, 3.5). El cambio de orden snapshot/`Step` es solo en `IN_RACE`; la cuenta regresiva
sigue igual (no hay recolección: los autos no se mueven).

Los efectos se arman en `update_positions` recorriendo los jugadores con `has_effect` / `get_effect_remaining` para
Nitro y Escudo; los power-ups, con `power_ups.get_power_ups()` (id = índice + 1).

### D5. Ciclo de vida de los power-ups
- Miembro `PowerUpManager power_ups` declarado **después** de `box2d_world` (se destruye antes) y **antes** de
  `collision_listener`, que se construye con `collision_listener(players, power_ups)`.
- `set_race`: después de `map_configuration` (que recrea la ciudad), `power_ups.load(race.checkpoints)`. Como
  `set_race` corre al entrar a `UPGRADE_SHOP`, los snapshots de la cuenta regresiva ya muestran los power-ups.
- Al salir de `IN_RACE` (a `LEADERBOARD` o `GAME_FINISHED`): `power_ups.clear()` y `clear_effects()` en **todos**
  los jugadores. `Player::clear_effects()` pasa a ser público para esto: cubre también a los que no llegaron cuando la
  carrera termina por timeout (a ellos no se les llama `finish_race()`).
- `Player::finish_race()` también llama a `clear_effects()`, y `start_race` repite la limpieza como red de seguridad.

### D6. RNG inyectable
- `GameLoop(..., const RaceCatalogueDTO&, PowerUpTypeGenerator type_generator =
  make_random_power_up_type_generator())`: el default mantiene el comportamiento de producción y los tests
  existentes.
- Para 3.8 el generador tiene que llegar a un loop creado por el lobby: `Lobby` y `Game` reciben un
  `PowerUpTypeGeneratorFactory = std::function<PowerUpTypeGenerator()>` opcional (default: factory del generador
  real), y `Lobby::create_new_game` crea un generador por partida. `RunningServer(port, config,
  std::function<std::function<PowerUpType()>()> factory = {})` lo pasa al `Lobby` (vacío → default). Se escribe sin
  el alias para que `server_test_utils.h` no incluya `server/power_up.h` (mismo motivo que en el Hito 2).

*Alternativa descartada:* sembrar el RNG desde la config. Agrega una clave al YAML y el test igual dependería de qué
salga con esa semilla.

### D7. Config de test y fixtures
- Helper nuevo `ServerConfig make_fast_power_up_game_config()` en `tests/server_test_utils.*`:
  `make_fast_game_config()` (fps 200, esperas 0) con `power_ups` pisados por los valores del Background del Hito 3
  (2.0 / 1 s / 30 / 1 s / 2 s), aunque coincidan o no con el YAML. También fija `timeout_seconds_race = 60`
  (distinto del default de 600 del YAML), porque la fase de carrera mide el timeout con reloj de pared y
  `make_fast_game_config()` lo hereda del YAML. No es un valor de ningún scenario: alcanza con que sobre para que
  ningún test de tick llegue al timeout (duran milisegundos).
- `tests/test_common.cpp`, suite `PowerUpProtocolTest` (3.1, 3.2): `ServerProtocol` serializa un `GameUpdateDTO`
  `SEND_UPDATE_POSITIONS` sobre un socket de test en memoria (un `ISocket` que escribe en un buffer y lee de él) y
  `ClientProtocol` lo deserializa; sin red. Ambos headers de protocolo pueden convivir (no incluyen `Car`, `Receiver`
  ni `Sender`; ya conviven en `test_server_integration.cpp`).
- `tests/test_game_loop.cpp`, fixture `GameLoopPowerUpTest`: config del helper, `make_test_race(4, 1)` (dos carreras
  para 3.6), `make_sequence_power_up_type_generator({Nitro})` (`{Shield}` en 3.7), jugador 1 con auto elegido, y
  `tick(n)` que llama `loop->tick()` n veces, drena la cola del jugador y devuelve el último `WorldSnapshotDTO`. Un
  helper lleva el loop hasta `IN_RACE` a fuerza de ticks.
  - 3.4 / 3.5: comando `SEND_CAR_MOVEMENT` `UP` y ticks hasta que el power-up 1 sale inactivo, con cota
    (`ASSERT` si no pasa en 2000 ticks); desde ahí se cuentan 199 y 200 ticks. Después de recoger, el test suelta
    `UP` y frena con `DOWN` hasta que el auto deja de avanzar, para no cruzar el power-up 2 (con "siempre Nitro"
    renovaría el efecto); 3.5 verifica con `ASSERT` que el power-up 2 sigue activo.
  - 3.6: recoger, `CHEAT_WIN_RACE`, ticks hasta el primer snapshot posterior al segundo `SEND_RACE`.
  - 3.7 (doble choque): el power-up 2 (x 1930–1960) está entre el 1 y la pared (x 1968), así que el auto no puede
    chocar sin cruzarlo, y con "siempre Escudo" eso renueva el escudo. Por eso: `CHEAT_INFINITE_HEALTH`; acelerar:
    recoge el Escudo 1, lo renueva con el 2 y choca la pared con escudo activo (ese choque no se mide); con el auto
    contra la pared, avanzar 200 ticks desde la renovación (`ASSERT` de que el escudo expiró; el power-up 2 recién
    reaparece a los 400 ticks); retroceder unos ticks con `DOWN` y volver a chocar con `UP` (`ASSERT` con cota de que
    el auto volvió a la pared). Se compara la salud del snapshot antes y después de ese segundo choque.
  - Control de 3.7, `GameLoopPowerUpTest.CrashIntoWallWithoutCheatTakesDamage`: la misma maniobra sin el cheat; la
    salud tiene que bajar. Prueba que el choque de 3.7 hace daño y que el test no pasa por un choque lento.
- `tests/test_server_integration.cpp`, fixture nuevo `ServerIntegrationPowerUpTest` (3.8). No se reutiliza
  `ServerIntegrationTest`: su `RunningServer` se arma con `make_fast_game_config()`, sin generador, y su carrera tiene
  3 checkpoints. El fixture nuevo tiene su propio `RunningServer` en un puerto de `next_port` (el mismo contador del
  archivo, así no se pisan los puertos), la config del helper, la factory "siempre Nitro" y la carrera
  `make_test_race(4, 1)`; reutiliza `TestClient` y el armado de partida del archivo. El cliente acelera y espera con
  `receive_until` / `wait_for_update` un snapshot con el power-up 1 inactivo. El timeout es solo de espera, no
  semántica.

## Risks / Trade-offs

- [`GAME_STEP` (1/60 s) ≠ `1 / game_fps` en tests (200)] → La física avanza más rápido que los efectos en los tests;
  es preexistente y en producción coinciden. Los scenarios cuentan ticks, no segundos de física.
- [Un choque lento contra la pared hace 0 de daño y 3.7 pasaría sin probar nada] → El test de control sin cheat
  exige que la misma maniobra baje la salud.
- [El auto no puede llegar a la pared sin cruzar el power-up 2, que con "siempre Escudo" renueva el escudo] → Doble
  choque: el primero (con escudo) no se mide; el segundo ocurre después de que expira el escudo y antes de que el
  power-up 2 reaparezca (400 ticks).
- [La pared depende de `on_ground_collidables_liberty_city.json`] → Es dato del mapa, no default de config; si cambia
  el mapa, el `ASSERT` de choque falla con un mensaje claro.
- [Cambio de wire **BREAKING**] → Cliente y servidor se compilan e instalan juntos; se documenta en README/manual al
  archivar.
- [Snapshot después del `Step` en `IN_RACE`] → Muestra posiciones del tick actual en vez del anterior; los tests
  existentes no dependen del desfase. Se valida con `GameLoopTest.*` en verde.
- [Mover `fixed_sleep` fuera de las fases] → Refactor sin cambio de comportamiento, cubierto por los tests existentes
  del loop y de integración.

## Migration Plan

Sin migración de datos ni de config. Despliegue: cliente y servidor nuevos a la vez. Rollback: volver ambos binarios.
