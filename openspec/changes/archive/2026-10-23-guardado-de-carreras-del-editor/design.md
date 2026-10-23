# Design

## Context

Ver `proposal.md` (Why) y `specs/race-editor/spec.md`, `specs/race-catalogue/spec.md`. Estado actual relevante:

- `Creador::on_Done_clicked` (`editor/EditorMapas/creador.cpp`) valida la carrera y abre
  `QFileDialog::getSaveFileName` desde el home. `Coordinates::export_checkpoints(file_path, ...)` arma
  `QDir("/var/nfs/races").filePath(file_path + ".csv")`; con la ruta absoluta del diálogo, `filePath` la devuelve tal
  cual y `/var/nfs/races` se ignora. Si el archivo existe pregunta con `QMessageBox::question` y, con "No", busca el
  primer `<nombre>_N.csv` libre.
- Al renombrar una carrera editada, `on_Done_clicked` borra `/var/nfs/races/<nombre viejo>.csv` (ruta fija).
- `SelectCircuit(path)` lista los `.csv` de `path`, pero `delete_race` borra de `/var/nfs/races/` (ruta fija) y
  `select_race` crea `Creador(race)` sin carpeta. `EditorMenu` le pasa `"/var/nfs/races"`; `Editor` crea
  `Creador(mapa, mask)` para cada ciudad.
- Tests del editor (`tests/client/test_editor_windows.cpp`): `ModalCloser` contesta "No" a los `QMessageBox` con
  botón No, acepta los demás (guardando su texto en `messages`) y rechaza cualquier otro `QDialog` (por eso hoy el
  diálogo de guardado se cancela y "Saving a race is not tested"). `CreadorTest` usa `Creador{LIBERTY_MAP,
  LIBERTY_MASK}`; `SelectCircuitTest` ya usa una `QTemporaryDir` y `write_race`.
- Servidor: `RaceLoader::load_races()` lee `config.world.races_folder`. `make_test_server_config()`
  (`tests/server_test_utils.cpp`) redirige `/var/nfs/races/` a `server/races/` del repo; de ahí cuentan 6 carreras
  `RaceLoaderTest.LoadsEveryRaceInTheRepository`, `LobbyTest.ListsTheRacesInTheRepository` (vía
  `make_fast_game_config()`) y `ServerIntegrationTest.ListsTheAvailableRaces`. `tests/data/races/` es la copia fija
  del Hito 5, hoy idéntica a `server/races/` salvo `under_bridges.csv`.

## Goals / Non-Goals

**Goals:**
- Que una carrera hecha con el editor instalado aparezca en el servidor instalado al reiniciarlo, sin pasar por el
  repositorio ni por el instalador.
- Que ningún test escriba en `/var/nfs/races/` y que el guardado quede probado.
- Que el resultado de los tests no dependa de qué carreras hay en `server/races/`.

**Non-Goals:**
- Recargar las carreras en un servidor que ya está corriendo (el lobby las lee al arrancar).
- Unificar la carpeta del editor con la config del servidor (`/etc/nfs/world_config.yaml`): el editor no lee esa
  config y sigue usando su propia constante.

## Decisions

### D1. Carpeta de carreras inyectable en el editor
- Constante `RACES_FOLDER = "/var/nfs/races"` en un header del editor (por ejemplo `editor/EditorMapas/races_folder.h`),
  usada por `Editor` y `EditorMenu` en lugar del literal repetido.
- `Creador(const QString& map, const QString& mask, const QString& races_folder, QWidget* parent = nullptr)` y
  `Creador(const Race& race, const QString& races_folder, QWidget* parent = nullptr)`; guarda la carpeta y la expone
  con `const QString& races_folder() const` (para el test del editor instalado).
- `SelectCircuit` pasa su `path_races` al `Creador` que abre y lo usa en `delete_race`; expone
  `const QString& races_folder() const`.
- Alternativa descartada: un parámetro por defecto `= RACES_FOLDER` en los constructores. Un test que se olvide de
  pasarla escribiría en `/var/nfs/races/`; obligar a pasarla hace imposible ese error.

### D2. Guardado sin diálogo
`Coordinates::export_checkpoints(const QString& races_folder, const QString& race_name, map, n_laps, mask, window)`:
arma `QDir(races_folder).filePath(race_name + ".csv")`, crea la carpeta si falta y conserva la pregunta de
sobrescribir y el sufijo `_N`. `on_Done_clicked` deja de usar `QFileDialog` y le pasa la carpeta y el nombre del
circuito. Al renombrar borra `QDir(races_folder).filePath(<nombre viejo> + ".csv")`. Se quita
`#include <QFileDialog>`. Como el hook de cppcheck corre sobre todo archivo tocado, `Coordinates::get_map_name` pasa a
ser `static` (aviso `functionStatic` que ya existía).

Alternativa descartada: dejar el diálogo pero abriéndolo en `/var/nfs/races`. Sigue permitiendo guardar donde el
servidor no lee, que es justo el problema reportado.

### D3. Tests del editor en carpeta temporal
- `CreadorTest` crea una `QTemporaryDir` y la pasa al `Creador`. `DoneValidatesTheRace` hoy termina con un clic
  válido que se apoyaba en el diálogo cancelado; ahora ese clic guardaría, así que se saca (pasa a
  `DoneSavesTheRaceInTheRacesFolder`) y el test chequea además que la carpeta temporal quedó vacía.
