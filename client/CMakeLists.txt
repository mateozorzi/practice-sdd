
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

    Lobby/lobby.cpp
    Lobby/selectcars.cpp
    menu_interface.cpp

    ${QRC_FILES}
    ${UI_FILES}

    PUBLIC
    # .h files
    client_protocol.h
    client.h
    sender.h
    receiver.h
    camera.h

    Lobby/lobby.h
    Lobby/selectcars.h
    menu_interface.h


    )
