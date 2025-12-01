#ifndef LEADERBOARD_INFO_H
#define LEADERBOARD_INFO_H

#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "../common/client_event_dto.h"
#include "../common/leaderboard_dto.h"

#include "camera.h"
#include "resource_manager.h"

class LeaderboardInfo {
private:
    SDL2pp::Renderer& renderer;
    SDL2pp::Mixer& mixer;
    ResourceManager& resources;
    std::string font_path;

    bool isOpen;
    int window_width;
    int window_height;

public:
    LeaderboardInfo(SDL2pp::Renderer& renderer, SDL2pp::Mixer& mixer, ResourceManager& resources,
                    int window_width, int window_height,
                    const std::string& font_path =
                            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    void render(const LeaderboardDTO& leaderboard, const std::string& title,
                const std::string& map_name, const Camera& camera);


    void fill_leaderboard(const LeaderboardDTO& leaderboard, const SDL2pp::Rect& rect,
                          std::string title);


    void fill_winner(const LeaderboardDTO& leaderboard, const SDL2pp::Rect& bottom_rect);
    bool is_leaderboard_open();
    void open_leaderboard();
    void close_leaderboard();
};


#endif
