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
  int ant_kills = thoom::save.geti(FOE_BUG_OBJ STATS);
  int drone_kills = thoom::save.geti(FOE_BAT_OBJ STATS);
  int tank_kills = thoom::save.geti(FOE_FROG_OBJ STATS);
  int agent_kills = thoom::save.geti(FOE_PORCUPINE_OBJ STATS);
  int spitter_time = thoom::save.geti(FOE_SPITTER_OBJ STATS) / 1000;

  this->tex_id =
      RENDER_CREDITS + thoom::credits_args(ant_kills, drone_kills, tank_kills,
                                           agent_kills, spitter_time);
  this->texture = thoom::load_bmp_texture(tex_id);

  this->src_rect = this->dst_rect =
      SDL_FRect{0, 0, THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT};

  thoom::game->corner_x = 0;
  thoom::game->corner_y = 0;
}

void Credits::step() {
  thoom::game->push_sprite(this->tex_id, this->texture, &this->src_rect,
                           &this->dst_rect, 0);

  if (thoom::local_controller.is_hit(thoom::Button::SELECT)) {
    thoom::game->map = "maps/init";
    thoom::save.clear();
  }
}