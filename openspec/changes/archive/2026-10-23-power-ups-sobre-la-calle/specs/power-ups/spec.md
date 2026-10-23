# Spec Delta

## MODIFIED Requirements

### Requirement: El power-up aparece en el punto medio del tramo
Cada power-up SHALL ubicarse en el punto medio del camino por la calle entre los dos checkpoints de su tramo: el
punto del camino que deja la mitad de su largo de cada lado, en las coordenadas en píxeles de la carrera. Si la
carrera no tiene grilla de calles, el power-up MUST ubicarse en el punto medio entre los centros de los dos
checkpoints, (P + Q) / 2.

#### Scenario: Posición del primer power-up (2.3)
- **GIVEN** un circuito de test de 4 checkpoints con los checkpoints 1 y 2 en P y Q (leídos de la carrera), sin
  grilla de calles (criterio 5.3: el test del Hito 2 no cambia)
- **WHEN** se cargan los power-ups de la carrera
- **THEN** el primer power-up está en (P + Q) / 2
- Test: `PowerUpManagerTest.PowerUpSpawnsAtMidpointOfCheckpoints` (`tests/test_power_up.cpp`)

#### Scenario: Calle recta (5.4)
- **GIVEN** la grilla de test del Background con celdas de 10 px y solo la calle recta, horizontal de (0, 0) a
  (200, 0), y un circuito con los checkpoints 1 y 2 en P = (0, 0) y Q = (200, 0)
- **WHEN** se cargan los power-ups con esa grilla
- **THEN** el primer power-up está en (P + Q) / 2 = (100, 0)
- Test: `StreetPowerUpTest.StraightStreetKeepsMidpoint` (`tests/test_power_up.cpp`)

#### Scenario: Calle en L (5.5)
- **GIVEN** la grilla de test con la calle en L, de (0, 0) a (200, 0) y de (200, 0) a (200, 200), y los checkpoints
  1 y 2 en P = (0, 0) y Q = (200, 200), donde (P + Q) / 2 = (100, 100) queda fuera de la calle
- **WHEN** se cargan los power-ups con esa grilla
- **THEN** el primer power-up está en (200, 0): el camino mide 400 px y ese punto deja 200 px de cada lado, sobre
  una celda de calle
- Test: `StreetPowerUpTest.LShapedStreetUsesPathMidpoint` (`tests/test_power_up.cpp`)

## ADDED Requirements

### Requirement: Grilla de calles de la ciudad
El servidor SHALL leer la grilla de calles de cada ciudad de un archivo JSON con el tamaño de celda en píxeles, el
ancho y el alto en celdas y, por cada celda, si es transitable. La ruta del archivo de cada ciudad se configura en
`world_config.yaml`.

#### Scenario: Se carga una grilla (5.1)
- **GIVEN** un JSON temporal escrito por el test con celdas de 10 px, 3 celdas de ancho, 2 de alto y las filas
  `101` y `011` (1 = calle)
- **WHEN** se carga la grilla
- **THEN** la grilla tiene celdas de 10 px, 3 × 2 celdas, y son calle exactamente las celdas (0, 0), (2, 0), (1, 1)
  y (2, 1)
- Test: `StreetsGridTest.LoadsCellSizeAndStreetCells` (`tests/test_streets_grid.cpp`)

### Requirement: Grilla de calles inválida
La carga de una grilla MUST fallar con un error que nombra el archivo y el problema si el JSON está mal formado, le
falta un campo, el tamaño de celda no es positivo o las dimensiones no coinciden con los datos de las celdas.

#### Scenario: JSON mal formado (5.2)
- **GIVEN** un archivo temporal de grilla cuyo contenido no es JSON válido
- **WHEN** se carga la grilla
- **THEN** la carga falla con un error que menciona la ruta del archivo
- Test: `StreetsGridTest.MalformedJsonThrows` (`tests/test_streets_grid.cpp`)

#### Scenario: El alto no coincide con las filas (5.2)
- **GIVEN** un JSON temporal de grilla que declara 3 filas de alto y trae 2
- **WHEN** se carga la grilla
- **THEN** la carga falla con un error que menciona `height`
- Test: `StreetsGridTest.HeightMismatchThrows` (`tests/test_streets_grid.cpp`)

#### Scenario: El ancho no coincide con una fila (5.2)
- **GIVEN** un JSON temporal de grilla que declara 3 celdas de ancho y trae una fila de 2
- **WHEN** se carga la grilla
- **THEN** la carga falla con un error que menciona `width`
- Test: `StreetsGridTest.WidthMismatchThrows` (`tests/test_streets_grid.cpp`)

