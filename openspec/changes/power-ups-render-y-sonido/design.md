# Design

## Context

Ver `proposal.md` (Why) y `specs/power-ups/spec.md` (requirements y scenarios 4.1 – 4.12, más 3.1 y 3.4
modificados). Estado actual relevante:

- `WorldSnapshotDTO` (`common/world_snapshot_dto.h`) ya trae `power_ups` (`PowerUpDTO`: id `uint8`, tipo, x/y en
  píxeles, `active`) y `effects` (`PowerUpEffectDTO`: `player_id`, tipo, `remaining_ms`). `ClientProtocol` los
  deserializa sin validar el tipo (`client/client_protocol.cpp`: "the render decides what to draw").
- El servidor arma los power-ups del snapshot en `GameLoop::add_power_ups_to_snapshot` desde `PowerUpManager`;
  `PowerUpManager::try_collect(index, Player&)` llama a `PowerUp::collect(respawn_cooldown)` y `PowerUp::respawn`
  lo reactiva. Ningún lado guarda quién lo recogió. `Player::get_id()` existe.
- Cliente: `GameLogic` (`client/game_logic.*`) aplica los snapshots al `LocalInfo` (`client/game_state.h`) y pide
  cambios de pantalla a través de la interfaz `GameView`, que implementa `Game`. En `update_race` solo se aplica el
  **último** snapshot de cada tanda de la cola. `tests/client/test_game_logic.cpp` usa un `FakeView` y los helpers
  `car()` / `snapshot()`.
- `SDLManager::render(LocalInfo&)` dibuja mapa → NPCs/autos/puentes/decoraciones (`render_players`, con
  `camera.on_screen` para descartar lo que no se ve) → HUD de salud → explosiones → minimapa. Las texturas salen de
  `ResourceManager::get_texture(path)` y los textos de `get_text(...)` con `FONT_PATH`; los paths viven en
  `client/client_constants.h` (`/var/nfs/assets/...`).
- `Sounds::play_upgrade_sound()` ya reproduce `sfx_upgrade.wav`; `SDLManager::play_upgrade_sound()` lo expone.
- Los tests SDL (`tests/client/test_sdl_client.cpp`, fixture `SdlTest`) corren offscreen con audio `dummy` y se
  saltean si no está `/var/nfs/assets`. No hay forma de leer lo que quedó dibujado en el frame.
- Los sprites `nitro.png`, `life.png` y `shield.png` ya están en `client/assets/sprites/`, y `installer.sh` copia
  `client/assets/*` a `/var/nfs/assets/`.
- `nfs_client_tests` compila las fuentes del cliente que necesita listándolas en `CMakeLists.txt` (raíz); `nfs_client`
  las lista en `client/CMakeLists.txt`.

## Goals / Non-Goals

**Goals:**
- Que todo lo verificable del render (qué power-ups se dibujan, qué carteles hay, su texto y su posición) se pueda
  testear sin leer píxeles y sin ventana.
- Que el cliente decida el sonido con un dato exacto del servidor, sin inferir.
- No cambiar el comportamiento de lo que ya se dibuja ni de los tests existentes del cliente (salvo los de bytes del
  snapshot).

**Non-Goals:**
- Un framework general de "lista de dibujo" para todo el render: solo power-ups y carteles pasan por la capa pura.
- Interpolar posiciones o tiempos entre snapshots.

## Decisions

### D1 — `collected_by` en `PowerUpDTO`

`PowerUpDTO` suma `uint32_t collected_by = 0` (id del jugador; 0 = nadie desde que apareció). En la red va como
`uint32` **después** de `active`, en `ServerProtocol::send_world_snapshot` y `ClientProtocol::receive_update_positions`
(cada power-up pasa de 11 a 15 bytes). En el servidor, `PowerUp::collect(respawn_cooldown, player_id)` lo guarda
(`try_collect` pasa `player.get_id()`), `PowerUp::respawn` lo vuelve a 0 y `PowerUp::get_collected_by()` lo expone a
`GameLoop::add_power_ups_to_snapshot`.

Alternativas: inferir la recolección en el cliente (aparece o se renueva un efecto propio, o sube la salud) — no
detecta una Reparación con la salud llena ni distingue dos autos cerca, y mete lógica de juego en el cliente; un
evento aparte "power-up recogido" — más protocolo para un dato que entra en el snapshot y que, por estar en el
estado, sobrevive a los snapshots coalescidos. Elegida por el usuario.

