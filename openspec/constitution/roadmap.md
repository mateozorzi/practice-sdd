# Roadmap

## Feature actual: power-ups en pista
Durante las carreras aparecen power-ups en la pista (Nitro, Reparación, Escudo). El jugador que pasa por uno recibe
un efecto. La feature atraviesa todas las capas (config, física, game loop, protocolo, render y sonido), así que
ejercita el flujo SDD de punta a punta.

**Fuentes de verdad** (este archivo no las duplica):
- [`TODO.md`](../../TODO.md): alcance, configuración y reglas de la feature.
- [`roadmap.md`](../../roadmap.md): Background y criterios de aceptación Given/When/Then de cada hito.

## Hitos
Cada hito es un change de OpenSpec (`/opsx:explore` → `/opsx:propose` → `/opsx:apply` → `/opsx:verify` →
`/opsx:archive`).

| # | Hito | Capas | Criterios | Estado |
|---|------|-------|-----------|--------|
| 1 | Configuración y modelo de efectos en el servidor (sin física ni red) | `server/player.*`, `server/car.*`, config loader | 1.1 – 1.17 | Pendiente |
| 2 | Power-ups en el mundo físico (sensores Box2D, spawn, reaparición) | `server/power_up.*`, `race_loader`, `collision_listener` | 2.1 – 2.13 | Pendiente |
| 3 | Game loop y protocolo (DTOs, snapshots) | `server/game_loop.*`, protocolos, `common/` | 3.1 – 3.8 | Pendiente |
| 4 | Render y sonido en el cliente (sprites, HUD, sonido) | `client/sdl_manager.*`, `client/game.*` | 4.1 – 4.12 | Pendiente |

## Gate para avanzar de hito
Ver el principio III de [`mission.md`](mission.md):
- (a) todos los tests del hito existen y pasan;
- (b) `make run-tests` en verde, sin regresiones;
- (c) los hooks de lint/cppcheck sin errores;
- (d) `/opsx:verify` sin CRITICAL.

## Definition of Done de la feature
- [ ] Todos los tests de los 4 hitos pasan; ningún test existente se rompe.
- [ ] Ningún test de power-ups depende de los valores de `server/config/*.yaml`.
- [ ] La cobertura (`make test`) de los archivos nuevos o modificados del servidor es ≥ la actual.
- [ ] Sin cambios en `editor/`.
- [ ] `openspec/specs/` contiene la spec de power-ups archivada.
- [ ] Prueba manual.

## Aclaraciones
- **Prueba manual del DoD:**: Usuario prueba una carrera a medida que se completan hitos, que tengan algo que probar.