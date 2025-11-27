target_sources(nfs_common
    PRIVATE
    # .cpp files
    car_catalogue_dto.cpp
    car_position_dto.cpp
    common_protocol.cpp
    movement_dto.cpp
    liberror.cpp
    player_dto.cpp
    resolver.cpp
    resolvererror.cpp
    socket.cpp
    world_snapshot_dto.cpp
    race_dto.cpp
    movement_dto.cpp
    PUBLIC
    # .h files
    car_catalogue_dto.h
    car_position_dto.h
    common_protocol.h
    game_command_dto.h
    movement_dto.h
    liberror.h
    player_dto.h
    queue.h
    resolver.h
    resolvererror.h
    socket.h
    thread.h
    movement_dto.h

    lobby_dto.h
    car_dto.h
    world_snapshot_dto.h
    race_dto.h
    checkpoint_dto.h
    race_catalogue_dto.h
    npc_dto.h

    player_race_stats_dto.h
    leaderboard_dto.h
    game_update_dto.h
    isocket.h
    )
