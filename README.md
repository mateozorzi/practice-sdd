# Introducción

**Need4Speed** es un juego 2D inspirado en *Need for Speed*,
desarrollado en C++ con SDL2, Qt y Box2D.\
El proyecto incluye tres aplicaciones principales:

-   **Cliente** (Corredores)
-   **Servidor** (Sincronización de carreras)
-   **Editor gráfico** (Creación de circuitos)

------------------------------------------------------------------------

# Instalación Automática

Desde la carpeta raíz del proyecto puede ejecutarse una instalación
completa (dependencias y compilación):

``` bash
chmod +x installer.sh
./installer.sh
```

------------------------------------------------------------------------

# Ejecución del Proyecto

El proyecto consta de tres aplicaciones: **Editor**, **Servidor** y
**Cliente**.

## Editor

Para ejecutarlo:

``` bash
nfs_editor
```

Permite:

-   Crear circuitos nuevos
-   Editar circuitos existentes

------------------------------------------------------------------------

## Servidor y Cliente

Para iniciar una partida se necesita un servidor activo y al menos un
cliente conectado.

1.  Ejecutar el servidor:

    ``` bash
    nfs_server
    ```

2.  En otra terminal, iniciar un cliente:

    ``` bash
    nfs_client
    ```

Al iniciar el cliente, primero se debe indicar dirección IP y puerto del
servidor.\
Luego se ingresa el nombre de usuario para habilitar el menú principal.

------------------------------------------------------------------------

# Menú del Cliente

El jugador puede:

-   **Crear una partida**
-   **Unirse a una partida**

## Crear una partida

El creador debe:

1.  Elegir nombre de la partida y cantidad de jugadores.
2.  Seleccionar uno o más circuitos (se muestra nombre, ciudad,
    checkpoints y vueltas).
3.  Elegir un auto. Cada auto tiene configuraciones únicas.

Cuando todos los jugadores eligen auto, comienza la partida.

## Unirse a una partida

El jugador ve la lista de partidas disponibles (no completas).\
Al seleccionar una, pasa al menú de elección de autos.\
Si la partida alcanza el cupo, inicia automáticamente.

------------------------------------------------------------------------

# Partida

## Controles

-   **W**: Avanzar
-   **A**: Izquierda
-   **S**: Retroceder
-   **D**: Derecha
-   **Q**: Salir

## Mejoras

Antes de cada carrera, durante 10 segundos, el jugador puede mejorar
velocidad, aceleración o vida.\
Cada mejora agrega una penalización de 10 segundos al tiempo final de la
carrera.\
Máximo de 4 mejoras por carrera, persistentes para toda la partida.

## Desarrollo de la carrera

-   Cada carrera tiene checkpoints y varias vueltas.
-   Los jugadores tienen una barra de vida.
-   Colisiones con edificios o autos restan vida.
-   Si un jugador pierde toda la vida, queda descalificado de esa
    carrera.
-   En la pista aparecen power-ups, uno entre cada par de checkpoints
    (1→2, 3→4, …), sobre la calle: a mitad del camino que haría un auto
    entre los dos checkpoints, cruzando los puentes por arriba o por
    debajo. El primer auto que pasa por uno recibe su efecto:
    -   Nitro: más velocidad máxima y aceleración por unos segundos.
    -   Reparación: recupera vida, sin superar la vida máxima.
    -   Escudo: los choques no restan vida por unos segundos.
-   Un power-up recogido desaparece para todos y vuelve a aparecer en el
    mismo lugar, con un tipo al azar, después de un tiempo.
-   Al recoger un power-up suena el mismo sonido que las mejoras.
-   Mientras dura un Nitro o un Escudo, sobre la barra de vida aparece un
    cartel con el ícono del power-up y los segundos que le quedan (por
    ejemplo `3 SEG`). Con los dos activos se muestran ambos carteles.

Al finalizar:

-   Se muestra la clasificación y los puntos obtenidos.
-   Se aplican penalizaciones por mejoras.

Al terminar todas las carreras se muestra la clasificación general.\
En caso de empate, gana el jugador con menor tiempo total.

------------------------------------------------------------------------

# Cheats

Durante una carrera pueden activarse:

-   1 Ganar carrera automáticamente
-   2 Ganar partida automáticamente
-   3 Activar vida infinita

------------------------------------------------------------------------

# Configuraciones Extras

Las configuraciones del juego se pueden modificar desde archivos YAML.\
Es posible ajustar:

-   Atributos de los autos (model, speed, acceleration, handling)
-   Valores de mejoras
-   Power-ups (multiplicador y duración del nitro, vida que repara la
    reparación, duración del escudo y tiempo de reaparición), en la
    sección `power_ups:` de `game_config.yaml`
-   Grillas de calles de cada ciudad (`streets:` en `world_config.yaml`),
    donde se ubican los power-ups. Se generan desde las máscaras de
    `server/tools/streets/masks/` con `make streets` y se instalan en
    `/var/nfs/world/streets/`; sin grilla, cada power-up va al punto medio
    en línea recta entre sus dos checkpoints
-   Parámetros del servidor (FPS, rendimiento)
-   Aspectos visuales (tamaño de sprites, etc.)

------------------------------------------------------------------------

# Créditos
Este proyecto utiliza tanto el socket, como el thread y queue provistos por la cátedra:  

[sockets-en-cpp](https://github.com/eldipa/sockets-en-cpp) desarrollado por [eldipa](https://github.com/eldipa), distribuido bajo la licencia GPL v2.

[hands-on-threads](https://github.com/eldipa/hands-on-threads) desarrollado por [eldipa](https://github.com/eldipa), distribuido bajo la licencia GPL v2
