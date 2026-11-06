#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

#include "map.h"
#include "object.h"
#include "scene.h"

enum EditorSelectedLayer { BACKGROUND, FOREGROUND, COLLISION };

enum EditorInputMode { INPUT_TILE, INPUT_SHEET };

class Editor : public thoom::Object {
 public:
  Editor(const std::vector<std::string>& args);
  ~Editor();

  void step() override;

  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  void input_tile();
  void input_sheet();
  void render();
  void render_collision_tile(SDL_Texture* target, int x, int y, int collider);
  void render_objects();

  std::string map_path;
  thoom::Map* map = nullptr;

  EditorSelectedLayer layer = BACKGROUND;
  int sel_x = 0, sel_y = 0;
  int sel_tilesheet = -1, sel_ts_x = 0, sel_ts_y = 0, sel_ts2_x = -1,
      sel_ts2_y = -1;
  int sel_collider = -1;

  bool user_inputting = false;
  EditorInputMode input_mode;
  std::string text;

  SDL_Texture* texture = nullptr;
  SDL_FRect src_rect, dst_rect;
  SDL_Texture* objects = nullptr;

  SDL_Texture *collision_texture = nullptr, *collision_sheet = nullptr;
};
