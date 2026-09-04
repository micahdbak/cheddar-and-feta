#include "feta.h"
#include "item.h"

bool Mouse::check_collision(float x, float y) {
  return thoom::game->point_in_collider(x + 2.0f, y) ||
         thoom::game->point_in_collider(x - 2.0f, y) ||
         thoom::game->point_in_collider(x, y - 2.0f);
}

std::string Mouse::encode_items(
    const std::vector<thoom::Game::HudItem>& items) {
  std::string items_s = "";
  for (int i = 0; i < items.size(); i++) {
    if (items[i].count <= 0) continue;

    char item[256];
    snprintf(item, sizeof(item), "%s*%d", items[i].item_id.c_str(),
             items[i].count);
    items_s += item;

    if (i + 1 < items.size()) {
      items_s += ",";
    }
  }
  return items_s;
}

std::vector<thoom::Game::HudItem> Mouse::read_items(
    const std::string& items_s) {
  std::vector<thoom::Game::HudItem> ret;

  if (!items_s.empty()) {
    const char* arr = items_s.c_str();
    int i = 0;
    do {
      char item_id[256];
      int count = 1;
      if (2 != sscanf(arr, "%255[^*] * %d", item_id, &count)) FATAL_ERROR

      // validate item
      if (item_info.find(item_id) == item_info.end() || count <= 0) continue;

      thoom::Game::HudItem item{item_id, count};
      ret.push_back(item);

      while (*arr != '\0' && *arr != ',') arr++;

      if (*arr == ',') arr++;

      if (*arr == '\0') break;
    } while (i++ < 100);
  }

  return ret;
}

std::string Mouse::audio_msg(const char* wav_path, float gain, int x, int y) {
  char buff[256];
  snprintf(buff, sizeof(buff), "%s %d,%d,%d\n", wav_path, (int)(10.0f * gain),
           (int)x, (int)y);
  return std::string(buff);
}