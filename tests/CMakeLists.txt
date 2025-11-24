target_sources(nfs_tests
    PRIVATE
    # .cpp files
    test_client.cpp
    ../client/client_protocol.cpp
    

    PUBLIC
    # .h files
    test_client.h
    mock_socket.h
    ../client/client_protocol.h
    )
