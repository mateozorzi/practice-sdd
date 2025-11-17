#include <iostream>


#include "collision_listener.h"
#include "world_utils.h"


CollisionListener::CollisionListener(std::unordered_map<uint32_t, Player>& players):
        players(players) {}
    

void CollisionListener::handle_sensor_contact(b2Contact* contact, bool is_begin) {

    b2Fixture* fixture_a = contact->GetFixtureA();
    b2Fixture* fixture_b = contact->GetFixtureB();

    b2Fixture* sensor_fixture = nullptr;
    b2Fixture* car_fixture = nullptr;

    if (fixture_a->IsSensor()) {

        sensor_fixture = fixture_a;
        car_fixture = fixture_b;
    } else if (fixture_b->IsSensor()) {

        sensor_fixture = fixture_b;
        car_fixture = fixture_a;
    } else {

        return;
    }

    SensorType* sensor_type_ptr = reinterpret_cast<SensorType*>(
        sensor_fixture->GetUserData().pointer
    );

    if (!sensor_type_ptr) 
        return;

    SensorType sensor_type = *sensor_type_ptr;
    b2Body* car_body = car_fixture->GetBody();
    
    for (auto& [client_id, player]: players) {
        b2Body* player_body = player.get_car_body();
    
        CarLayer current_car_layer = player.get_car_layer();

        if (player_body == car_body) {

            if (is_begin) {     
           
                if (current_car_layer == CarLayer::ON_GROUND) {
    
                    if (sensor_type == SensorType::ON_BRIDGE) {
                        player.set_car_layer(CarLayer::ON_BRIDGE);
                        std::cerr << "Player with ID: " << client_id << " is ON_BRIDGE" << std::endl;
                    }
                    else if (sensor_type == SensorType::UNDER_BRIDGE) {
                        player.set_car_layer(CarLayer::UNDER_BRIDGE);
                        std::cerr << "Player with ID: " << client_id << " is UNDER_BRIDGE" << std::endl;
                    }
                }
            } else {

                if ((sensor_type == SensorType::ON_BRIDGE && current_car_layer == CarLayer::ON_BRIDGE) ||
                    (sensor_type == SensorType::UNDER_BRIDGE && current_car_layer == CarLayer::UNDER_BRIDGE)) {

                    player.set_car_layer(CarLayer::ON_GROUND);
                    std::cerr << "Player with ID: " << client_id << " is ON_GROUND" << std::endl;
                }
            }
        }
        break;
    }
    
}

void CollisionListener::BeginContact(b2Contact* contact) {

    b2Fixture* fixture_a = contact->GetFixtureA();
    b2Fixture* fixture_b = contact->GetFixtureB();
    
    if (fixture_a->IsSensor() || fixture_b->IsSensor()) {
        handle_sensor_contact(contact, true);
        return;
    }

    const b2Body* body_a = fixture_a->GetBody();
    const b2Body* body_b = fixture_b->GetBody();

    for (auto& [client_id, player]: players) {

        const b2Body* car_body = player.get_car_body();
        if (car_body == body_a || car_body == body_b)
            player.set_car_collision(true);
    }
}

void CollisionListener::EndContact(b2Contact* contact) {

    b2Fixture* fixture_a = contact->GetFixtureA();
    b2Fixture* fixture_b = contact->GetFixtureB();
    
    if (fixture_a->IsSensor() || fixture_b->IsSensor()) {
        handle_sensor_contact(contact, false);
        return;
    }

    const b2Body* body_a = fixture_a->GetBody();
    const b2Body* body_b = fixture_b->GetBody();

    for (auto& [client_id, player]: players) {

        const b2Body* player_body = player.get_car_body();
        if (player_body == body_a || player_body == body_b)
            player.set_car_collision(false);
    }
}
