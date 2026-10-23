# Spec Delta

## Purpose

Cómo el editor de circuitos guarda, renombra y borra carreras: siempre en su carpeta de carreras, la misma que lee el
servidor instalado, sin preguntar dónde guardar.

## ADDED Requirements

### Requirement: Done guarda la carrera en la carpeta de carreras
Al confirmar una carrera válida con "Done", el editor SHALL guardarla en su carpeta de carreras como
`<nombre del circuito>.csv`, sin abrir un diálogo para elegir dónde guardarla. En el programa instalado esa carpeta
MUST ser `/var/nfs/races/`, la que lee el servidor.

#### Scenario: Se guarda sin preguntar dónde
- **GIVEN** el creador de Liberty City con una carpeta de carreras temporal vacía, 5 checkpoints sobre la calle,
  2 vueltas y el nombre `my circuit`
- **WHEN** se hace clic en "Done"
- **THEN** existe `<carpeta>/my circuit.csv` con el mapa y la máscara de Liberty City en la primera línea,
  `Liberty City,2` como ciudad y vueltas y los 5 checkpoints en orden; el único mensaje es
  `Congratulations! Creation complete` y el creador se cierra
- Test: `CreadorTest.DoneSavesTheRaceInTheRacesFolder` (`tests/client/test_editor_windows.cpp`)

#### Scenario: Una carrera inválida no se guarda
- **GIVEN** el creador con una carpeta de carreras temporal vacía
- **WHEN** se hace clic en "Done" sin checkpoints, después con 0 vueltas y después con un nombre corto (`abc`)
- **THEN** se muestran los tres errores de validación de hoy, el creador sigue abierto y la carpeta sigue vacía
- Test: `CreadorTest.DoneValidatesTheRace` (`tests/client/test_editor_windows.cpp`)

#### Scenario: El editor instalado usa la carpeta del servidor
- **GIVEN** el editor instalado
- **WHEN** se abre el creador de una ciudad o la lista de carreras para editar
- **THEN** los dos usan la carpeta de carreras `/var/nfs/races`
- Test: `EditorWindowsTest.InstalledEditorUsesTheServerRacesFolder` (`tests/client/test_editor_windows.cpp`)

### Requirement: Una carrera existente no se pisa sin confirmar
Si en la carpeta de carreras ya hay una carrera con el mismo nombre, el editor MUST preguntar antes de
sobrescribirla. Si no se confirma, SHALL guardar la nueva como `<nombre>_N.csv`, con el primer `N` (desde 1) libre,
y dejar la existente sin cambios.

#### Scenario: No se confirma sobrescribir
- **GIVEN** el creador con una carpeta de carreras temporal que ya tiene `my circuit.csv`, y una carrera válida
  con el nombre `my circuit`
- **WHEN** se hace clic en "Done" y se responde "No" a `Already exists a race with this name...`
- **THEN** existe `my circuit_1.csv` con la carrera nueva y `my circuit.csv` no cambió
- Test: `CreadorTest.ExistingRaceIsNotOverwritten` (`tests/client/test_editor_windows.cpp`)

### Requirement: Editar, renombrar y borrar usan la carpeta de la lista
Una carrera abierta desde la lista de carreras SHALL guardarse en la carpeta de esa lista. Si se le cambió el
nombre, el editor MUST borrar de esa carpeta el archivo con el nombre viejo. "Delete" en la lista MUST borrar el
archivo de la carpeta que se está listando.

#### Scenario: Renombrar reemplaza el archivo
- **GIVEN** la lista de carreras de una carpeta temporal con `first.csv` (5 checkpoints)
- **WHEN** se abre `first`, se cambia el nombre a `second race` y se hace clic en "Done"
- **THEN** la carpeta tiene `second race.csv` y ya no tiene `first.csv`, y la lista vuelve a mostrarse con una
  sola carrera, `second race`
- Test: `SelectCircuitTest.RenamedRaceReplacesItsFile` (`tests/client/test_editor_windows.cpp`)

#### Scenario: Borrar desde la lista
- **GIVEN** la lista de carreras de una carpeta temporal con `first.csv`
- **WHEN** se hace clic en "Delete" en la fila de `first`
- **THEN** la carpeta ya no tiene `first.csv` y la lista queda vacía
- Test: `SelectCircuitTest.DeleteRemovesTheRaceFromItsFolder` (`tests/client/test_editor_windows.cpp`)

### Requirement: Una carrera abierta para editar se muestra desde su checkpoint 1
Al abrir una carrera existente en el creador, el editor SHALL mostrar el mapa centrado en el checkpoint 1 (salvo
que el borde del mapa no lo permita).

#### Scenario: El mapa se abre centrado en el checkpoint 1
- **GIVEN** una carrera de Liberty City cuyo checkpoint 1 está en (2300, 2300), lejos de los bordes del mapa
- **WHEN** se abre en el creador maximizado
- **THEN** el centro de la parte visible del mapa está a lo sumo a 2 px del centro del checkpoint 1
- Test: `EditorWindowsTest.OpenedRaceIsCenteredOnTheFirstCheckpoint` (`tests/client/test_editor_windows.cpp`)
