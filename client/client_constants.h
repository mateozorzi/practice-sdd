#ifndef CLIENT_CONSTANTS_H
#define CLIENT_CONSTANTS_H

#include <cstdint>

// Screen dimensions
constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;

// World dimensions
constexpr int WORLD_WIDTH = 4640;
constexpr int WORLD_HEIGHT = 4672;

// Camera view dimensions
constexpr int CAMERA_VIEW_WIDTH = 512;
constexpr int CAMERA_VIEW_HEIGHT = 288;

// Minimap dimensions
constexpr int MINIMAP_SCALE_DIVISOR = 5;  // Screen size / MINIMAP_SCALE_DIVISOR
constexpr int MINIMAP_WIDTH = SCREEN_WIDTH / MINIMAP_SCALE_DIVISOR;    // 384
constexpr int MINIMAP_HEIGHT = SCREEN_HEIGHT / MINIMAP_SCALE_DIVISOR;  // 216
constexpr int MINIMAP_MARGIN = 20;
constexpr int MINIMAP_BORDER_THICKNESS = 2;
constexpr int MINIMAP_PLAYER_SIZE = 10;
constexpr int MINIMAP_PLAYER_OFFSET = 2;

// Health bar dimensions
constexpr int HEALTH_BAR_WIDTH = 400;
constexpr int HEALTH_BAR_HEIGHT = 20;
constexpr int HEALTH_BAR_X = 20;
constexpr int HEALTH_BAR_Y = SCREEN_HEIGHT - HEALTH_BAR_HEIGHT - HEALTH_BAR_X;

// Car rendering
constexpr int CAR_SPRITE_SIZE = 50;
constexpr int CAR_RENDER_SIZE = 100;
constexpr int CAR_RENDER_OFFSET = 50;  // CAR_RENDER_SIZE / 2
constexpr int DEFAULT_SPRITE_X = 128;
constexpr int DEFAULT_SPRITE_Y = 32;
constexpr int DEFAULT_SPRITE_W = 32;
constexpr int DEFAULT_SPRITE_H = 32;

// NPC rendering
constexpr int NPC_SPRITE_X = 0;
constexpr int NPC_SPRITE_Y = 0;
constexpr int NPC_SPRITE_SIZE = 50;
constexpr int NPC_ANGLE_OFFSET = 90;

// Arrow hint
constexpr int ARROW_WIDTH = 60;
constexpr int ARROW_HEIGHT = 60;
constexpr int ARROW_VERTICAL_OFFSET = 140;

// Stats box
constexpr int STATS_BOX_WIDTH_RATIO = 4;  // 4/5 of screen
constexpr int STATS_BOX_HEIGHT_RATIO = 4;
constexpr int STATS_BOX_DIVISOR = 5;
constexpr int STATS_MARGIN = 20;
constexpr int STATS_FONT_SIZE_DIVISOR = 20;
constexpr int STATS_SPACING_OFFSET = 10;

// Colors (RGBA)
constexpr uint8_t COLOR_BLACK_R = 0;
constexpr uint8_t COLOR_BLACK_G = 0;
constexpr uint8_t COLOR_BLACK_B = 0;
constexpr uint8_t COLOR_BLACK_A = 255;

constexpr uint8_t COLOR_RED_R = 255;
constexpr uint8_t COLOR_RED_G = 0;
constexpr uint8_t COLOR_RED_B = 0;
constexpr uint8_t COLOR_RED_A = 255;
constexpr uint8_t COLOR_RED_ALPHA = 200;

constexpr uint8_t COLOR_YELLOW_R = 255;
constexpr uint8_t COLOR_YELLOW_G = 255;
constexpr uint8_t COLOR_YELLOW_B = 0;
constexpr uint8_t COLOR_YELLOW_A = 255;
constexpr uint8_t COLOR_YELLOW_ALPHA = 200;

constexpr uint8_t COLOR_WHITE_R = 255;
constexpr uint8_t COLOR_WHITE_G = 255;
constexpr uint8_t COLOR_WHITE_B = 255;
constexpr uint8_t COLOR_WHITE_A = 255;