### Requirement: La grilla sale de la máscara de calles
Una celda de la grilla generada SHALL ser calle si y solo si el píxel de la máscara `B_STREETS` de la ciudad en el
centro de la celda es del color de calle de la máscara (RGB 255, 36, 250, con tolerancia 30 por canal), el mismo
criterio que usa el editor.

#### Scenario: Se clasifican los píxeles de una máscara (soporte de 5.11)
- **GIVEN** una imagen de máscara de 30 × 20 px armada en el test, con color de calle en los centros de las celdas
  (0, 0) y (2, 1) de 10 px y otros colores en el resto
- **WHEN** se genera la grilla con celdas de 10 px
- **THEN** la grilla tiene 3 × 2 celdas y son calle solo la (0, 0) y la (2, 1)
- Test: `StreetsGeneratorTest.ClassifiesStreetPixels` (`tests/client/test_streets_generator.cpp`)

### Requirement: El camino por arriba de un puente no baja
El camino entre dos checkpoints SHALL recorrer celdas de calle vecinas y llevar la capa de un auto: pasa a
`ON_BRIDGE` al entrar desde el suelo a un sensor `ON_BRIDGE` y vuelve a `ON_GROUND` al salir de él. Con capa
`ON_BRIDGE` MUST NOT atravesar las barandas del puente.

#### Scenario: Cruce por el camino de arriba (5.6)
- **GIVEN** el cruce de test del Background (puente en (100..160, 100..160), camino de arriba horizontal con sensor
  `ON_BRIDGE` (40..220, 110..150) y barandas en y = 100 y y = 160, camino de abajo vertical con sensor
  `UNDER_BRIDGE` (110..150, 40..220), rodeo) y P = (0, 130), Q = (260, 130), a los dos lados del camino de arriba
- **WHEN** se calcula el camino de P a Q
- **THEN** el camino mide 260 px (sigue derecho), no pisa ninguna celda del camino de abajo fuera del rectángulo
  del puente y su capa en cada celda dentro del rectángulo del puente es `ON_BRIDGE`
- Test: `StreetPathTest.OverBridgeGoesStraightOnTop` (`tests/test_streets_grid.cpp`)

### Requirement: El camino por debajo de un puente sigue por debajo
El camino SHALL pasar a `UNDER_BRIDGE` al entrar desde el suelo a un sensor `UNDER_BRIDGE` y volver a `ON_GROUND` al
salir de él. Un camino que ya tiene capa de puente MUST NOT cambiarla al entrar a otro sensor. Con capa
`UNDER_BRIDGE` atraviesa las barandas, que solo frenan a los autos de arriba.

#### Scenario: Cruce por el camino de abajo (5.7)
- **GIVEN** el cruce de test del Background y P = (130, 0), Q = (130, 260), a los dos lados del camino de abajo
- **WHEN** se calcula el camino de P a Q
- **THEN** el camino mide 260 px, pasa por las celdas de las barandas (y = 100 y y = 160) y su capa en cada celda
  dentro del rectángulo del puente es `UNDER_BRIDGE`
- Test: `StreetPathTest.UnderBridgeCrossesBelowTheRails` (`tests/test_streets_grid.cpp`)

### Requirement: El camino no dobla dentro de un puente
Dentro del rectángulo de un puente, el camino MUST seguir en la dirección con la que entró al rectángulo: no puede
pasar del camino de arriba al de abajo ni al revés dentro del cruce. Para unir los dos caminos SHALL ir por calles
de afuera del cruce.

#### Scenario: De arriba hacia abajo va por el rodeo (5.8)
- **GIVEN** el cruce de test del Background, con el rodeo que une el extremo este del camino de arriba (260, 130) con
  el extremo sur del camino de abajo (130, 260) pasando por (260, 260), y P = (0, 130) en el camino de arriba y
  Q = (130, 0) en el de abajo
- **WHEN** se calcula el camino de P a Q
- **THEN** el camino pasa por (260, 260), mide 780 px y no cambia de dirección en ninguna celda dentro del
  rectángulo del puente
- Test: `StreetPathTest.FromTopToBottomTakesTheDetour` (`tests/test_streets_grid.cpp`)

#### Scenario: De abajo hacia arriba va por el rodeo (5.8)
- **GIVEN** el cruce de test del Background con P = (130, 0) en el camino de abajo y Q = (0, 130) en el de arriba
- **WHEN** se calcula el camino de P a Q
- **THEN** el camino pasa por (260, 260), mide 780 px y no cambia de dirección en ninguna celda dentro del
  rectángulo del puente
- Test: `StreetPathTest.FromBottomToTopTakesTheDetour` (`tests/test_streets_grid.cpp`)

### Requirement: Checkpoints fuera de la calle
Si un checkpoint no cae sobre una celda de calle (o cae fuera de la grilla), el camino SHALL arrancar o terminar en
la celda de calle cuyo centro está más cerca del checkpoint, y el power-up MUST quedar en el punto medio de ese
camino.

