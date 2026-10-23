#ifndef POWER_UP_VIEW_H
#define POWER_UP_VIEW_H


#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../common/power_up_type.h"

#include "game_state.h"


// What the screen shows of the power-ups, without SDL: the render only draws these lists.

// Power-up on the track, centered in (x, y) world pixels and drawn as a square of `size`
struct PowerUpSprite {
    uint8_t id;
    std::string sprite;
    int x;
    int y;
    int size;
};

// Badge of an effect of the own player: the power-up sprite and the seconds left, in screen pixels
struct EffectBadge {
    PowerUpType type;
    std::string sprite;
    std::string text;
    int x;
    int y;
    int w;
    int h;
};

// Sprite of each power-up type, nullopt for a type the client does not know
std::optional<std::string> power_up_sprite(PowerUpType type);

// Active power-ups with a known type
std::vector<PowerUpSprite> power_ups_to_draw(const LocalInfo& state);

// One badge per own effect with duration (Nitro or Shield), side by side
std::vector<EffectBadge> effect_badges(const LocalInfo& state);


#endif
