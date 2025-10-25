target_sources(taller_client
    PRIVATE
    # .cpp files
    main.cpp
    client_protocol.cpp
    client.cpp
    sender.cpp
    receiver.cpp
    PUBLIC
    # .h files
    client_protocol.h
    client.h
    sender.h
    receiver.h
    )
