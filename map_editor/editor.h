#pragma once

#include <SDL3/SDL.h>

#include <string>
#include <vector>

#include "editor_ui.h"
#include "map.h"
#include "map_layer.h"
#include "object.h"
#include "scene.h"

class Editor : public thoom::Object {
 public:
  explicit Editor(const std::vector<std::string>& args);
  ~Editor() override;

  void step() override;

  static thoom::Object* create_object(thoom::Scene* scene,
                                      const std::string& options);

 private:
  thoom::MapLayer* map_layer() const;
  void draw_status() const;
  void handle_placement();
  void cycle_layer();

  void set_camera(int x, int y);
  void draw_view();
  void render_objects();
  void draw_billboard(const thoom::ObjectRequest& object);
  void draw_object(const thoom::ObjectRequest& object);

  std::string map_path;
  thoom::Map* map = nullptr;
  EditorUI ui;
  EditorSelectedLayer layer = BACKGROUND;
  bool objects_dirty = true;

  SDL_Texture* view_texture = nullptr;
  SDL_Texture* objects_texture = nullptr;
  SDL_FRect viewport;
  SDL_FRect world_rect;
};
