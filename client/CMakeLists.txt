
file(GLOB QRC_FILES "Lobby/*.qrc")

file(GLOB UI_FILES "Lobby/*.ui")

target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    client_protocol.cpp
    client.cpp
    sender.cpp
    receiver.cpp
    camera.cpp
    input_manager.cpp
    sdl_manager.cpp

    Lobby/lobby.cpp
    Lobby/selectcars.cpp
    Lobby/creategame.cpp
    Lobby/createlobby.cpp
    Lobby/joinlobby.cpp
    Lobby/waitingroom.cpp
    menu_interface.cpp
    sounds.cpp

    ${QRC_FILES}
    ${UI_FILES}

    PUBLIC
    # .h files
    client_protocol.h
    client.h
    sender.h
    receiver.h
    camera.h
    input_manager.h
    sdl_manager.h
    sounds.h

    Lobby/lobby.h
    Lobby/selectcars.h
    menu_interface.h
    Lobby/creategame.h
    Lobby/createlobby.h
    Lobby/joinlobby.h
    Lobby/waitingroom.h


    )
