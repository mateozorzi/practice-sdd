#include "power_up.h"

#include <cstdint>
#include <random>
#include <utility>

#include "../common/constants.h"

#include "city_utils.h"
#include "player.h"
#include "server_constants.h"


PowerUpTypeGenerator make_random_power_up_type_generator() {
    return [engine = std::mt19937(std::random_device{}()),
            distribution = std::uniform_int_distribution<int>(0, 2)]() mutable {
        return static_cast<PowerUpType>(distribution(engine));
    };
}


PowerUp::PowerUp(const PowerUpType type, const float x, const float y, b2Body* body):
        type(type), x(x), y(y), body(body) {}

PowerUpType PowerUp::get_type() const { return type; }

float PowerUp::get_x() const { return x; }

float PowerUp::get_y() const { return y; }

bool PowerUp::is_active() const { return active; }

const b2Body* PowerUp::get_body() const { return body; }

b2Body* PowerUp::get_body() { return body; }

uint32_t PowerUp::get_collected_by() const { return collected_by; }

void PowerUp::collect(const double respawn_cooldown, const uint32_t player_id) {
    active = false;
    collected_by = player_id;
    respawn_remaining = respawn_cooldown;
}

bool PowerUp::advance(const double elapsed_seconds) {

    if (active)
        return false;

    respawn_remaining -= elapsed_seconds;
    return respawn_remaining <= EFFECT_TIME_EPSILON;
}

void PowerUp::respawn(const PowerUpType new_type) {
    type = new_type;
    active = true;
    collected_by = 0;
    respawn_remaining = 0.0;
}


PowerUpManager::PowerUpManager(b2World& box2d_world, const ServerConfig& server_config,
                               PowerUpTypeGenerator type_generator):
        box2d_world(box2d_world),
        server_config(server_config),
        type_generator(std::move(type_generator)) {}

PowerUpManager::~PowerUpManager() { clear(); }

b2Body* PowerUpManager::create_sensor(const float x, const float y, const size_t index) {

    const auto pixels_per_meter = static_cast<float>(server_config.world.pixels_per_meter);

    // Kinematic, not static: CityUtils::cleanup destroys every static body when a city is reloaded,
    // and these bodies are owned by the manager
    b2BodyDef box2d_body_definition;
    box2d_body_definition.type = b2_kinematicBody;
    box2d_body_definition.position.Set(x / pixels_per_meter, y / pixels_per_meter);

    b2Body* box2d_body = box2d_world.CreateBody(&box2d_body_definition);

    const float half_size = static_cast<float>(POWER_UP_HALF_SIZE_PIXELS) / pixels_per_meter;
    b2PolygonShape box2d_polygon_shape;
    box2d_polygon_shape.SetAsBox(half_size, half_size);

    b2FixtureDef box2d_fixture_definition;
    box2d_fixture_definition.shape = &box2d_polygon_shape;
    box2d_fixture_definition.isSensor = true;
    box2d_fixture_definition.userData.pointer = static_cast<uintptr_t>(index) << SENSOR_DATA_SHIFT |
                                                static_cast<uintptr_t>(SensorType::POWER_UP);
    box2d_fixture_definition.filter.categoryBits = CATEGORY_SENSOR;
    box2d_fixture_definition.filter.maskBits = MASK_SENSOR;

    box2d_body->CreateFixture(&box2d_fixture_definition);

    return box2d_body;
}

void PowerUpManager::load(const std::vector<CheckpointDTO>& checkpoints) {

    clear();

    for (size_t first = 0; first + 1 < checkpoints.size(); first += 2) {

        const CheckpointDTO& p = checkpoints[first];
        const CheckpointDTO& q = checkpoints[first + 1];
        const float x = (static_cast<float>(p.x) + static_cast<float>(q.x)) / 2.0f;
        const float y = (static_cast<float>(p.y) + static_cast<float>(q.y)) / 2.0f;

        b2Body* body = create_sensor(x, y, power_ups.size());
        power_ups.emplace_back(type_generator(), x, y, body);
    }
}

bool PowerUpManager::try_collect(const size_t index, Player& player) {

    if (index >= power_ups.size())
        return false;

    PowerUp& power_up = power_ups[index];
    if (!power_up.is_active() || player.is_dead())
        return false;

    const PowerUpConfig& power_up_config = server_config.game.power_ups;
    power_up.collect(power_up_config.respawn_cooldown, player.get_id());
    player.apply_power_up(power_up.get_type(), power_up_config);

    return true;
}

void PowerUpManager::advance(const double elapsed_seconds) {

    // The type is drawn again every time a power-up appears, not when it is collected
    for (auto& power_up: power_ups) {
        if (power_up.advance(elapsed_seconds))
            power_up.respawn(type_generator());
    }
}

void PowerUpManager::clear() {

    for (auto& power_up: power_ups) box2d_world.DestroyBody(power_up.get_body());

    power_ups.clear();
}

const std::vector<PowerUp>& PowerUpManager::get_power_ups() const { return power_ups; }
