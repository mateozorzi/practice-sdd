# Proposal

## Why

Después del Hito 3 los power-ups ya funcionan en una carrera real y viajan en el snapshot (`power_ups` y `effects`
de `WorldSnapshotDTO`), y el cliente los deserializa, pero no los usa: el jugador no ve los power-ups en la pista, no
sabe qué efecto tiene ni cuánto le queda, y no escucha nada al recogerlos. Este change implementa el **Hito 4 de
`roadmap.md`** ("Render y sonido en el cliente", criterios 4.1 – 4.12) y cierra la feature de punta a punta.

## What Changes

- **Power-ups en la pista**: el cliente dibuja cada power-up activo del snapshot con su sprite (`nitro.png`,
  `life.png`, `shield.png`); los inactivos no se dibujan y los de tipo desconocido se ignoran sin crashear.
- **Carteles de efecto en el HUD**: por cada efecto con duración del jugador propio (Nitro, Escudo) se muestra un
  cartel con el sprite del power-up y los segundos restantes redondeados hacia arriba (`3s`). Con dos efectos, los
  dos carteles van uno al lado del otro sin superponerse. El tiempo sale del snapshot: al expirar el cartel
  desaparece y al renovar vuelve a la duración completa. La Reparación no muestra cartel y los efectos de otros
  jugadores no se muestran.
- **Sonido al recoger**: cuando el jugador propio recoge un power-up suena una sola vez el sonido de las mejoras
  (`sfx_upgrade.wav`, `Sounds::play_upgrade_sound`); si lo recoge otro jugador no suena.
- **Quién lo recogió** (**BREAKING** de protocolo): `PowerUpDTO` suma `collected_by` (id del jugador que lo recogió,
  0 = nadie). Hoy el snapshot no permite saber quién recogió un power-up, y sin eso el cliente no puede decidir el
  sonido sin inferirlo (lo que no detectaría, por ejemplo, una Reparación con la salud llena). El servidor lo llena
  al recoger y lo vuelve a 0 al reaparecer. Cliente y servidor se distribuyen juntos.
- Los sprites de power-ups quedan instalados en `/var/nfs/assets/sprites` (ya los copia `installer.sh`; se verifica).

### Fuera de alcance

- Lógica de efectos o timers propios en el cliente: el cliente solo dibuja lo que manda el servidor y reproduce
  sonido.
- Sprites o sonidos nuevos: se usan los assets existentes de `client/assets/`.
- Mostrar los efectos de otros jugadores (ni en el HUD ni sobre sus autos).
- Animaciones de recolección o de reaparición de los power-ups.
- Cambios en la lógica de efectos (Hito 1), en spawn, sensor y reaparición (Hito 2) o en el game loop (Hito 3), salvo
  registrar quién recogió cada power-up.
- Cambios en `editor/`, en el formato de los circuitos y en los valores de `server/config/*.yaml`.
- Compatibilidad con clientes o servidores de versiones anteriores del protocolo.

## Capabilities

### New Capabilities
<!-- Ninguna: se extiende la capability existente. -->

### Modified Capabilities
- `power-ups`: suma los requirements de render en el cliente (sprites en la pista, carteles del HUD, sonido al
  recoger, tipos desconocidos, assets instalados) y modifica los requirements de protocolo y snapshot del Hito 3
  (3.1 y 3.4) para incluir quién recogió el power-up.

## Impact

- **Common**: `common/power_up_dto.*` (campo `collected_by`).
- **Servidor**: `server/power_up.*` (el power-up recuerda quién lo recogió), `server/game_loop.cpp` (lo manda en el
  snapshot), `server/server_protocol.cpp` (serialización).
- **Cliente**: `client/client_protocol.cpp` (deserialización), `client/game_state.h` (power-ups y efectos propios en
  el estado local), `client/game_logic.*` (actualización del estado y detección de la recolección propia),
  `client/game.*` (sonido), `client/sdl_manager.*` y `client/client_constants.h` (sprites y HUD), y un módulo nuevo de
  presentación sin SDL para los power-ups y los carteles (`client/CMakeLists.txt`). Además, tres funciones
  auxiliares existentes de `SDLManager` (`parse_file_data`, `get_car_sprite_rect`,
  `calculate_angle_to_checkpoint`) pasan a ser `static`, porque cppcheck lo exige al commitear `sdl_manager.*`;
  no cambia el comportamiento.
- **Tests**: `tests/client/test_sdl_client.cpp`, `tests/client/test_game_logic.cpp`; se actualizan
  `tests/test_common.cpp`, `tests/test_game_loop.cpp`, `tests/test_server.cpp` y `tests/test_client.cpp` por el
  formato nuevo del snapshot.
- Documentación del protocolo y del juego (README / manuales) al archivar. Sin dependencias nuevas ni cambios en
  `editor/`.
