target_sources(taller_common
    PRIVATE
    # .cpp files
    car_catalogue_dto.cpp
    car_position_dto.cpp
    common_protocol.cpp
    foo.cpp
    movement_dto.cpp
    liberror.cpp
    player_dto.cpp
    resolver.cpp
    resolvererror.cpp
    socket.cpp
    PUBLIC
    # .h files
    car_catalogue_dto.h
    car_position_dto.h
    common_protocol.h
    game_command_dto.h
    foo.h
    movement_dto.h
    liberror.h
    player_dto.h
    queue.h
    resolver.h
    resolvererror.h
    socket.h
    thread.h

    lobby_dto.h
    car_dto.h
    )
