#include "collision_listener.h"
#include "car.h"

CollisionListener::CollisionListener(std::unordered_map<uint32_t, Car>& cars) 
    : cars(cars) {}

void CollisionListener::BeginContact(b2Contact* contact) {
    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();

    // Check if either body belongs to a car and set collision state
    for (auto& [client_id, car] : cars) {
        if (car.get_body() == bodyA || car.get_body() == bodyB) {
            car.set_collision(true);
        }
    }
}

void CollisionListener::EndContact(b2Contact* contact) {
    b2Body* bodyA = contact->GetFixtureA()->GetBody();
    b2Body* bodyB = contact->GetFixtureB()->GetBody();

    // Check if either body belongs to a car and clear collision state
    for (auto& [client_id, car] : cars) {
        if (car.get_body() == bodyA || car.get_body() == bodyB) {
            car.set_collision(false);
        }
    }
}
