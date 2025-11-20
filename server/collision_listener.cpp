#include <iostream>
#include <cmath>


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

    uintptr_t sensor_data = sensor_fixture->GetUserData().pointer;

    SensorType sensor_type = static_cast<SensorType>(sensor_data);
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
            break;
        }
    }
}


float CollisionListener::calculate_damage_two_cars_collision(b2Body* car_body_a, b2Body* car_body_b, float relative_speed) {

    b2Vec2 forward_vector_a = car_body_a->GetWorldVector(b2Vec2(0.0f, 1.0f));
    b2Vec2 forward_vector_b = car_body_b->GetWorldVector(b2Vec2(0.0f, 1.0f));
    
    float dot_product = b2Dot(forward_vector_a, forward_vector_b);
    float angle_factor = (1.0f - dot_product) / 2.0f;
    
    if (angle_factor < 0.5f)
        angle_factor = 0.1f;
    else if (angle_factor < 0.7f)
        angle_factor = 0.3f;
    else
        angle_factor = 1.0f;

    float speed_factor = relative_speed / 10.0f;
    if (speed_factor > 1.0f) speed_factor = 1.0f;
    if (speed_factor < 0.2f) speed_factor = 0.2f;

    float base_damage = 10.0f;
    float taken_damage = base_damage * angle_factor * speed_factor;
    
    return taken_damage;
}


void CollisionListener::handle_collision_damage(b2Contact* contact) {

    b2Body* body_a = contact->GetFixtureA()->GetBody();
    b2Body* body_b = contact->GetFixtureB()->GetBody();

    b2Vec2 velocity_a = body_a->GetLinearVelocity();
    b2Vec2 velocity_b = body_b->GetLinearVelocity();

    b2Vec2 relative_velocity = velocity_a - velocity_b;
    float relative_speed = relative_velocity.Length();

    if (relative_speed < 1.0f)
        return;

    Player* player_a = nullptr;
    Player* player_b = nullptr;
    bool is_car_a = false;
    bool is_car_b = false;
    
    for (auto& [client_id, player]: players) {

        b2Body* car_body = player.get_car_body();
        if (car_body == body_a) {

            player_a = &player;
            is_car_a = true;
        }
        if (car_body == body_b) {

            player_b = &player;
            is_car_b = true;
        }
    }

    // TWO CARS COLLISION
    if (is_car_a && is_car_b && player_a && player_b) {

        float taken_damage = calculate_damage_two_cars_collision(body_a, body_b, relative_speed);

        player_a->apply_collision_damage(static_cast<uint16_t>(taken_damage));
        player_b->apply_collision_damage(static_cast<uint16_t>(taken_damage));
        
        std::cout << "Player with ID " << player_a->get_client_id() 
                  << " Has collided - Remaining HP: " << player_a->get_health() << std::endl;

        std::cout << "Player with ID " << player_b->get_client_id() 
                  << " Has collided - Remaining HP: " << player_b->get_health() << std::endl;
    
        float velocity_reduction = 0.5f + (taken_damage / 100.0f);
    
        if (velocity_reduction > 0.9f) velocity_reduction = 0.9f;
    
        float scale_a = 1.0f - velocity_reduction;
        float scale_b = 1.0f - velocity_reduction;
        
        body_a->SetLinearVelocity(b2Vec2(velocity_a.x * scale_a, velocity_a.y * scale_a));
        body_b->SetLinearVelocity(b2Vec2(velocity_b.x * scale_b, velocity_b.y * scale_b));
        
        return;
    }
    
    // CAR COLLIDING WITH STATIC BODY
    if (is_car_a && player_a) {

        float damage = relative_speed * 2.0f;
        if (damage > 40.0f) damage = 40.0f;
        
        player_a->apply_collision_damage(static_cast<uint16_t>(damage));
        
        std::cout << "Player with ID " << player_a->get_client_id() 
                  << " Has collided with an static object - Remaining HP: " << player_a->get_health() << std::endl;
        
        body_a->SetLinearVelocity(b2Vec2(velocity_a.x * 0.3f, velocity_a.y * 0.3f));
    } else if (is_car_b && player_b) {

        float damage = relative_speed * 2.0f;
        if (damage > 40.0f) damage = 40.0f;
        
        player_b->apply_collision_damage(static_cast<uint16_t>(damage));

        std::cout << "Player with ID " << player_b->get_client_id() 
            << " Has collided with an static object - Remaining HP: " << player_b->get_health() << std::endl;
        
        body_b->SetLinearVelocity(b2Vec2(velocity_b.x * 0.3f, velocity_b.y * 0.3f));
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

    processed_collisions.erase(contact);
}

void CollisionListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {

    (void)oldManifold;
    b2Fixture* fixture_a = contact->GetFixtureA();
    b2Fixture* fixture_b = contact->GetFixtureB();

    if (fixture_a->IsSensor() || fixture_b->IsSensor())
        return;

    if (processed_collisions.find(contact) != processed_collisions.end())
        return;

    processed_collisions.insert(contact);

    handle_collision_damage(contact);
}