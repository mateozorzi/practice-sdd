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
    void run();
    void change_phase(int last_it);
    virtual void handle_game_events() = 0;
    virtual bool update_game_state() = 0;
    virtual void show_game(const int& it) = 0;
    virtual ~GamePhase() {}
};

/*
class WaitingForGamePhase : public GamePhase {
public:
    WaitingForGamePhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game(const int& it) override;
};
*/

class UpgradePhase : public GamePhase {
public:
    UpgradePhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game(const int& it) override;
};

class RacePhase : public GamePhase {
public:
    RacePhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game(const int& it) override;
};

/*
class StatsPhase : public GamePhase {
public:
    StatsPhase(Game& game, int starting_it);
    void handle_game_events() override;
    bool update_game_state() override;
    void show_game(const int& it) override;
};
*/

#endif
