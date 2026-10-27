# Prompts del flujo SDD — practice-sdd

Prompts enviados a Claude Code en `~/Escritorio/tp/practice-sdd`, en orden cronológico, desde la preparación (definición de la feature, `openspec init` y constitution) hasta el cierre del último (sin los análisis de Sonar). Hora local de Buenos Aires.

## Preparación — Definición de la feature (TODO)

**1.** `21/10 10:39`

```
[texto pegado]

# TODO: Nueva feature para Need4Speed

Creacion de nueva feature con Spec-Driven Development (OpenSpec) sobre este proyecto brownfield.

Feature: Durante las carreras deberan aparecer power-ups en la pista, estos le daran al jugador que lo consiga un efecto con duración limitada.

## Flujo de trabajo
Claude + OpenSpec
Ciclo de open spec, a partir de una nueva feature:
1. Creacion de constitution, se crea una vez y es como la doc base del proyecto, se crea un roadmap, la mision del proyecto
2. Creacion del plan, /opsx:propose -> El modelo propone un plan para conseguir el primer hito del roadmap
3. Aplicar tareas -> Claude codea la feature, se configuran hooks para la comporbacion de linter, cppcheck y tests
4. Actualizacion doc
5. Repetir con el siguiente hito

## Por qué esta feature
- Pasa por todas las capas (config servidor, física Box2D, gameloop, protocolos, render SDL del cliente).

## Alcance (MVP)
Los efectos se calculan solo en el servidor.

| Tipo       | Efecto                                                   | Duración      |
|------------|----------------------------------------------------------|---------------|
| Nitro      | Multiplica la velocidad máxima y la aceleración x1.25    | 3 s           |
| Reparación | Suma 50 de salud, sin pasar la salud máxima              | Instantáneo   |
| Escudo     | Ignora el daño por choques                               | 5 s           |

Reglas a especificar:
- [ ] Aparición: los spawn points se aparecen entre checkpoints.
- [ ] Sensor mediante Box2D.
- [ ] Cooldown de reaparición de 20s.
- [ ] Un solo efecto activo por tipo, volver a agarrarlo renueva la duración.
- [ ] El cheat de vida infinita debe seguir funcionando luego de utilizar escudo o reparación.
- [ ] Los autos muertos no recogen power-ups.
- [ ] Limpieza de power-ups y efectos entre carreras.
- [ ] No se pueden colocar power-ups desde el editor de carreras.


 Estyo escribiendo este archivo TODO para la creacion del nuevo feature para el proyecto, primero queiro desarrollar bienla documentacion antes de pasar al ciclo de OpenSpec. Me refinas los que escribi sobre el flujo de trabajo. Y luego tenemos que agregar test de aceptacion para que el modelo entienda cuando puede seguir avanzando
```

**2.** `21/10 10:45`

```
(respuesta al rechazar una acción) Bien escribir el TODO.md nomas
```

**3.** `21/10 10:57`

```
actualice un poco el TODO las reglas y demas, reescribir los test que sean necesarios o agrega mas
```

**4.** `21/10 11:03`

```
agrega una ragla que sea que se debe uindicar si el jugador tiene uno de los power ups activos, puede ser un cartel con el sprite por ejemplo y cuento tiempo le queda de efecto
```

**5.** `21/10 11:17`

```
agrega en cada hito un backgorund para condifugrar los datos que uses, por ejemplo el timepo de un power up, que no depende en los test del server porque si en un momento cambia entyonces los test fallarian
```

## Preparación — openspec init, constitution y explore

**6.** `21/10 11:58`

```
openspec init
```

**7.** `21/10 11:59`

```
si quiero construir primero un constitution que acompañe a todo el flujo, que comando deberia usar¡
```

**8.** `21/10 12:04`

```
(respuesta al rechazar una acción) Y si quiero crear un constitution tipo el de spec-kit, un prmpt asi serviria? que habria que confuigrar ademas para que open spec lo utilice/openspec-explore

En este proyecto brownfield, analiza el README.md y el TODO.md. Explora también la estructura del repositorio para entender cómo está organizado y qué tecnologías utiliza.

Antes de escribir o modificar archivos, entrevístame usando la herramienta AskUserQuestion. Pregúntame sobre:
- La misión del proyecto y el problema que busca resolver.
- La audiencia objetivo y sus necesidades principales.
- Las brechas, dudas o decisiones pendientes sobre el stack tecnológico.

Cuando haya respondido, revisa la información disponible y crea o actualiza estos archivos:
- openspec/specs/mission.md
- openspec/specs/tech-stack.md
- openspec/specs/roadmap.md

Usa la documentación y mis respuestas como fuentes. Si falta información importante, indícala como pendiente en lugar de asumirla. Al terminar, resume brevemente los archivos creados o modificados y las decisiones que quedaron abiertas.
```

**9.** `21/10 12:07`