constexpr uint8_t COLOR_GOLD_R = 255;
constexpr uint8_t COLOR_GOLD_G = 215;
constexpr uint8_t COLOR_GOLD_B = 0;
constexpr uint8_t COLOR_GOLD_A = 255;

constexpr uint8_t COLOR_BLACK_ALPHA = 100;

// Rendering delays
constexpr uint32_t RENDER_DELAY_MS = 10;

// Angle conversion
constexpr double ANGLE_OFFSET_DEGREES = 90.0;
constexpr double FULL_CIRCLE_DEGREES = 360.0;

// Text rendering positions
constexpr int TEXT_WAITING_X = 700;
constexpr int TEXT_WAITING_Y = 500;
constexpr int TEXT_WAITING_SIZE = 48;

constexpr int TEXT_COUNTDOWN_X = 600;
constexpr int TEXT_COUNTDOWN_Y = 500;
constexpr int TEXT_COUNTDOWN_SIZE = 48;

constexpr int TEXT_LEADERBOARD_X = 850;
constexpr int TEXT_LEADERBOARD_Y = 100;
constexpr int TEXT_LEADERBOARD_SIZE = 64;

// Paths
constexpr const char* FONT_PATH = "client/assets/fonts/UrbanHaze.ttf";
constexpr const char* CARS_SPRITE_PATH = "client/assets/sprites/cars-no-background.png";
constexpr const char* ARROW_SPRITE_PATH = "client/assets/sprites/arrow.png";
constexpr const char* MAP_BACKGROUND_PREFIX =
        "client/assets/sprites/Game Boy _ GBC - Grand Theft Auto - Backgrounds - ";
constexpr const char* MAP_BACKGROUND_SUFFIX = ".png";
constexpr const char* DECORATIONS_PREFIX = "server/world/decorations_";
constexpr const char* BRIDGES_PREFIX = "server/world/bridges_";
constexpr const char* JSON_SUFFIX = ".json";

// Default city
constexpr const char* DEFAULT_CITY = "Liberty City";

// Upgrade Shop
constexpr int SHOP_FONT_SIZE = 64;
constexpr int SHOP_SIZE_RATIO_NUM = 3;
constexpr int SHOP_SIZE_RATIO_DEN = 4;  // Shop is 3/4 of window
constexpr int SHOP_BORDER_THICKNESS = 5;
constexpr int SHOP_TITLE_Y_OFFSET = 15;

constexpr int SHOP_BUTTON_WIDTH_RATIO_NUM = 3;
constexpr int SHOP_BUTTON_WIDTH_RATIO_DEN = 5;   // 3/5 of shop width
constexpr int SHOP_BUTTON_HEIGHT_DIVISOR = 8;    // 1/8 of shop height
constexpr int SHOP_BUTTON_SPACING_DIVISOR = 20;  // 1/20 of shop height
constexpr int SHOP_BUTTON_START_Y_DIVISOR = 4;   // 1/4 of shop height
constexpr int SHOP_BUTTON_BORDER_THICKNESS = 2;

// Upgrade Shop Colors (RGBA)
constexpr uint8_t COLOR_SHOP_BG_R = 40;
constexpr uint8_t COLOR_SHOP_BG_G = 40;
constexpr uint8_t COLOR_SHOP_BG_B = 40;
constexpr uint8_t COLOR_SHOP_BG_A = 230;

constexpr uint8_t COLOR_SHOP_BUTTON_R = 80;
constexpr uint8_t COLOR_SHOP_BUTTON_G = 80;
constexpr uint8_t COLOR_SHOP_BUTTON_B = 80;
constexpr uint8_t COLOR_SHOP_BUTTON_A = 255;

constexpr uint8_t COLOR_SHOP_BUTTON_HOVER_R = 120;
constexpr uint8_t COLOR_SHOP_BUTTON_HOVER_G = 120;
constexpr uint8_t COLOR_SHOP_BUTTON_HOVER_B = 120;
constexpr uint8_t COLOR_SHOP_BUTTON_HOVER_A = 255;

// Upgrade Shop Text
constexpr const char* SHOP_TITLE_TEXT = "UPGRADE SHOP";
constexpr const char* SHOP_SECONDS_TEXT = " seconds";

#endif
