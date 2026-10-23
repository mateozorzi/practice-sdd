# Spec Delta

## Purpose

Qué carreras ofrece el servidor: todas las válidas de su carpeta de carreras. Los tests del catálogo usan una copia
fija de las carreras, así que agregar o editar carreras del juego no los rompe.

## ADDED Requirements

### Requirement: El servidor ofrece las carreras de su carpeta
El servidor SHALL ofrecer, en el lobby y por el protocolo, una carrera por cada archivo `.csv` de su carpeta de
carreras, con el nombre del archivo sin la extensión.

#### Scenario: El cargador lee las carreras fijas de test
- **GIVEN** la copia fija de `tests/data/races/` (6 carreras: `bridge_tour`, `dock_adventure`,
  `highways_are_not_playgrounds`, `ocean_bridge_cruise`, `ocean_dream`, `park_day`)
- **WHEN** el servidor carga sus carreras
- **THEN** hay exactamente esas 6, cada una con vueltas, sus checkpoints y la cantidad de checkpoints que declara
- Test: `RaceLoaderTest.LoadsTheFixedTestRaces` (`tests/test_race_loader.cpp`)

#### Scenario: El lobby lista las carreras fijas de test
- **GIVEN** un lobby con la config de test (carreras de `tests/data/races/`)
- **WHEN** se pide la lista de carreras
- **THEN** el catálogo tiene 6 carreras
- Test: `LobbyTest.ListsTheFixedTestRaces` (`tests/test_lobby.cpp`)

#### Scenario: Un cliente recibe las carreras fijas de test
- **GIVEN** un servidor real con la config de test y un cliente conectado
- **WHEN** el cliente pide el catálogo de carreras
- **THEN** recibe 6 carreras, cada una con checkpoints
- Test: `ServerIntegrationTest.ListsTheAvailableRaces` (`tests/test_server_integration.cpp`)

### Requirement: Los tests no dependen de las carreras del juego
La configuración de test MUST leer las carreras de la copia fija de `tests/data/races/` y nunca de `server/races/`,
así agregar, editar o borrar una carrera del juego no cambia el resultado de los tests del catálogo.

#### Scenario: La config de test apunta a la copia fija
- **GIVEN** la configuración de test (`make_test_server_config()`)
- **WHEN** se mira su carpeta de carreras
- **THEN** es `tests/data/races/` del repositorio
- Test: `ServerTestUtilsTest.RacesComeFromTheFixedCopy` (`tests/test_race_loader.cpp`)

### Requirement: Las carreras del juego son válidas
Toda carrera de `server/races/` (las que instala `installer.sh`) MUST cargarse sin errores: ciudad conocida, al
menos una vuelta y al menos dos checkpoints, con tantos checkpoints como declara. El test no fija cuántas carreras
hay.

#### Scenario: Se cargan todas las carreras del repositorio
- **GIVEN** la carpeta `server/races/` del repositorio con N archivos `.csv` (N no está fijo en el test)
- **WHEN** el servidor carga sus carreras desde esa carpeta
- **THEN** carga N carreras sin lanzar, y cada una tiene vueltas > 0, al menos 2 checkpoints y tantos como declara;
  una falla nombra la carrera
- Test: `RaceLoaderTest.EveryGameRaceIsValid` (`tests/test_race_loader.cpp`)
