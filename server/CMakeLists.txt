target_sources(taller_server
    PRIVATE
    # .cpp files
    acceptor.cpp
    client_handler.cpp
    main.cpp
    receiver.cpp
    sender.cpp
    server.cpp
    server_protocol.cpp
    PUBLIC
    # .h files
    acceptor.h
    client_handler.h
    client_request.h
    receiver.h
    sender.h
    server.h
    server_protocol.h
    )
