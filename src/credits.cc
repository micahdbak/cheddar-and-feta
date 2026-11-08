#include "credits.h"

#include "foes/bat.h"
#include "foes/bug.h"
#include "foes/frog.h"
#include "foes/porcupine.h"
#include "foes/spitter/spitter.h"
#include "input/controller.h"
#include "input/input.h"
#include "object.h"
#include "render_functions.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "texture/texture.h"
#include "window/window.h"

Credits::Credits() {
  int ant_kills = thoom::save.geti(FOE_BUG_OBJ STATS);
  int drone_kills = thoom::save.geti(FOE_BAT_OBJ STATS);
  int tank_kills = thoom::save.geti(FOE_FROG_OBJ STATS);
  int agent_kills = thoom::save.geti(FOE_PORCUPINE_OBJ STATS);
  int spitter_time = thoom::save.geti(FOE_SPITTER_OBJ STATS) / 1000;

  this->tex_id =
      RENDER_CREDITS + credits_args(ant_kills, drone_kills, tank_kills,
                                    agent_kills, spitter_time);
  this->texture = thoom::Texture::instance->load(tex_id);

  this->src_rect = this->dst_rect =
      SDL_FRect{0, 0, THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT};
}

void Credits::step() {
  if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
    thoom::Scene::instance->request_map("maps/init");
    thoom::save.clear();
  }
}

void Credits::render() {
  thoom::Renderer::instance->push_sprite(this->tex_id, this->texture,
                                         &this->src_rect, &this->dst_rect, 0);
}

thoom::Object* Credits::create_object(thoom::Scene* scene,
                                      const std::string& options) {
  return new Credits();
}
