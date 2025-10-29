target_sources(taller_common
    PRIVATE
    # .cpp files
    car_position_dto.cpp
    common_protocol.cpp
    foo.cpp
    movement_dto.cpp
    liberror.cpp
    resolver.cpp
    resolvererror.cpp
    socket.cpp
    PUBLIC
    # .h files
    car_position_dto.h
    common_protocol.h
    foo.h
    movement_dto.h
    liberror.h
    queue.h
    resolver.h
    resolvererror.h
    socket.h
    thread.h
    )
