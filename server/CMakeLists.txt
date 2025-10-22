target_sources(taller_server
    PRIVATE
    # .cpp files
    acceptor.cpp
    client_handler.cpp
    main.cpp
    server.cpp
    PUBLIC
    # .h files
    acceptor.h
    client_handler.h
    server.h
    )
