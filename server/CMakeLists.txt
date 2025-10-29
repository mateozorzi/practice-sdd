target_sources(taller_server
    PRIVATE
    # .cpp files
    acceptor.cpp
    car.cpp
    client_handler.cpp
    game_loop.cpp
    main.cpp
    receiver.cpp
    sender.cpp
    server.cpp
    server_protocol.cpp
    PUBLIC
    # .h files
    acceptor.h
    car.h
    client_handler.h
    client_request.h
    game_loop.h
    receiver.h
    sender.h
    server.h
    server_protocol.h
    )
