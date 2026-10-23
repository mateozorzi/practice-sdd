#include "power_up_view.h"

#include <string>
#include <vector>

#include "client_constants.h"


namespace {

constexpr uint32_t MS_PER_SECOND = 1000;

// Seconds left rounded up, so the badge shows "1 SEG" until the effect expires. The game font
// draws a lowercase "s" like a digit ("3s" reads as "35"), hence the separate "SEG".
std::string seconds_left(uint32_t remaining_ms) {
    return std::to_string((remaining_ms + MS_PER_SECOND - 1) / MS_PER_SECOND) + " SEG";
}

}  // namespace


std::optional<std::string> power_up_sprite(const PowerUpType type) {

    switch (type) {
        case PowerUpType::Nitro:
            return NITRO_SPRITE_PATH;
        case PowerUpType::Repair:
            return LIFE_SPRITE_PATH;
        case PowerUpType::Shield:
            return SHIELD_SPRITE_PATH;
    }
    // The snapshot can bring a type this client does not know
    return std::nullopt;
}

std::vector<PowerUpSprite> power_ups_to_draw(const LocalInfo& state) {

    std::vector<PowerUpSprite> sprites;
    for (const auto& power_up: state.power_ups) {

        std::optional<std::string> sprite = power_up_sprite(power_up.type);
        if (!power_up.active || !sprite)
            continue;

        sprites.push_back(PowerUpSprite{power_up.id, *sprite, static_cast<int>(power_up.x_position),
                                        static_cast<int>(power_up.y_position), POWER_UP_RENDER_SIZE,
                                        power_up.layer});
    }
    return sprites;
}

std::vector<PowerUpSprite> power_ups_to_draw(const LocalInfo& state, const PowerUpDrawPass pass) {

    std::vector<PowerUpSprite> sprites;
    for (const auto& sprite: power_ups_to_draw(state)) {

        const bool above = sprite.layer == CarLayer::ON_BRIDGE;
        if (above == (pass == PowerUpDrawPass::ABOVE_BRIDGES))
            sprites.push_back(sprite);
    }
    return sprites;
}

std::vector<EffectBadge> effect_badges(const LocalInfo& state) {

    std::vector<EffectBadge> badges;
    for (const auto& effect: state.effects) {

        // The repair is instant: only the effects with duration have a badge
        if (effect.type != PowerUpType::Nitro && effect.type != PowerUpType::Shield)
            continue;

        const int x = EFFECT_BADGE_X +
                      static_cast<int>(badges.size()) * (EFFECT_BADGE_WIDTH + EFFECT_BADGE_SPACING);
        badges.push_back(EffectBadge{effect.type, power_up_sprite(effect.type).value_or(""),
                                     seconds_left(effect.remaining_ms), x, EFFECT_BADGE_Y,
                                     EFFECT_BADGE_WIDTH, EFFECT_BADGE_HEIGHT});
    }
    return badges;
}
