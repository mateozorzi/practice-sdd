# Tasks

## 1. Tests de aceptación en rojo

- [x] 1.1 En `tests/test_race_loader.cpp`: renombrar `LoadsEveryRaceInTheRepository` → `LoadsTheFixedTestRaces`,
      agregar `ServerTestUtilsTest.RacesComeFromTheFixedCopy` y `RaceLoaderTest.EveryGameRaceIsValid` (lee
      `server/races/`, cuenta los `.csv` sin fijar cuántos son, nombra la carrera que falla); en `tests/test_lobby.cpp`
      renombrar `ListsTheRacesInTheRepository` → `ListsTheFixedTestRaces` (design D4)
- [x] 1.2 En `tests/client/test_editor_windows.cpp`: `QTemporaryDir` en `CreadorTest`; sacar el clic válido de
      `DoneValidatesTheRace` y chequear que la carpeta queda vacía; agregar
      `CreadorTest.DoneSavesTheRaceInTheRacesFolder`, `CreadorTest.ExistingRaceIsNotOverwritten`,
      `SelectCircuitTest.RenamedRaceReplacesItsFile`, `SelectCircuitTest.DeleteRemovesTheRaceFromItsFolder` y
      `EditorWindowsTest.InstalledEditorUsesTheServerRacesFolder` (design D3); actualizar los comentarios que dicen
      que el guardado no se prueba (también en `tests/client/test_editor_model.cpp`)
- [x] 1.3 Agregar solo lo mínimo para compilar: `RACES_FOLDER`, los constructores de `Creador` con
      `races_folder` (que la guardan y la exponen con `races_folder()`, sin usarla todavía), `SelectCircuit::races_folder()`,
      y actualizar las llamadas en `Editor`, `EditorMenu` y `SelectCircuit`; compilar con
      `cmake --build build --target nfs_tests nfs_client_tests -j16` y verificar con
      `./build/nfs_tests --gtest_filter='RaceLoaderTest.*:ServerTestUtilsTest.*:LobbyTest.*:ServerIntegrationTest.ListsTheAvailableRaces'`
      y `./build/nfs_client_tests --gtest_filter='CreadorTest.*:SelectCircuitTest.*:EditorWindowsTest.*'` que fallan
      `RacesComeFromTheFixedCopy`, los dos tests renombrados y `ListsTheAvailableRaces` (mientras
      `server/races/under_bridges.csv` exista), `DoneSavesTheRaceInTheRacesFolder`, `ExistingRaceIsNotOverwritten`,
      `RenamedRaceReplacesItsFile` y `DeleteRemovesTheRaceFromItsFolder`; anotar los que pasan por coincidir con lo
      actual (`EveryGameRaceIsValid`, `InstalledEditorUsesTheServerRacesFolder`, `DoneValidatesTheRace`) y que el
      resto sigue en verde. Ningún test escribe en `/var/nfs/races/` (`ls -la /var/nfs/races` antes y después)

## 2. Catálogo de carreras en los tests

- [x] 2.1 Fijar `races_folder` en `tests/data/races/` en `make_test_server_config()` (D4); verificar que pasan
      `ServerTestUtilsTest.RacesComeFromTheFixedCopy`, `RaceLoaderTest.*`, `LobbyTest.*` y
      `ServerIntegrationTest.*`, con y sin `server/races/under_bridges.csv` presente

## 3. Guardado del editor

- [x] 3.1 Cambiar `Coordinates::export_checkpoints` a carpeta + nombre y quitar el `QFileDialog` de
      `Creador::on_Done_clicked`, con el borrado del nombre viejo en la carpeta del creador (D2); verificar que pasan
      `CreadorTest.DoneSavesTheRaceInTheRacesFolder`, `CreadorTest.ExistingRaceIsNotOverwritten` y
      `CreadorTest.DoneValidatesTheRace`
- [x] 3.2 Que `SelectCircuit` le pase su carpeta al `Creador` que abre y borre de su carpeta (D1); verificar que pasan
      `SelectCircuitTest.*` y `EditorWindowsTest.*`, y que `/var/nfs/races/` no cambió después de correr
      `nfs_client_tests`

## 4. Gate

- [x] 4.1 Hooks sin errores (formato, cpplint, cppcheck) sobre los archivos tocados y `make run-tests` en verde, sin
      regresiones, con `server/races/under_bridges.csv` presente

## 5. Abrir una carrera centrada en el checkpoint 1

- [x] 5.1 Agregar `EditorWindowsTest.OpenedRaceIsCenteredOnTheFirstCheckpoint` (design D6), compilar con
      `cmake --build build --target nfs_client_tests -j16` y verificar que falla
- [x] 5.2 Implementar D6 (`Creador::showEvent` centra la vista en el checkpoint 1); verificar que pasan
      `EditorWindowsTest.*`, `CreadorTest.*` y `SelectCircuitTest.*`
- [x] 5.3 Hooks sin errores (formato, cpplint, cppcheck) sobre los archivos tocados y `make run-tests` en verde, sin
      regresiones, con `server/races/under_bridges.csv` presente

## 6. Prueba manual

- [x] 6.1 Con el editor compilado (`build/nfs_editor`), crear una carrera, ver que se guarda en
      `/var/nfs/races/<nombre>.csv` sin diálogo, abrirla desde "Edit" y ver que el mapa se muestra centrado en el
      checkpoint 1, reiniciar `nfs_server` y ver que aparece en la lista del cliente

## Workflow follow-up

- `/opsx:verify` y después `/opsx:archive`.
- Al archivar: actualizar la sección "Editor" del README y el manual del editor (dónde se guardan las carreras y que
  para versionar una hay que copiarla a `server/races/`).
- Decidir aparte si `server/races/under_bridges.csv` se versiona.
