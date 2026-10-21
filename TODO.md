# TODO: Power-ups en pista (Need4Speed)

Feature nueva desarrollada con Spec-Driven Development (OpenSpec) sobre un proyecto brownfield.

**Feature:** durante las carreras aparecen power-ups en la pista; el jugador que pasa por uno
recibe un efecto de duración limitada.

**Por qué esta feature:** atraviesa todas las capas (config del servidor, física Box2D,
game loop, protocolo, render y sonido del cliente), así que ejercita el flujo SDD de punta a punta.

## Flujo de trabajo (Claude Code + OpenSpec)

0. **Setup (una sola vez):** `openspec init` y completar `openspec/config.yaml`:
   - `context`: misión del proyecto, tech-stack, reglas de `CLAUDE.md`
     (no mezclar headers de cliente y servidor, rangos de puertos por archivo de test).
   - `rules`: "cada requirement tiene al menos un scenario Given/When/Then que se mapea
     a un test de gtest nombrado".
   - Este TODO funciona como **roadmap**: cada hito de abajo = un *change* de OpenSpec.
1. **Explorar:** `/opsx:explore` para que el modelo lea el código del hito antes de proponer.
2. **Proponer:** `/opsx:propose <hito>` genera `proposal.md`, `design.md`, `tasks.md` y
   los deltas de specs. **Revisión obligatoria** antes de seguir: que los scenarios
   cubran todos los criterios de aceptación del hito.
3. **Aplicar:** `/opsx:apply`.
   - Ciclo TDD: primero se escriben los tests de aceptación del hito y se verifica que **fallen**;
     después se implementa hasta que pasen.
   - Hooks ya estan configurados en `.claude/settings.json`: al editar → formato + cppcheck;
     al commitear → linters + tests. Si un hook falla, se arregla.
4. **Verificar:** `/opsx:verify` → 0 issues CRITICAL. Revisar warnings de cobertura de scenarios.
5. **Archivar / actualizar doc:** `/opsx:archive` mergea los cambios a `openspec/specs/`.
   Si cambia algo visible para el usuario o el protocolo, actualizar README / manuales.
6. **Siguiente hito:** volver al paso 1. **No se arranca un hito si el anterior no pasó su gate.**

## Alcance (MVP)
Los efectos se calculan **solo en el servidor**; el cliente sólo dibuja y reproduce sonido.

| Tipo       | Efecto                                                                              | Duración              |
|------------|-------------------------------------------------------------------------------------|-----------------------|
| Nitro      | Velocidad máxima y aceleración × `nitro_multiplier` (sobre los valores con upgrades)| `nitro_duration`      |
| Reparación | + `repair_amount` de salud, sin superar la salud máxima                             | Instantáneo           |
| Escudo     | Ignora el daño por choques                                                          | `shield_duration`     |

### Configuración
Todos los valores se leen de `server/config/game_config.yaml`, bajo `power_ups:`.
Los tests **no** dependen de estos valores: cada hito define los suyos en un Background
(ver `roadmap.md`) y el test los fija explícitamente.

```yaml
power_ups:
  nitro_multiplier: 1.25
  nitro_duration: 3        # segundos
  repair_amount: 50
  shield_duration: 5       # segundos
  respawn_cooldown: 20     # segundos
```

### Reglas
- [ ] **Ubicación:** los spawn points se calculan en el punto medio entre checkpoints consecutivos,
      no se cargan desde el mapa. Solo hay uno cada dos checkpoints (tramos 1→2, 3→4, …).
- [ ] **Tipo aleatorio:** el tipo de cada power-up se sortea de forma uniforme entre Nitro, Reparación
      y Escudo, cada vez que aparece (al inicio de la carrera y en cada reaparición).
      El generador aleatorio es inyectable para que los tests sean deterministas.
- [ ] **Detección:** cada power-up es un sensor Box2D (no frena ni daña); lo recoge el primer auto que lo toca.
      Si dos autos lo tocan en el mismo paso de física, lo recoge solo uno.
- [ ] **Reaparición:** al recogerse desaparece para todos y reaparece en el mismo lugar después de
      `respawn_cooldown` segundos.
- [ ] **Acumulación:** un solo efecto activo por tipo; volver a agarrarlo renueva la duración (no se suma
      ni se multiplica de nuevo). Efectos de distinto tipo sí conviven.
- [ ] **Fin del efecto:** al expirar el nitro, velocidad y aceleración vuelven exactamente a sus valores previos.
- [ ] **Cheat de vida infinita:** sigue activo después de usar escudo o reparación
      (el escudo usa su propio estado, no el flag `infinite_health`).
- [ ] **Autos muertos:** no recogen power-ups y pierden los efectos activos.
- [ ] **Limpieza:** al terminar una carrera se borran power-ups y efectos activos; la siguiente arranca limpia.
- [ ] **Sonido:** al recoger un power-up, el cliente del jugador que lo recogió reproduce el mismo sonido
      que las mejoras (`sfx_upgrade.wav`, vía `Sounds::play_upgrade_sound`).
- [ ] **Indicador en el HUD:** mientras el jugador tenga un efecto con duración activo (Nitro o Escudo),
      su pantalla muestra un cartel por efecto con el sprite del power-up (`nitro.png`, `shield.png`)
      y los segundos restantes (redondeados hacia arriba, ej. `3s`). El tiempo restante lo manda el
      servidor en el snapshot; el cliente no lleva su propio timer. Al expirar, el cartel desaparece;
      al renovar el efecto, el contador vuelve a la duración completa. Con dos efectos activos se
      muestran los dos carteles, uno al lado del otro. La Reparación es instantánea, así que no muestra cartel.
      Solo se muestran los efectos del propio auto.
- [ ] **Editor:** no se pueden colocar power-ups desde el editor de mapas (sin cambios en `editor/`).

@roadmap.md -> test de aceptacion