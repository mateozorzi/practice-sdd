#ifndef COLLISION_LISTENER_H
#define COLLISION_LISTENER_H

#include <box2d/box2d.h>
#include <unordered_map>
#include <cstdint>

class Car;

class CollisionListener : public b2ContactListener {
private:
    std::unordered_map<uint32_t, Car>& cars;

public:
    explicit CollisionListener(std::unordered_map<uint32_t, Car>& cars);

    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
};

#endif