Los ids de jugador son `uint32` en el resto del protocolo, por eso no se usa `uint8`.

### D2 — Estado de power-ups en el cliente

`LocalInfo` suma `std::vector<PowerUpDTO> power_ups` y `std::vector<PowerUpEffectDTO> effects`.
`GameLogic::update_game_state_from_snapshot` reemplaza `power_ups` con los del snapshot y `effects` con los del
snapshot **filtrados por `player_id == game_state.id`**. No hay timer en el cliente: el cartel muestra el
`remaining_ms` del último snapshot aplicado, así que expirar (4.5) y renovar (4.6) salen solos. El filtro hace que
los efectos de otros jugadores (4.8) nunca lleguen al HUD.

Alternativa: guardar los efectos en `PlayerInfo` de cada jugador — innecesario porque solo se muestran los propios.

### D3 — Sonido por flanco en `GameLogic`

`GameView` suma `virtual void play_power_up_sound() = 0`; `Game` lo implementa con `sdl.play_upgrade_sound()` y el
`FakeView` de los tests cuenta las llamadas (el "fake de sonido" del Background). Antes de reemplazar `power_ups`,
`update_game_state_from_snapshot` compara: por cada power-up nuevo con `!active && collected_by == game_state.id`, si
en el estado anterior el mismo id **no** estaba inactivo con `collected_by == game_state.id` (o no existía), llama a
`view.play_power_up_sound()`. Así suena una vez por recolección (4.9), nunca por otro jugador (4.10), y una tanda de
snapshots coalescidos en `update_race` no pierde la recolección porque el estado inactivo persiste hasta el
cooldown.

Alternativa: detectar en `SDLManager::render` como hoy los choques — `render` no es testeable con un contador sin
una ventana y mezcla la detección con el dibujo; `GameLogic` ya es el lugar de los eventos y tiene el fake.

### D4 — Capa de presentación pura: `client/power_up_view.{h,cpp}`

Módulo nuevo **sin SDL** (solo `game_state.h` y `client_constants.h`), compilado en `nfs_client` y en
`nfs_client_tests`:

- `std::optional<std::string> power_up_sprite(PowerUpType)` → `NITRO_SPRITE_PATH` / `LIFE_SPRITE_PATH` /
  `SHIELD_SPRITE_PATH`; `nullopt` para un tipo desconocido (4.11).
- `std::vector<PowerUpSprite> power_ups_to_draw(const LocalInfo&)` → `{id, sprite, x, y, size}` de cada power-up activo
  con sprite conocido, con `size = POWER_UP_RENDER_SIZE` (cuadrado) (4.1, 4.2, 4.11).
- `std::vector<EffectBadge> effect_badges(const LocalInfo&)` → por cada efecto propio Nitro o Escudo:
  `{type, sprite, text, x, y, w, h}`, con `text = ceil(remaining_ms / 1000) + "s"` en aritmética entera
  (`(ms + 999) / 1000`, 2400 → `3s`), en una fila sobre la barra de salud, cada cartel desplazado
  `EFFECT_BADGE_WIDTH + EFFECT_BADGE_SPACING` del anterior (4.3, 4.4). Orden estable: el del snapshot. El sprite
  del cartel también es cuadrado de `POWER_UP_RENDER_SIZE`, para que haya una sola medida de sprite.

`SDLManager` solo itera estas listas. Los tests de 4.3 – 4.8 usan `effect_badges` sobre el estado (los de
`test_game_logic.cpp` después de pasar snapshots por `GameLogic`), y los de 4.1, 4.2 y 4.11 usan `power_ups_to_draw`
y además llaman a `SDLManager::render` para verificar que el frame termina sin errores.

Alternativas: un contador de "cosas dibujadas" en `SDLManager` — acopla los tests a la cámara y al culling, y no da
textos ni rectángulos; leer píxeles del renderer offscreen — frágil y lento.

### D5 — Render en `SDLManager`

- `client_constants.h`: `NITRO_SPRITE_PATH`, `LIFE_SPRITE_PATH`, `SHIELD_SPRITE_PATH`
  (`/var/nfs/assets/sprites/{nitro,life,shield}.png`), `POWER_UP_RENDER_SIZE` y las medidas de los carteles
  (`EFFECT_BADGE_WIDTH`, `EFFECT_BADGE_HEIGHT`, `EFFECT_BADGE_SPACING`, `EFFECT_BADGE_X`, `EFFECT_BADGE_Y`).