```
(respuesta al rechazar una acción) okey hace eso, pero utiliza el prompt que te marque antes para la creacion del contitution + lo que se necesite de config de open spec
```

## Hito 1 — Configuración y modelo de efectos en el servidor

**10.** `22/10 17:49`

```
/opsx:propose Configuracion y modelo de efectos en el servidor
```

**11.** `22/10 18:06`

```
/opsx:apply
```

**12.** `22/10 18:22`

```
porque no puedo hacer ./opsx:verify?
```

**13.** `22/10 18:28`

```
/opsx:sync
```

**14.** `22/10 18:31`

```
/opsx:archive
```

**15.** `22/10 18:33`

```
que cambios estan pendientes?
```

**16.** `22/10 18:34`

```
si commitea
```

## Entre hitos — Pipeline de CI en GitHub

**17.** `22/10 18:36`

```
quiero que crees un pipeline para el repo que se ejecute al subirlo a github
```

**18.** `22/10 18:51`

```
y?
```

**19.** `22/10 18:52`

```
dale, pusheá
```

**20.** `22/10 18:54`

```
porque si hice el updata, aun no puedo hacer verify
```

**21.** `22/10 18:56`

```
y donde se sube el reporte de cobertura del pipeline de github?
```

**22.** `22/10 18:57`

```
chequea el error del pipeline
```

**23.** `22/10 19:00`

```
dale, pusheá
```

**24.** `22/10 19:06`

```
sigue fallando el pipeline, mira la sesion dechrome que tengo abuierta
```

**25.** `22/10 19:18`

```
porque hay test que se skippean?
```

## Hito 2 — Power-ups en el mundo físico

**26.** `22/10 19:21`

```
/opsx:propose Power-ups en el mundo físico
```

**27.** `22/10 19:29`

```
/opsx:apply
```

**28.** `22/10 19:37`

```
si arma commits atomicos primero, luego sigo con los pasos, no pushes
```

**29.** `22/10 19:41`

```
/opsx:verify
```

**30.** `22/10 19:42`

```
/opsx:sync
```

**31.** `22/10 19:43`

```
si realiza los cambios y luego el archive
```

**32.** `22/10 19:45`

```
crea los commit
```

## Hito 3 — Game loop y protocolo

**33.** `23/10 11:30`

```
/opsx:propose Game loop y protocolo
```

**34.** `23/10 11:40`

```
/opsx:update en la ruta /server/world/object_rendering estan los archivos decorations_*.json que contienen las coordenadas de los edificions y demas de los mapas del juego
```

**35.** `23/10 11:44`

```
y sobre las otras decisiones del design?
```

**36.** `23/10 11:45`

```
sí, aplicá los tres cambios
```

**37.** `23/10 11:46`

```
/opsx:apply
```

**38.** `23/10 12:16`

```
/opsx:verify
```

**39.** `23/10 12:19`

```
marcá la 6.2 y arreglá el warning 1 en código y luego hace el opensepc update
```

**40.** `23/10 12:24`

```
sí, armá los commits atómicos
```

**41.** `23/10 12:33`

```
/opsx:archive
```

**42.** `23/10 12:37`

```
sí, hacé el commit del archive
```

## Hito 4 — Render y sonido en el cliente

**43.** `23/10 12:40`

```
/opsx:propose Render y sonido en el cliente
```

**44.** `23/10 12:50`

```
/opsx:update que el tamaño del sprite como primer acercaameinto sea del ancho del spirte del auto compact y con forma cuadrada, la imagen no deberia tener color de fondo ya es transaparente
```

**45.** `23/10 12:52`

```
/opsx:apply
```

**46.** `23/10 13:04`

```
/opsx:verify
```

**47.** `23/10 13:06`

```
arregla todo lo del verify
```

**48.** `23/10 13:14`

```
/opsx:update las carreras funcionan bien. Dos puntos acerca de los power ups. Deberian aparecer entre dos checkpoints pero siempre sobre la calle, no sobre un edificio o pared porque se pierde un poco la inmersion, busca los archivos donde se establecen cuales son las calles trasitables del mapa no recuerdo el nombre ahora mismo. Y luego sobre el indicador de duracion de los power up, esta mostrando cuando quedan 3seg muestra 30 y baja de a 5, el efecto dura 3 segundos reales pero hay un error al mostrar el formato
```

**49.** `23/10 13:23`

```
/opsx:apply
```

**50.** `23/10 13:27`

```
/opsx:verify
```

**51.** `23/10 13:30`

```
marcá la 7.2 y archivá
```

**52.** `23/10 13:32`

```
me equivque, queria poner que si
```

## Exploración de los hitos nuevos (5 y 6)

**53.** `23/10 13:36`

