target_sources(taller_common
    PRIVATE
    # .cpp files
    common_protocol.cpp
    foo.cpp
    liberror.cpp
    resolver.cpp
    resolvererror.cpp
    socket.cpp
    PUBLIC
    # .h files
    common_protocol.h
    foo.h
    liberror.h
    queue.h
    resolver.h
    resolvererror.h
    socket.h
    thread.h
    )
