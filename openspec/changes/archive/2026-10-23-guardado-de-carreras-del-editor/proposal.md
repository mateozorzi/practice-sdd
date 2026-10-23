# Proposal

## Why

Al crear una carrera en el editor para probar los power-ups `UNDER_BRIDGE`, "Done" abrió un diálogo para elegir
dónde guardar el `.csv`. La carrera quedó en `server/races/` del repositorio, no en `/var/nfs/races/`, que es la
carpeta que lee el servidor instalado, así que no apareció en el juego. Además, con una carrera más en
`server/races/` fallan tres tests que esperan exactamente 6 carreras. Como `installer.sh` corre los tests antes de
instalar (`set -e`), la reinstalación se cortó y la carrera tampoco llegó a `/var/nfs/races/`.

El diálogo viene del commit `d6cf4c5` ("Can save the race.csv wherever you want"), pero el resto del editor sigue
suponiendo `/var/nfs/races/`: la lista de carreras para editar, el borrado y el borrado del archivo viejo al
renombrar.

Al abrir una carrera ya creada para editarla, el creador muestra el mapa desde la esquina de arriba a la izquierda.
Si la carrera está en otra parte del mapa (Liberty City mide 4640×4672), cuesta encontrar dónde empieza.

**Hito:** este change no implementa un hito del roadmap de power-ups (`roadmap.md`). Es un arreglo del editor y de
los tests que surgió al probar el Hito 6, y queda fuera de esa feature (ver Impact).

## What Changes

- **Guardado sin diálogo**: "Done" guarda la carrera directamente en la carpeta de carreras del editor,
  `/var/nfs/races/<nombre del circuito>.csv`, la misma que lee el servidor instalado. Se quita el
  `QFileDialog`.
- **Una sola carpeta de carreras en el editor**: crear, editar, renombrar y borrar usan la misma carpeta. Al
  renombrar se borra el archivo viejo de esa carpeta, y "Delete" en la lista borra de la carpeta que se está
  listando (hoy está fija en `/var/nfs/races/` aunque la lista venga de otra carpeta).
- **Sin sobrescribir sin preguntar**: si ya existe una carrera con ese nombre, el editor sigue preguntando. Si no
  se confirma, guarda como `<nombre>_1.csv` (o el primer `_N` libre), como hoy.
- **Abrir centrada en el checkpoint 1**: al abrir una carrera desde la lista de carreras, el mapa se muestra
  centrado en su checkpoint 1. Una carrera nueva se sigue abriendo arriba a la izquierda.
- **Tests de carreras independientes del repositorio**: los tests que cuentan carreras dejan de leer
  `server/races/` y usan la copia fija de `tests/data/races/` (la del Hito 5). Agregar o editar una carrera en
  `server/races/` ya no rompe el build ni el instalador.
- **Las carreras del repositorio se siguen validando**: un test nuevo carga todas las carreras de `server/races/`,
  sin fijar cuántas son, y exige que cada una sea válida. Así una carrera rota que se versione sí falla.
- **Guardado testeable**: la carpeta de carreras se puede inyectar en el editor, así los tests guardan en una
  carpeta temporal y nunca en `/var/nfs/races/`.

### Fuera de alcance

- Cambiar dónde lee el servidor (`races_folder` en `world_config.yaml`) o el formato de los `.csv`.
- Cambiar `installer.sh`: sigue corriendo los tests antes de instalar y cortando si fallan.
- Copiar automáticamente las carreras del editor a `server/races/` (el repositorio) o al revés: versionar una
  carrera sigue siendo copiarla a mano.
- La carrera `server/races/under_bridges.csv` que hoy está sin versionar: este change no la agrega ni la borra. Con
  este change se puede versionar sin romper los tests, o copiar a `/var/nfs/races/`.
- Validar el nombre del circuito más allá del largo mínimo actual (por ejemplo, rechazar `/` en el nombre).
- Cambios en la lógica de power-ups, en el cliente o en el protocolo.

## Capabilities

### New Capabilities
- `race-editor`: dónde y cómo guarda, renombra y borra carreras el editor de circuitos.
- `race-catalogue`: qué carreras ofrece el servidor (todas las válidas de su carpeta de carreras).

### Modified Capabilities
<!-- Ninguna: power-ups no cambia. -->

## Impact

- **Editor** (`editor/EditorMapas/`): `creador.*` (sin diálogo, carpeta de carreras, centra la vista en el
  checkpoint 1 al abrir una carrera), `coordinates.*`
  (`export_checkpoints` recibe la carpeta y el nombre), `selectcircuit.*` (le pasa su carpeta al creador y la usa
  para borrar), `editor.cpp` y `editormenu.cpp` (pasan `/var/nfs/races`).
- **Tests**: `tests/server_test_utils.cpp` (`make_test_server_config()` apunta `races_folder` a
  `tests/data/races/`), `tests/test_race_loader.cpp`, `tests/test_lobby.cpp`;
  `tests/test_server_integration.cpp` no cambia (`ListsTheAvailableRaces` pasa a leer la copia fija vía la config);
  `tests/client/test_editor_windows.cpp` y `tests/client/test_editor_model.cpp` (guardado en carpeta temporal).
- **Feature de power-ups**: su Definition of Done pide "sin cambios en `editor/`" y la constitution dice "sin cambios
  en `editor/` para la feature de power-ups". Este change toca `editor/` a propósito y no forma parte de esa feature.
  Al cerrar el DoD hay que mirar los changes de la feature (Hitos 1 – 6), no todo el historial de `editor/`.
- Sin dependencias nuevas. Sin cambios en el servidor salvo los tests.
