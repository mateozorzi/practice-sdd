#ifndef SERVER_CONSTANTS_H
#define SERVER_CONSTANTS_H


// Classes names
constexpr std::string_view ACCEPTOR = "ACCEPTOR";
constexpr std::string_view CLIENT_HANDLER = "CLIENT_HANDLER";
constexpr std::string_view SENDER = "SENDER";
constexpr std::string_view RECEIVER = "RECEIVER";
constexpr std::string_view LOBBY = "LOBBY";
constexpr std::string_view GAME = "GAME";
constexpr std::string_view GAME_LOOP = "GAME_LOOP";
constexpr std::string_view GAME_MONITOR = "GAME_MONITOR";
constexpr std::string_view SERVER = "SERVER";
constexpr std::string_view SERVER_MAIN = "SERVER_MAIN";


// Acceptor
constexpr uint32_t FIRST_CLIENT_ID = 1;

// CityUtils
constexpr uint8_t POLYGONS_SIZE = 4;

// ClientHandler
constexpr std::string_view DEFAULT_USERNAME = "Player";

// CollisionListener
constexpr uint8_t SENSOR_TYPE_MASK = 0xFF;
constexpr uint8_t SENSOR_DATA_SHIFT = 8;
constexpr float MIN_COLLISION_SPEED = 1.0f;

constexpr float ANGLE_DOT_NORMALIZATION = 2.0f;
constexpr float ANGLE_FACTOR_LOW_THRESHOLD = 0.5f;
constexpr float ANGLE_FACTOR_MID_THRESHOLD = 0.7f;

constexpr float ANGLE_FACTOR_LOW_VALUE = 0.1f;
constexpr float ANGLE_FACTOR_MID_VALUE = 0.3f;
constexpr float ANGLE_FACTOR_HIGH_VALUE = 1.0f;

constexpr float RELATIVE_SPEED_NORMALIZATION = 10.0f;
constexpr float SPEED_FACTOR_MIN = 0.2f;
constexpr float SPEED_FACTOR_MAX = 1.0f;

constexpr float BASE_DAMAGE = 10.0f;

constexpr float BASE_VELOCITY_REDUCTION = 0.5f;
constexpr float DAMAGE_TO_REDUCTION_SCALE = 0.01f;
constexpr float MAX_VELOCITY_REDUCTION = 0.9f;

constexpr float STATIC_COLLISION_DAMAGE_MULTIPLIER = 2.0f;
constexpr float STATIC_COLLISION_DAMAGE_MAX = 40.0f;
constexpr float STATIC_COLLISION_VELOCITY_REDUCTION = 0.05f;

// GameLoop
constexpr float GAME_STEP = 1.0f / 60.0f;
constexpr int VELOCITY_ITERATIONS = 10;
constexpr int POSITION_ITERATIONS = 6;
constexpr uint8_t INITIAL_RACE_INDEX = 0;

// Main
constexpr uint8_t SUCCESS = 0;
constexpr uint8_t ERROR = 1;
constexpr std::string_view UNKNOWN_EXCEPTION = "Unknown exception";

// Server
constexpr char SERVER_CMD_EXIT = 'q';
constexpr std::string_view SERVER_STARTED = "SERVER STARTED";
constexpr std::string_view SERVER_STOPPED = "SERVER STOPPED";
constexpr std::string_view SERVER_CONFIG_FILEPATH = "/etc/nfs";

// RaceLoader
constexpr uint8_t ID = 0;
constexpr uint8_t ORIENTATION = 1;
constexpr uint8_t X_CENTER = 2;
constexpr uint8_t Y_CENTER = 3;
constexpr uint8_t WIDTH = 4;
constexpr uint8_t HEIGHT = 5;


#endif
