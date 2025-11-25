target_sources(nfs_tests
    PRIVATE
    # .cpp files
    test_client.cpp
    test_server.cpp
    ../client/client_protocol.cpp
    ../server/server_protocol.cpp
    

    PUBLIC
    # .h files
    test_client.h
    test_server.h
    mock_socket.h
    ../client/client_protocol.h
    )
