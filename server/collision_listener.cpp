#include <algorithm>
#include <iostream>
#include <cmath>


#include "collision_listener.h"
#include "world_utils.h"


CollisionListener::CollisionListener(std::unordered_map<uint32_t, Player>& players):
        players(players) {}
    

std::optional<std::reference_wrapper<Player>> CollisionListener::find_player_by_body(const b2Body& body) {

    for (auto& [client_id, player] : players) {

        if (player.get_car_body() == &body)
            return std::ref(player);
    }

    return std::nullopt;
}

void CollisionListener::update_car_collision_state(const b2Contact& contact, const bool is_colliding) {

    const b2Body* body_a = contact.GetFixtureA()->GetBody();
    const b2Body* body_b = contact.GetFixtureB()->GetBody();

    for (auto& [_, player] : players) {

        const b2Body* car_body = player.get_car_body();
        if (car_body == body_a || car_body == body_b)
            player.set_car_collision(is_colliding);
    }
}

float CollisionListener::calculate_damage_two_cars_collision(const b2Body& body_a, const b2Body& body_b, const float relative_speed) {
    
    b2Vec2 forward_a = body_a.GetWorldVector(b2Vec2(0.0f, 1.0f));
    b2Vec2 forward_b = body_b.GetWorldVector(b2Vec2(0.0f, 1.0f));
    
    float angle_factor = (1.0f - b2Dot(forward_a, forward_b)) / 2.0f;
    angle_factor = (angle_factor < 0.5f) ? 0.1f : (angle_factor < 0.7f) ? 0.3f : 1.0f;
    
    float speed_factor = std::clamp(relative_speed / 10.0f, 0.2f, 1.0f);
    
    return 10.0f * angle_factor * speed_factor;
}


void CollisionListener::handle_sensor_contact(b2Contact& contact, const bool is_begin) {

    auto [sensor_fixture, car_fixture] = [&]()->std::pair<b2Fixture*, b2Fixture*> {

        auto* fixture_a = contact.GetFixtureA();
        auto* fixture_b = contact.GetFixtureB();
        
        if (fixture_a->IsSensor()) return {fixture_a, fixture_b};
        if (fixture_b->IsSensor()) return {fixture_b, fixture_a};
        return {nullptr, nullptr};
    }();

    if (!sensor_fixture) return;

    auto sensor_type = static_cast<SensorType>(sensor_fixture->GetUserData().pointer);
    auto player_ptr = find_player_by_body(*car_fixture->GetBody());

    if (!player_ptr) return;
    
    Player& player = player_ptr->get();
    auto current_layer = player.get_car_layer();

    if (is_begin) {
        if (current_layer != CarLayer::ON_GROUND) return;
        
        if (sensor_type == SensorType::ON_BRIDGE) {
            player.set_car_layer(CarLayer::ON_BRIDGE);
            std::cerr << "Player with ID: " << player.get_client_id() << " is ON_BRIDGE" << std::endl;
        } else if (sensor_type == SensorType::UNDER_BRIDGE) {
            player.set_car_layer(CarLayer::UNDER_BRIDGE);
            std::cerr << "Player with ID: " << player.get_client_id() << " is UNDER_BRIDGE" << std::endl;
        }
    } else {
        bool should_reset = (sensor_type == SensorType::ON_BRIDGE && current_layer == CarLayer::ON_BRIDGE) ||
                           (sensor_type == SensorType::UNDER_BRIDGE && current_layer == CarLayer::UNDER_BRIDGE);
        
        if (should_reset) {
            player.set_car_layer(CarLayer::ON_GROUND);
            std::cerr << "Player with ID: " << player.get_client_id() << " is ON_GROUND" << std::endl;
        }
    }
}


void CollisionListener::handle_two_car_collision(Player& player_a, Player& player_b, b2Body& body_a, b2Body& body_b, const float relative_speed) {

    float damage = calculate_damage_two_cars_collision(body_a, body_b, relative_speed);

    player_a.apply_collision_damage(static_cast<uint16_t>(damage));
    player_b.apply_collision_damage(static_cast<uint16_t>(damage));

    std::cerr << "Player with ID: " << player_a.get_client_id()
              << " collided - HP: " << player_a.get_health() << '\n';
    std::cerr << "Player with ID: " << player_b.get_client_id()
              << " collided - HP: " << player_b.get_health() << '\n';

    float velocity_reduction = std::min(0.5f + damage / 100.0f, 0.9f);
    float scale = 1.0f - velocity_reduction;

    body_a.SetLinearVelocity(scale * body_a.GetLinearVelocity());
    body_b.SetLinearVelocity(scale * body_b.GetLinearVelocity());
}

void CollisionListener::handle_static_collision(Player& player, b2Body& body, float speed) {

    float damage = std::min(speed * 2.0f, 40.0f);
    player.apply_collision_damage(static_cast<uint16_t>(damage));

    std::cerr << "Player with ID: " << player.get_client_id()
              << " collided with static object - HP: " << player.get_health() << '\n';

    body.SetLinearVelocity(0.3f * body.GetLinearVelocity());
}

void CollisionListener::handle_collision(b2Contact& contact) {

    b2Body& body_a = *contact.GetFixtureA()->GetBody();
    b2Body& body_b = *contact.GetFixtureB()->GetBody();

    b2Vec2 relative_velocity = body_a.GetLinearVelocity() - body_b.GetLinearVelocity();
    float relative_speed = relative_velocity.Length();

    if (relative_speed < 1.0f) return;

    auto player_a_ptr = find_player_by_body(body_a);
    auto player_b_ptr = find_player_by_body(body_b);

    // Two cars colliding
    if (player_a_ptr && player_b_ptr) {

        handle_two_car_collision(player_a_ptr->get(), player_b_ptr->get(), body_a, body_b, relative_speed);
        return;
    }

    if (player_a_ptr) handle_static_collision(player_a_ptr->get(), body_a, relative_speed);
    else if (player_b_ptr) handle_static_collision(player_b_ptr->get(), body_a, relative_speed);
}


void CollisionListener::BeginContact(b2Contact* contact) {

    if (!contact) return;
    
    if (contact->GetFixtureA()->IsSensor() || contact->GetFixtureB()->IsSensor()) {
        handle_sensor_contact(*contact, true);
        return;
    }

    update_car_collision_state(*contact, true);
}

void CollisionListener::EndContact(b2Contact* contact) {

    if (!contact) return;
    
    if (contact->GetFixtureA()->IsSensor() || contact->GetFixtureB()->IsSensor()) {
        handle_sensor_contact(*contact, false);
        return;
    }

    update_car_collision_state(*contact, false);
    processed_collisions.erase(contact);
}

void CollisionListener::PreSolve(b2Contact* contact, const b2Manifold*) {

    if (!contact || contact->GetFixtureA()->IsSensor() || contact->GetFixtureB()->IsSensor())
        return;

    if (processed_collisions.insert(contact).second)
        handle_collision(*contact);
}