```
tengo dos cambios que proponer para un nuevo hito que surgio en medio de la implementacion: power-ups-spawn-sobre-calle (ver task archivado del render de power ups) y luego hay algunos power ups que se supoerponen con los puentes, un power up que deberia estar en el puente y se ubica justo en el borde queda la mitad del pówer up abajo y la otra mitad siguen en la calle que no pertenece al puente. Deberian sert dos hitos o uno solo?
```

**54.** `23/10 13:38`

```
pero es que deberia ser por capas, un power up que esta en un puente no deberia poder ser agarrado por un auto que pasa por debajo. Hace un /opsx:explore para estos dos cambios enotnces en dos hitos distintos
```

**55.** `23/10 13:48`

```
si tu solucion sigue la logica de que autos puedan pasar por el puente o si vienen por debajo que puedan pasar pode debajo esta perfecto. Ahora luego en el hito 6 que los power ups que esten encima de un puetne solo puedan ser agarrados por lo que van por el punete y si esta por debajo dle puente que solo puedan ser agarrados por lo que estan yendo debajo del puente si. Hace una modificaion (podes dejarlo por escrito) que se agregan dos hitos en el roadmap del consitution y actrualizas las specs que hagan falta. Si no podes copiar las mascaras al servidor y que el script de paso este alli tambien, lo necesita el servidor no? sino en el cleinte como lo veas mejor. Y por ultimo si es correcto lo que decis de los puentes donde debajo hay agua, si te referis a que deberia pasar y en el medio de dos checkpoints justo el power up cae en el punete, se deberia ver si el camino pasa por enciuma del punete entonces el power up va encima, si el camino es por debajo del puente entonces el power up va por debajo, seria incorreto que la carrera pide pasar por debajo y que el power up este encima del puente
```

**56.** `23/10 13:57`

```
ocrrecion en los test del 5.11, en el editor se pueden agregar carreras, por lo que si en algun momento el cliente ccrea una carrera este test dejaria de funcionar
```

## Hito 5 — Power-ups sobre la calle

**57.** `23/10 13:59`

```
/opsx:propose Power-ups sobre la calle
```

**58.** `23/10 14:14`

```
/opsx:apply
```

**59.** `23/10 14:50`

```
/opsx:verify
```

**60.** `23/10 14:53`

```
aplica las surgerencias y archiva
```

## Hito 6 — Power-ups por capas en los puentes (+ explore/propose del editor)

**61.** `23/10 16:26`

```
/opsx:propose Power-ups por capas en los puentes
```

**62.** `23/10 16:35`

```
/opsx:apply
```

**63.** `23/10 16:53`

```
/opsx:verify
```

**64.** `23/10 16:55`

```
realiza tus sugerencias y verifica todo denuevos y marca la 6.2 como check
```

**65.** `23/10 17:04`

```
/opsx:explore quise crear una nueva carrera en el editor para probar los power ups under bridges. Cuando use el editro y le di a done me pregunto donde queria guardar el .cvs (?) que paso? no deberia preguntar, deberia guardar en la capreta races del server. Entro al server y no aaparece la carrera y los test fallan
```

**66.** `23/10 17:10`

```
/opsx:propose cambios editor para guardado correcto de las races y tests para que no deje de romper al agregar carreras
```

## Hito 6 (cont.) — Cambios del editor, verify, sync y archive

**67.** `23/10 17:15`

```
/opsx:apply
```

**68.** `23/10 17:24`

```
/opsx:update un minimo cambios, cuando en el editor quiero editar una ya creada, por predeterminado te abre el editor con el mapa arriba ala izq. Pero si la carrera se creo en otro punto del mapa es muy dificil enocontrar donde comienza. Podes hacer que se abra el mapa con el checkpoint 1 centrado?
```

**69.** `23/10 17:26`

```
perfecto
```

**70.** `23/10 17:27`

```
/opsx:apply
```

**71.** `23/10 17:31`

```
pero no es que no hacia falta que esten en server/races ahora? si cree otra desde el editor en la prueba manua
```

**72.** `23/10 17:34`

```
si funciona perfecto, ahora tengo una pregunta de funcionalidad. El editor lo usa el cliente para crear sus carreras y jugarals, porque al crear una carrera tengo que reiniciar el server, el cliente no tendria acceso a este. Es un cambio grande?
```

**73.** `23/10 17:35`

```
/opsx:verify
```

**74.** `23/10 17:37`

```
hiciste el verify de los dos cambios
```

**75.** `23/10 17:37`

```
aplica las suggestions y luego /opsx:verify power-ups-por-capas-en-los-puentes.
```

**76.** `23/10 17:39`

```
hace un sync para ambos cambios
```

**77.** `23/10 17:40`

```
/opsx:archive
```

**78.** `23/10 17:43`

```
crea commits atomicos para estos cambios que hiciste
```

**79.** `23/10 17:50`

```
porque cuando hago make run-tests a mi se me ejecutan 126 y los del server???
```