- **Tamaño (primer acercamiento):** `POWER_UP_RENDER_SIZE = CAR_RENDER_SIZE`, es decir el ancho con que se dibuja
  en pantalla el sprite del auto Compact (celda de 50 px en la hoja, `CAR_SPRITE_SIZE`, dibujada a 100 px), en
  forma cuadrada (100 × 100). Se define en función de `CAR_RENDER_SIZE` para que siga al tamaño de los autos. El
  power-up se centra en su posición con un offset de `size / 2`, como `render_player`.
- **Sin fondo:** los PNG de los power-ups ya tienen transparencia; se copian tal cual con `renderer.Copy` (blending
  alfa de la textura), sin rectángulo ni color detrás, tanto en la pista como en el HUD.
- `render_power_ups(view_rect, scale_x, scale_y, state)`: dentro de `render_players`, después de los NPCs y antes de
  los autos de suelo (los autos pasan por encima del power-up), con la misma conversión mundo→pantalla y el mismo
  `camera.on_screen` que los autos.
- `render_hud_effects(state)`: en `render`, después de `render_hud_health`; por cartel el sprite (sin fondo)
  y al lado el texto (`get_text` con `FONT_PATH`).

### D6 — Tiempo, aleatoriedad y config en los tests de este hito

- **Tiempo:** el cliente no tiene reloj propio para los power-ups. El tiempo avanza solo con snapshots que el test
  arma y empuja a la cola de `GameLogic` (cada `update_race()` aplica el último) o asigna directo al `LocalInfo`; el
  render es una llamada explícita a `SDLManager::render`. No hay sleeps ni esperas.
- **RNG:** no aplica en el cliente: los tipos vienen fijos en el snapshot del Background. En el servidor, el test
  nuevo `RespawnedPowerUpHasNoCollector` y los modificados (3.4) siguen usando el generador "siempre Nitro" del
  Hito 3 inyectado en `GameLoop`.
- **Config:** los tests del cliente no leen config del servidor; todo sale del snapshot del Background del Hito 4
  (helpers locales en `test_game_logic.cpp` / `test_sdl_client.cpp`: power-ups 1 Nitro (100, 100), 2 Reparación
  (200, 100), 3 Escudo (300, 100), jugador propio 1 y otro 2). Los tests de servidor usan
  `make_fast_power_up_game_config()` (`tests/server_test_utils.*`) como en el Hito 3.

### D7 — Assets (4.12)

No hace falta cambiar la instalación: `installer.sh` ya copia `client/assets/*`. `PowerUpAssetsTest.SpritesAreInstalled`
verifica que los tres archivos existen en `/var/nfs/assets/sprites`, y se saltea, como el resto de los tests SDL, si
`/var/nfs/assets` no existe.

## Risks / Trade-offs

- [Los tests de render se saltean si no están instalados los assets] → los de 4.3 – 4.10 no necesitan SDL (capa
  pura y `GameLogic`), y en 4.1, 4.2 y 4.11 las aserciones sobre `power_ups_to_draw` corren siempre: solo el render
  del frame se saltea sin assets. En la máquina de desarrollo y en CI los assets están instalados.
- [Cambio de protocolo: cliente y servidor viejos no se entienden] → se distribuyen juntos (igual que en el Hito 3);
  se actualizan los tests de bytes y la documentación del protocolo al archivar.
- [Recolección, reaparición y nueva recolección dentro de una misma tanda coalescida] → el cooldown (segundos) es
  mucho mayor que el período de render; se acepta perder ese sonido.
- [El renderer de SDL ajusta el tamaño lógico y el texto puede medir distinto que el cartel] → el texto se escala al
  rectángulo del cartel, que es el que testea 4.4.

## Migration Plan

Sin migración de datos. Cliente y servidor se actualizan juntos; rollback = volver a la versión anterior de ambos.

## Open Questions

- Ajuste fino del tamaño (hoy `CAR_RENDER_SIZE`, primer acercamiento) y de la ubicación de los carteles: se revisa
  en la prueba manual cambiando solo constantes de `client_constants.h`, sin cambiar specs ni tests (4.1 compara
  contra `CAR_RENDER_SIZE` y 4.4 solo exige que los carteles no se superpongan).
