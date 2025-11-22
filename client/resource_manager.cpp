#include "resource_manager.h"

#include <memory>

ResourceManager::ResourceManager(SDL2pp::Renderer& renderer): renderer(renderer) {}

SDL2pp::Texture& ResourceManager::get_texture(const std::string& path) {
    const auto it = textures.find(path);
    if (it != textures.end()) {
        return it->second;
    }

    const SDL2pp::Surface surface(path);
    const auto result = textures.emplace(path, SDL2pp::Texture(renderer, surface));
    return result.first->second;
}

SDL2pp::Chunk& ResourceManager::get_sound(const std::string& path) {
    const auto it = sounds.find(path);
    if (it != sounds.end()) {
        return it->second;
    }

    const auto result = sounds.emplace(path, SDL2pp::Chunk(path));
    return result.first->second;
}

SDL2pp::Font& ResourceManager::get_font(const std::string& path, int size) {
    std::string key = path + std::to_string(size);

    const auto it = fonts.find(key);
    if (it != fonts.end()) {
        return it->second;
    }

    const auto result = fonts.emplace(key, SDL2pp::Font(path, size));
    return result.first->second;
}

SDL2pp::Texture& ResourceManager::get_text(const std::string& text, const std::string& font_path,
                                           int size, const SDL2pp::Color& color) {
    std::string key = text + font_path + std::to_string(size) + std::to_string(color.GetRed()) +
                      std::to_string(color.GetGreen()) + std::to_string(color.GetBlue()) +
                      std::to_string(color.GetAlpha());

    const auto it = textures.find(key);
    if (it != textures.end()) {
        return it->second;
    }

    SDL2pp::Font& font = get_font(font_path, size);
    const SDL2pp::Surface surface = font.RenderText_Blended(text, color);
    const auto result = textures.emplace(key, SDL2pp::Texture(renderer, surface));
    return result.first->second;
}
