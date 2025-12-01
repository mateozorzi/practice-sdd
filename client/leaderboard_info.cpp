#include "leaderboard_info.h"
#include "client_constants.h"

// Constantes para el leaderboard
constexpr int LEADERBOARD_TITLE_FONT_SIZE = 64;
constexpr int LEADERBOARD_HEADER_FONT_SIZE = 48;
constexpr int LEADERBOARD_ENTRY_FONT_SIZE = 32;
constexpr int LEADERBOARD_TITLE_SPACING = 40;
constexpr int LEADERBOARD_ENTRY_SPACING = 25;
constexpr int LEADERBOARD_HEADER_ENTRY_SPACING = 50;
constexpr int LEADERBOARD_SEPARATOR_SPACING = 30;

// Anchos de columnas
constexpr int COLUMN_POS_WIDTH = 100;
constexpr int COLUMN_NAME_WIDTH = 200;
constexpr int COLUMN_TIME_WIDTH = 100;
constexpr int COLUMN_POINTS_WIDTH = 100;


LeaderboardInfo::LeaderboardInfo(SDL2pp::Renderer& renderer, SDL2pp::Mixer& mixer,
                                 ResourceManager& resources, int window_width,
                                 int window_height,
                                 const std::string& font_path) :
        renderer(renderer),
        mixer(mixer),
        resources(resources),
        font_path(font_path),
        isOpen(false),
        window_width(window_width),
        window_height(window_height) {}

void LeaderboardInfo::render(const LeaderboardDTO& leaderboard, const std::string& title, const std::string& map_name,
                             const Camera& camera) {
        SDL_BlendMode prev_blend = renderer.GetDrawBlendMode();
        SDL_Color prev_color = renderer.GetDrawColor();
    SDL2pp::Texture& map = resources.get_texture(map_name);
    SDL2pp::Rect view_rect = camera.get_view_rect();

    renderer.Copy(map, view_rect,
                  SDL2pp::Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

    int rect_w = SCREEN_WIDTH * 3 / 4;
    int rect_h = SCREEN_HEIGHT * 3 / 4;
    int rect_x = (SCREEN_WIDTH - rect_w) / 2;
    int rect_y = (SCREEN_HEIGHT - rect_h) / 2;

    SDL2pp::Rect board_rect(rect_x, rect_y, rect_w, rect_h);

    SDL2pp::Color bg_color(30, 30, 30, 220);   // Fondo negro semi-transparente
    SDL2pp::Color border_color(255, 215, 0, 255); // Dorado
    int border = 5;

    // Fondo del cuadro
    renderer.SetDrawBlendMode(SDL_BLENDMODE_BLEND);
    renderer.SetDrawColor(bg_color);
    renderer.FillRect(board_rect);

    // Bordes
    renderer.SetDrawBlendMode(SDL_BLENDMODE_NONE);
    renderer.SetDrawColor(border_color);
    renderer.FillRect(SDL2pp::Rect(rect_x, rect_y, rect_w, border));               // Top
    renderer.FillRect(SDL2pp::Rect(rect_x, rect_y + rect_h - border, rect_w, border)); // Bottom
    renderer.FillRect(SDL2pp::Rect(rect_x, rect_y, border, rect_h));               // Left
    renderer.FillRect(SDL2pp::Rect(rect_x + rect_w - border, rect_y, border, rect_h)); // Right

    int content_x = rect_x + 30;
    (void) content_x;
    int content_y = rect_y + 30;

    SDL2pp::Color title_color(COLOR_GOLD_R, COLOR_GOLD_G, COLOR_GOLD_B, COLOR_GOLD_A);
    SDL2pp::Texture& title_texture =
        resources.get_text(title, FONT_PATH, LEADERBOARD_TITLE_FONT_SIZE, title_color);

    int title_x = rect_x + (rect_w - title_texture.GetWidth()) / 2;
    renderer.Copy(title_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(title_x, content_y,
                               title_texture.GetWidth(),
                               title_texture.GetHeight()));

    int header_y = content_y + title_texture.GetHeight() + 40;


    SDL2pp::Color header_color(255, 255, 255, 255);

    std::vector<std::string> headers = {"POS", "NAME", "TIME", "PTS"};
    std::array<int, 4> column_widths =
        { COLUMN_POS_WIDTH, COLUMN_NAME_WIDTH, COLUMN_TIME_WIDTH, COLUMN_POINTS_WIDTH };

    int table_start_x = rect_x + (rect_w - (
        COLUMN_POS_WIDTH + COLUMN_NAME_WIDTH + COLUMN_TIME_WIDTH + COLUMN_POINTS_WIDTH
    )) / 2;

    int current_x = table_start_x;

    for (size_t i = 0; i < headers.size(); ++i) {
        SDL2pp::Texture& header_texture = resources.get_text(
            headers[i], FONT_PATH, LEADERBOARD_HEADER_FONT_SIZE, header_color);

        renderer.Copy(header_texture, SDL2pp::NullOpt,
            SDL2pp::Rect(current_x + 10, header_y,
                         header_texture.GetWidth(),
                         header_texture.GetHeight()));

        current_x += column_widths[i];
    }


    SDL2pp::Color entry_color(255, 255, 255, 255);
    int entry_y = header_y + 55;

    for (const auto& entry : leaderboard.players) {
        current_x = table_start_x;

        // POS
        std::string pos_text = std::to_string(entry.position);
        SDL2pp::Texture& pos_texture = resources.get_text(
            pos_text, FONT_PATH, LEADERBOARD_ENTRY_FONT_SIZE, entry_color);

        renderer.Copy(pos_texture, SDL2pp::NullOpt,
                      SDL2pp::Rect(current_x + 10, entry_y,
                                   pos_texture.GetWidth(), pos_texture.GetHeight()));
        current_x += COLUMN_POS_WIDTH;

        // NAME
        SDL2pp::Texture& name_texture = resources.get_text(
            entry.username, FONT_PATH, LEADERBOARD_ENTRY_FONT_SIZE, entry_color);
        renderer.Copy(name_texture, SDL2pp::NullOpt,
                      SDL2pp::Rect(current_x + 10, entry_y,
                                   name_texture.GetWidth(), name_texture.GetHeight()));
        current_x += COLUMN_NAME_WIDTH;

        // TIME
        std::string time_text = entry.points == 0
            ? "DSQ"
            : std::to_string(entry.total_time) + " seg";

        SDL2pp::Texture& time_texture = resources.get_text(
            time_text, FONT_PATH, LEADERBOARD_ENTRY_FONT_SIZE, entry_color);
        renderer.Copy(time_texture, SDL2pp::NullOpt,
                      SDL2pp::Rect(current_x + 10, entry_y,
                                   time_texture.GetWidth(), time_texture.GetHeight()));
        current_x += COLUMN_TIME_WIDTH;

        // POINTS
        SDL2pp::Texture& points_texture = resources.get_text(
            std::to_string(entry.points), FONT_PATH,
            LEADERBOARD_ENTRY_FONT_SIZE, entry_color);

        renderer.Copy(points_texture, SDL2pp::NullOpt,
                      SDL2pp::Rect(current_x + 10, entry_y,
                                   points_texture.GetWidth(), points_texture.GetHeight()));

        entry_y += 30;
    }
    renderer.SetDrawBlendMode(prev_blend);
        renderer.SetDrawColor(prev_color);
}

void LeaderboardInfo::open_leaderboard() { isOpen = true; }
void LeaderboardInfo::close_leaderboard() { isOpen = false; }
bool LeaderboardInfo::is_leaderboard_open() { return isOpen; }