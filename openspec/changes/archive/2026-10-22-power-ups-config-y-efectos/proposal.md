# Proposal

## Why

La feature de power-ups (Nitro, Reparación, Escudo) necesita primero una base en el servidor: valores configurables y
un modelo de efectos sobre el jugador y su auto que se pueda probar de forma determinista, sin física de sensores ni
red. Este change implementa el **Hito 1 de `roadmap.md`** ("Configuración y modelo de efectos en el servidor",
criterios 1.1 – 1.17), que es el gate para arrancar el Hito 2.

## What Changes

- Nueva sección **obligatoria** `game.power_ups` en `server/config/game_config.yaml` (`nitro_multiplier`,
  `nitro_duration`, `repair_amount`, `shield_duration`, `respawn_cooldown`) con los defaults del juego
  (1.25 / 3 / 50 / 5 / 20). El loader falla con un error claro si falta la sección o una clave, o si un valor es
  negativo. **BREAKING** para cualquier `game_config.yaml` sin esa sección (el del repo se actualiza; el instalado en
  `/etc/nfs` se pisa con `installer.sh`).
- Modelo de efectos en el servidor (`Player` / `Car`):
  - **Nitro**: velocidad máxima y aceleración × `nitro_multiplier`, sobre los valores con upgrades; al expirar vuelven
    exactamente a los previos.
  - **Reparación**: + `repair_amount` de salud, sin superar la salud máxima (incluye upgrades de salud).
  - **Escudo**: ignora el daño por choques durante `shield_duration`, con un estado propio, independiente del cheat de
    vida infinita.
  - Un efecto activo por tipo: volver a tomarlo renueva la duración, no acumula. Efectos de distinto tipo conviven.
  - Un auto muerto no recibe efectos y, al morir, pierde los que tenía. Al empezar una carrera no hay efectos.
- API explícita para avanzar el tiempo de los efectos (sin reloj de pared) y helpers de test con los valores del
  Background del hito.

### Fuera de alcance

- Power-ups en el mundo físico: sensores Box2D, spawn en puntos medios entre checkpoints, reaparición con
  `respawn_cooldown`, sorteo del tipo y RNG inyectable (**Hito 2**). `respawn_cooldown` se carga y valida acá, pero no
  se usa todavía.
- Integración con el game loop, DTOs, snapshots y protocolo (**Hito 3**).
- Render, HUD y sonido en el cliente (**Hito 4**).
- Cualquier cambio en `editor/`.
- Un test que lea los defaults reales de `power_ups` del YAML del repo: no se incluye (la referencia del Background
  de `roadmap.md` al "test 1.2 como excepción" quedó desactualizada; el 1.2 válido es el de la tabla).

## Capabilities

### New Capabilities
- `power-ups`: power-ups en pista. Este change cubre su configuración y los efectos (Nitro, Reparación, Escudo) que
  el servidor aplica a un jugador. Los Hitos 2–4 suman requirements a la misma capability (mundo físico, protocolo,
  cliente).

### Modified Capabilities
<!-- Ninguna: openspec/specs/ todavía no tiene specs. -->

## Impact

- **Servidor**: `server/server_config.h` (nuevo `PowerUpConfig` dentro de `GameConfig`),
  `server/server_config_loader.*` (carga y validación), `server/config/game_config.yaml`, `server/player.*`
  (efectos, reparación, escudo, limpieza), `server/car.*` (multiplicador de nitro sobre velocidad y aceleración).
- **Común**: nuevo `common/power_up_type.h` (enum del tipo, sin serialización).
- **Tests**: `tests/test_player.cpp`, `tests/test_server_config_loader.cpp` (incluye agregar `power_ups` al
  `MINIMAL_GAME` existente), `tests/server_test_utils.*` (helpers con los valores del Background).
- Sin dependencias nuevas. Sin cambios en protocolo, cliente ni `editor/`.