- `DoneSavesTheRaceInTheRacesFolder`: lee el `.csv` escrito y compara la primera línea (mapa y máscara), la ciudad y
  vueltas y las filas de checkpoints contra los puntos clickeados (`street_points(5)`); el único mensaje es el de
  éxito. Que el archivo exista prueba que no hubo diálogo: `ModalCloser` rechazaría un `QFileDialog` y no se
  guardaría nada.
- `ExistingRaceIsNotOverwritten`: escribe antes `my circuit.csv` con un contenido conocido; `ModalCloser` contesta
  "No"; se espera `my circuit_1.csv` y el original intacto.
- `SelectCircuitTest.RenamedRaceReplacesItsFile` y `DeleteRemovesTheRaceFromItsFolder` usan `write_race` (con 5
  filas para el primero, que debe pasar la validación) y los botones de la tabla (columna 4 "Open", 5 "Delete").
- `EditorWindowsTest.InstalledEditorUsesTheServerRacesFolder`: abre el creador desde `Editor` y la lista desde
  `EditorMenu` (como los tests de navegación que ya existen) y compara `races_folder()` con `"/var/nfs/races"`, sin
  clic en "Done".
- Se actualizan los comentarios de `test_editor_windows.cpp` y `test_editor_model.cpp` que dicen que el guardado no
  se prueba.

### D4. Tests del catálogo con la copia fija
- `make_test_server_config()` fija `config.world.races_folder = source_path("tests/data/races/")` en lugar de
  redirigirla a `server/races/`. Las rutas de `server/world/` se siguen redirigiendo al repo. Como la copia fija es
  idéntica a las 6 carreras de hoy, ningún test que use otra carrera (por ejemplo `park_day`) cambia.
- `RaceLoaderTest.LoadsEveryRaceInTheRepository` → `LoadsTheFixedTestRaces` y
  `LobbyTest.ListsTheRacesInTheRepository` → `ListsTheFixedTestRaces` (mismo contenido, nombre fiel a lo que miran);
  `ServerIntegrationTest.ListsTheAvailableRaces` no cambia.
- `ServerTestUtilsTest.RacesComeFromTheFixedCopy` chequea la ruta de la config de test.
- `RaceLoaderTest.EveryGameRaceIsValid`: `config.world.races_folder = source_path("server/races/")`, cuenta los
  `.csv` de la carpeta con `std::filesystem::directory_iterator` y exige que `load_races()` no lance, devuelva esa
  cantidad y que cada carrera sea válida (mismas condiciones que hoy chequea `LoadsEveryRaceInTheRepository`).
- 5.11 y 6.15 ya fijan `tests/data/races/` explícitamente; no cambian.

### D5. Tiempo, RNG y config de test
Este change no avanza tiempo ni sortea nada: no hay pasos de física, ticks ni RNG en sus tests. La única config de
test que cambia es la carpeta de carreras de `make_test_server_config()` (D4); el editor no usa la config del
servidor y recibe su carpeta por constructor (D1).

### D6. Abrir una carrera centrada en el checkpoint 1
`Creador` sobrescribe `showEvent`: la primera vez que se muestra, busca el checkpoint con id 1 (por id, no el primero
del vector) y llama a `ui->labelmap->centerOn(...)` con el centro de su rectángulo. Sin checkpoints (carrera nueva)
no hace nada, así que el creador nuevo sigue arrancando arriba a la izquierda.

Test `EditorWindowsTest.OpenedRaceIsCenteredOnTheFirstCheckpoint`: crea `Creador(race, <carpeta temporal>)` con el
checkpoint 1 en (2300, 2300), lo muestra maximizado (offscreen), procesa eventos y compara
`view->mapToScene(view->viewport()->rect().center())` con el centro del checkpoint (tolerancia de 2 px).

Alternativa descartada: centrar en el constructor. Ahí la vista todavía no tiene su tamaño final, así que
`centerOn` calcula mal el desplazamiento.

## Risks / Trade-offs

- [Algún test usa una carrera que solo está en `server/races/`] → hoy las dos carpetas tienen las mismas 6
  carreras; `EveryGameRaceIsValid` sigue cubriendo las del juego. Si en el futuro se agrega una carrera al juego y
  se la quiere en los tests, se copia a `tests/data/races/` a propósito.
- [`server/races/under_bridges.csv` sin versionar hace fallar `EveryGameRaceIsValid` si es inválida] → es válida
  (Liberty City, 1 vuelta, 5 checkpoints); igual queda fuera de alcance decidir si se versiona.
- [Se pierde poder guardar una carrera en otra carpeta] → aceptado: el servidor solo lee `/var/nfs/races/`. Quien
  quiera versionarla la copia a `server/races/`.
- [Revierte una decisión de otro integrante (`d6cf4c5`)] → se documenta en la proposal y en el commit.
- [Un nombre de circuito con `/` escribe fuera de la carpeta] → fuera de alcance (proposal); se anota para un change
  aparte.

## Migration Plan

Sin migración de datos: las carreras ya guardadas en `/var/nfs/races/` siguen ahí. Rollback: revertir el change.
