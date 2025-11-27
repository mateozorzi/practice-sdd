#ifndef GAME_PHASE_H
#define GAME_PHASE_H

class Game;

class GamePhase {
protected:
    Game& game;
    int it;
    const int FPS_CLIENT;

public:
    GamePhase(Game& game, int starting_it);
    bool run();
    virtual void handle_game_events() = 0;
    virtual bool update_game_state() = 0;
    virtual void show_game() = 0;
    virtual ~GamePhase() {}
};


class WaitingForGamePhase: public GamePhase {
public:
    WaitingForGamePhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game() override;
};


class UpgradePhase: public GamePhase {
public:
    UpgradePhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game() override;
};

class CountdownPhase: public GamePhase {
public:
    CountdownPhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game() override;
};

class RacePhase: public GamePhase {
public:
    RacePhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game() override;
};


class LeaderboardPhase: public GamePhase {
public:
    LeaderboardPhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game() override;
};

class GameFinishedPhase: public GamePhase {
public:
    GameFinishedPhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game() override;
};

class WinnerPhase: public GamePhase {
public:
    WinnerPhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game() override;
};


#endif
