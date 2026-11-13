#include "cheddar.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "game_hud.h"
#include "hurtbox.h"
#include "items/item.h"
#include "items/persister.h"
#include "items/save.h"
#include "mouse.h"
#include "object/entity.h"
#include "peer/peer.h"
#include "remote_feta.h"
#include "renderer/renderer.h"
#include "save/save_data.h"
#include "scene/scene.h"

#ifdef ONSCREEN_DEBUG
#include "foes/foe.h"
#endif

Cheddar::Cheddar(const thoom::Entity::Context& context,
                 std::vector<Mouse::SpawnCoord>& coordinates)
    : Mouse(context, "sprites/cheddar.bmp", false, CHEDDAR_OBJ) {
  if (cheddar != nullptr) {
    // TODO(micahdbak): error
    std::exit(1);
  }
  cheddar = this;

  std::string items_s = thoom::save.value(this->save_id + MOUSE_ITEMS);
  if (!items_s.empty()) {
    this->items = Mouse::read_items(items_s, require_item);
  } else {
    this->items.push_back(GameHud::Item{ITEM_SAVE, 1});
  }

  int feta_x = 0, feta_y = 0, feta_animation = 0;

  // if loading a save, read the location from the save file
  if (thoom::save.geti(LOAD_SAVE) && thoom::save.has(this->save_id + MOUSE_X)) {
    this->sprite->set_animation(
        thoom::save.geti(this->save_id + MOUSE_ANIMATION));
    this->place(thoom::save.getf(this->save_id + MOUSE_X),
                thoom::save.getf(this->save_id + MOUSE_Y));

    // don't set remote feta options; it loads from the save file as well
  } else {
    // get which coordinate to spawn at
    int coord = thoom::save.geti(MOUSE_SPAWN_AT);
    thoom::save.puti(MOUSE_SPAWN_AT, 0);  // unset

    // validate
    if (coordinates.empty() || coord < 0 || coord >= coordinates.size()) {
      // TODO(micahdbak): error
      std::exit(1);
    }

    this->sprite->set_animation(coordinates[coord].animation);
    this->place(coordinates[coord].x, coordinates[coord].y);

    feta_x = coordinates[coord].x;
    feta_y = coordinates[coord].y;
    feta_animation = coordinates[coord].animation;
  }

  // make remote feta
  char feta_options[256];
  snprintf(feta_options, sizeof(feta_options), "%d,%d,%d", feta_x, feta_y,
           feta_animation);
  thoom::Scene::instance->create_entity(
      FETA_OBJ, std::string(feta_options),
      {thoom::Entity::Ownership::kFixed, this->owner_pid});

  thoom::Scene::instance->create_object(ITEM_PERSISTER_OBJ, "");
  new HurtBox(*this, -8, -8, 16, 12);
}

Cheddar::~Cheddar() { cheddar = nullptr; }

void Cheddar::render() {
  GameHud::draw_items(this->items, this->sel_item, this->health,
                      this->max_health);

#ifdef ONSCREEN_DEBUG
  for (int ty = this->tile_y - 12; ty < this->tile_y + 13; ty++) {
    for (int tx = this->tile_x - 12; tx < this->tile_x + 13; tx++) {
      foe_debug_tile(tx, ty);
    }
  }
#endif

  thoom::Renderer::instance->set_view(this->x, this->y);
  Mouse::render();
}

void Cheddar::signal_down() {
  // signal implies that the remote feta is down; check if we are down too
  if (this->is_down && this->is_busy == DOWNED && remote_feta != nullptr &&
      remote_feta->is_down) {
    thoom::Scene::instance->request_map("maps/dead", true);
  }
}

void Cheddar::on_down() {
  uint64_t peer_state = thoom::Peer::instance->get_state();

  // game over if the remote feta is down or disconnected
  if ((remote_feta != nullptr && remote_feta->is_down) ||
      peer_state == thoom::Peer::kStateNoConnection ||
      peer_state == thoom::Peer::kStateWaitingForPeer) {
    thoom::Scene::instance->request_map("maps/dead", true);
  }
}

thoom::Entity* Cheddar::create_entity(const thoom::Entity::Context& context,
                                      const std::string& options) {
  const char* arr = options.c_str();
  std::vector<Mouse::SpawnCoord> coordinates;

  int i = 0;
  do {
    int x = 0, y = 0, animation = 0;
    if (3 != sscanf(arr, "%d,%d,%d", &x, &y, &animation)) break;

    Mouse::SpawnCoord coord;
    coord.x = (float)x;
    coord.y = (float)y;
    coord.animation = animation;
    coordinates.push_back(coord);

    while (*arr != '\0' && *arr != ' ') arr++;
    if (*arr == '\0') break;
    while (*++arr == ' ') arr++;
    if (*arr == '\0') break;
  } while (++i < 10);  // max ten coords, incase something really breaks

  return new Cheddar(context, coordinates);
}
