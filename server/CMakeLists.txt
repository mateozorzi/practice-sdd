target_sources(taller_server
    PRIVATE
    # .cpp files
    acceptor.cpp
    client_handler.cpp
    main.cpp
    sender.cpp
    server.cpp
    PUBLIC
    # .h files
    acceptor.h
    client_handler.h
    sender.h
    server.h
    )
