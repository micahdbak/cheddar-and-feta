#include "credits.h"

#include "bmp_texture.h"
#include "controller.h"
#include "foes/bat.h"
#include "foes/bug.h"
#include "foes/frog.h"
#include "foes/porcupine.h"
#include "foes/spitter/spitter.h"
#include "game.h"
#include "save_data.h"

Credits::Credits() {
  int ant_kills = save.geti(FOE_BUG_OBJ STATS);
  int drone_kills = save.geti(FOE_BAT_OBJ STATS);
  int tank_kills = save.geti(FOE_FROG_OBJ STATS);
  int agent_kills = save.geti(FOE_PORCUPINE_OBJ STATS);
  int spitter_time = save.geti(FOE_SPITTER_OBJ STATS) / 1000;

  this->tex_id =
      RENDER_CREDITS + credits_args(ant_kills, drone_kills, tank_kills,
                                    agent_kills, spitter_time);
  this->texture = load_bmp_texture(tex_id);

  this->src_rect = this->dst_rect =
      SDL_FRect{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

  game->corner_x = 0;
  game->corner_y = 0;
}

void Credits::step() {
  game->push_sprite(this->tex_id, this->texture, &this->src_rect,
                    &this->dst_rect, 0);

  if (local_controller.is_hit(Button::SELECT)) {
    game->map = "maps/init";
    save.clear();
  }
}