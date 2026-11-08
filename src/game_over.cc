#include "game_over.h"

#include "constants.h"
#include "input/controller.h"
#include "input/input.h"
#include "object.h"
#include "renderer/font.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"
#include "texture/texture.h"
#include "window/window.h"

GameOver::GameOver() {
  this->ui = thoom::Texture::instance->create_texture(
      THOOM_SCREEN_WIDTH, THOOM_SCREEN_HEIGHT, SDL_PIXELFORMAT_RGBA8888,
      SDL_SCALEMODE_LINEAR);

  // SDL_FRect ui_box = { 112.0f, 176.0f, 96.0f, 32.0f };
  thoom::Renderer::instance->draw_text(this->ui, DEFAULT_FONT,
                                       "(Press [Enter] to try again)", 98, 160,
                                       0, kBackground);
  this->src = this->dst = {0.0f, 0.0f, (float)THOOM_SCREEN_WIDTH,
                           (float)THOOM_SCREEN_HEIGHT};

  thoom::Renderer::instance->reset_view();
}

GameOver::~GameOver() { SDL_DestroyTexture(this->ui); }

void GameOver::step() {
  if (thoom::Input::instance->world.consume_hit(thoom::Button::SELECT)) {
    thoom::Scene::instance->request_map("maps/init");
    thoom::save.clear();
  }
}

void GameOver::render() {
  thoom::Renderer::instance->push_sprite("", this->ui, &this->src, &this->dst,
                                         0);
}

thoom::Object* GameOver::create_object(thoom::Scene* scene,
                                       const std::string& options) {
  return new GameOver();
}
