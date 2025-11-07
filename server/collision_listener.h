#ifndef COLLISION_LISTENER_H
#define COLLISION_LISTENER_H


#include <box2d/box2d.h>
#include <unordered_map>
#include <cstdint>


#include "player.h"


class CollisionListener : public b2ContactListener {


    std::unordered_map<uint32_t, Player>& players;


public:
    CollisionListener(const CollisionListener&) = delete;
    CollisionListener& operator=(const CollisionListener&) = delete;

    explicit CollisionListener(std::unordered_map<uint32_t, Player>& players);

    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
};

#endif
