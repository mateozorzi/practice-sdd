#include "collision_listener.h"

CollisionListener::CollisionListener(std::unordered_map<uint32_t, Player>& players):
        players(players) {}

void CollisionListener::BeginContact(b2Contact* contact) {

    const b2Body* bodyA = contact->GetFixtureA()->GetBody();
    const b2Body* bodyB = contact->GetFixtureB()->GetBody();

    for (auto& [client_id, player]: players) {

        const b2Body* car_body = player.get_car_body();
        if (car_body == bodyA || car_body == bodyB)
            player.set_car_collision(true);
    }
}

void CollisionListener::EndContact(b2Contact* contact) {

    const b2Body* bodyA = contact->GetFixtureA()->GetBody();
    const b2Body* bodyB = contact->GetFixtureB()->GetBody();

    for (auto& [client_id, player]: players) {

        const b2Body* player_body = player.get_car_body();
        if (player_body == bodyA || player_body == bodyB)
            player.set_car_collision(false);
    }
}
