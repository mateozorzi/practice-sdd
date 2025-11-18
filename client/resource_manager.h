#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

class ResourceManager {
private:
    SDL2pp::Renderer& renderer;
    std::unordered_map<std::string, SDL2pp::Texture> textures;
    std::unordered_map<std::string, SDL2pp::Chunk> sounds;
    std::unordered_map<std::string, SDL2pp::Font> fonts;

    SDL2pp::Font& get_font(const std::string& path, int size);

public:
    explicit ResourceManager(SDL2pp::Renderer& renderer);

    SDL2pp::Texture& get_texture(const std::string& path);
    SDL2pp::Chunk& get_sound(const std::string& path);
    SDL2pp::Texture& get_text(const std::string& text, const std::string& font_path, int size, const SDL2pp::Color& color);
};


#endif
