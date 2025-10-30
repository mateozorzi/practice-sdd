#include "lobby_interface.h"

static int fake_argc = 0;
static char *fake_argv[] = { nullptr };

LobbyInterface::LobbyInterface() : app(fake_argc, fake_argv) {}