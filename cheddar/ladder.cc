#include "ladder.h"

#include "audio_playback.h"
#include "game.h"
#include "mouse.h"
#include "save_data.h"

Ladder::Ladder(int x, int y, std::string next_map, int which_coord) {
  this->x = (float)x;
  this->y = (float)y;

  this->which_coord = which_coord;

  this->next_map = next_map;
}

Ladder::~Ladder() {
  // pass
}

void Ladder::step() {
  Mouse* mouse = Mouse::closest_mouse(this->x, this->y, 8, true);

  if (mouse != nullptr) {
    game->map = this->next_map;
    save.puti(MOUSE_SPAWN_AT, this->which_coord);
    play_audio("sfx/ladder.wav", 1.0f, this->x, this->y, false);
  }
}
