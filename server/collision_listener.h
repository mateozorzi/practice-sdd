#ifndef COLLISION_LISTENER_H
#define COLLISION_LISTENER_H

#include <optional>
#include <cstdint>
#include <unordered_map>
#include <set>

#include <box2d/box2d.h>

#include "player.h"

class CollisionListener: public b2ContactListener {

    std::unordered_map<uint32_t, Player>& players;
    std::set<b2Contact*> processed_collisions;

    std::optional<std::reference_wrapper<Player>> find_player_by_body(const b2Body& body);
    void update_car_collision_state(const b2Contact& contact, bool is_colliding);
    float calculate_damage_two_cars_collision(const b2Body& body_a, const b2Body& body_b, float relative_speed);

    void handle_sensor_contact(b2Contact& contact, bool is_begin);

    void handle_two_car_collision(Player& player_a, Player& player_b, b2Body& body_a, b2Body& body_b, float relative_speed);
    void handle_static_collision(Player& player, b2Body& body, float speed);
    void handle_collision(b2Contact& contact);

public:
    CollisionListener(const CollisionListener&) = delete;
    CollisionListener& operator=(const CollisionListener&) = delete;

    explicit CollisionListener(std::unordered_map<uint32_t, Player>& players);

    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;
    void PreSolve(b2Contact* contact, const b2Manifold*) override;
};

#endif
