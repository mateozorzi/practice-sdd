# Misión

## Problema
Need4Speed es un juego 2D de carreras multijugador (TP de Taller de Programación) que ya está terminado y
funcionando: cliente, servidor y editor de circuitos. Sumarle features en un código brownfield sin romper lo
que ya anda requiere un proceso que deje explícito qué se construye, cómo se prueba y cuándo está terminado.

## Misión
El proyecto tiene dos objetivos con el mismo peso:

1. **Producto:** hacer evolucionar Need4Speed con features jugables. La feature actual son los power-ups en pista
   (Nitro, Reparación, Escudo).
2. **Proceso:** practicar y demostrar Spec-Driven Development con OpenSpec + Claude Code en un proyecto brownfield.
   Cada feature se trabaja como specs → tests de aceptación → implementación, en hitos con gates.

## Audiencia

| Audiencia | Necesidades principales |
|-----------|-------------------------|
| **Docentes / cátedra** | Código claro y consistente, tests automatizados que respalden cada requirement, documentación al día (README, manuales) y una instalación simple (`installer.sh`). |
| **Jugadores** | Carreras multijugador fluidas y divertidas, controles simples y features que se sientan parte del juego (por ejemplo, power-ups visibles y con feedback de sonido). |

## Principios no negociables
Estos principios aplican a todos los artefactos de OpenSpec (proposal, specs, design, tasks) y a la implementación.
Si un artefacto o un cambio contradice alguno, se trata como un problema bloqueante.

### I. TDD obligatorio
- Primero se escriben los tests de aceptación del hito (GoogleTest) y se verifica que **fallen**; después se
  implementa hasta que pasen.
- Cada requirement tiene al menos un scenario Given/When/Then, y cada scenario se mapea a un test con nombre
  (`Suite.Test`).
- No se considera hecha una tarea sin un test que la respalde.

### II. Tests deterministas
- El tiempo avanza solo con llamadas explícitas del test (pasos de física o ticks del loop), nunca con el reloj de pared.
- La aleatoriedad se inyecta: los tests usan un RNG falso con una secuencia fija. La excepción es el test que verifica
  la distribución.
- Los valores que usa cada test salen del **Background** del hito y el test los fija explícitamente. Los tests nunca
  dependen de los defaults de `server/config/*.yaml`; la excepción documentada es el test que detecta un cambio
  accidental de esos defaults.

### III. Gates entre hitos
No se arranca un hito hasta que el anterior cumpla todo esto:
- (a) todos los tests del hito existen y pasan;
- (b) `make run-tests` en verde, sin regresiones;
- (c) los hooks de lint/formato/cppcheck sin errores;
- (d) `/opsx:verify` sin issues CRITICAL.

## Fuera de alcance (feature power-ups)
- Cambios en el editor de mapas (`editor/`): los power-ups no se colocan desde el editor.
- Lógica de efectos en el cliente: el cliente solo dibuja y reproduce sonido.