#### Scenario: P fuera de la calle (5.9)
- **GIVEN** la grilla de test con solo la calle recta, P = (50, 40) (fuera de la calle) y Q = (200, 0)
- **WHEN** se calcula el camino de P a Q
- **THEN** el camino arranca en la celda de centro (50, 0), mide 150 px y su punto medio es (125, 0)
- Test: `StreetPathTest.StartsFromNearestStreetCell` (`tests/test_streets_grid.cpp`)

### Requirement: Tramo sin camino por la calle
Si no hay camino por la calle entre los dos checkpoints de un tramo, el power-up SHALL ubicarse en (P + Q) / 2 y el
servidor MUST informarlo con un mensaje en el log de errores que nombra los checkpoints del tramo.

#### Scenario: Dos calles sin conexión (5.10)
- **GIVEN** una grilla de test con la calle recta y otra calle horizontal de (0, 100) a (200, 100) sin conexión con
  ella, y los checkpoints 1 y 2 en P = (0, 0) y Q = (200, 100)
- **WHEN** se cargan los power-ups con esa grilla
- **THEN** el primer power-up está en (P + Q) / 2 = (100, 50) y el log de errores tiene un mensaje que menciona el
  tramo `1 -> 2`
- Test: `StreetPowerUpTest.NoPathFallsBackToMidpointAndReportsStretch` (`tests/test_power_up.cpp`)

### Requirement: Cargar una carrera nunca falla por la ubicación
Cargar los power-ups de cualquier carrera, incluidas las nuevas del editor con checkpoints arbitrarios, MUST NOT
fallar: cada power-up SHALL quedar en el punto medio del camino de su tramo o, si no lo hay, en (P + Q) / 2.

#### Scenario: Carrera con checkpoints arbitrarios (5.12)
- **GIVEN** la grilla de 5.10 (dos calles sin conexión) y una carrera de 6 checkpoints: (0, 0), (200, 0) sobre la
  primera calle; (0, 0), (200, 100) en calles distintas; (1000, 1000) fuera de la grilla y (0, 100)
- **WHEN** se cargan los power-ups con esa grilla
- **THEN** la carga no falla y hay 3 power-ups: en (100, 0) (camino), en (100, 50) ((P + Q) / 2, sin camino) y en
  (100, 100) (camino desde la celda de calle más cercana a (1000, 1000), la (200, 100))
- Test: `StreetPowerUpTest.ArbitraryCheckpointsNeverFail` (`tests/test_power_up.cpp`)

### Requirement: Las carreras del juego tienen sus power-ups sobre la calle
Con las grillas de calles y los puentes de las tres ciudades, todos los power-ups de las carreras incluidas en el
juego SHALL quedar sobre una celda de calle, y ningún tramo MUST caer en el caso sin camino.

#### Scenario: Carreras fijas de test (5.11)
- **GIVEN** las carreras fijas de `tests/data/races/` (copia de las 6 de `server/races/` al crear el hito), las
  grillas de `server/world/streets/` y los sensores, barandas y puentes de `server/world/`
- **WHEN** se cargan los power-ups de cada carrera con la grilla y los puentes de su ciudad
- **THEN** cada tramo tiene camino por la calle y cada power-up está sobre una celda de calle
- Test: `RealRacesPowerUpTest.AllPowerUpsLandOnStreets` (`tests/test_power_up.cpp`)

### Requirement: Las grillas de calles se instalan
El repositorio SHALL incluir `server/world/streets/streets_<ciudad>.json` para Liberty City, San Andreas y Vice
City; la instalación MUST dejarlos en `/var/nfs/world/streets/` y `world_config.yaml` MUST apuntar a ellos.

#### Scenario: La configuración apunta a las grillas (5.13)
- **GIVEN** el `world_config.yaml` del repositorio (excepción documentada: el test verifica la configuración, no
  usa sus valores de juego)
- **WHEN** se lee la clave `streets` de cada ciudad
- **THEN** es `/var/nfs/world/streets/streets_<ciudad>.json` y el archivo correspondiente existe en
  `server/world/streets/`
- Test: `StreetsGridInstallTest.ConfigPointsToTheCityGrids` (`tests/test_streets_grid.cpp`)

#### Scenario: Grillas instaladas (5.13)
- **GIVEN** el juego instalado con `installer.sh` (si `/var/nfs/world` no existe, el test se saltea)
- **WHEN** se buscan las grillas de calles
- **THEN** existen `/var/nfs/world/streets/streets_liberty_city.json`, `streets_san_andreas.json` y
  `streets_vice_city.json`
- Test: `StreetsGridInstallTest.GridsAreInstalled` (`tests/test_streets_grid.cpp`)
