#include "mouse.h"
#include "items/item.h"

Mouse *cheddar = nullptr;
Mouse *feta = nullptr;
bool mice_locked = false;

bool Mouse::check_collision(float x, float y) {
    return game->point_in_collider(x+2.0f, y) ||
        game->point_in_collider(x-2.0f, y) ||
        game->point_in_collider(x, y-2.0f);
}

std::string Mouse::encode_items(const std::vector<Game::HudItem> &items) {
    std::string items_s = "";
    for (int i = 0; i < items.size(); i++) {
        if (items[i].count <= 0)
            continue;

        char item[256];
        snprintf(item, sizeof(item), "%s*%d", items[i].item_id.c_str(), items[i].count);
        items_s += item;

        if (i + 1 < items.size()) {
            items_s += ",";
        }
    }
    return items_s;
}

std::vector<Game::HudItem> Mouse::read_items(const std::string &items_s) {
    std::vector<Game::HudItem> ret;

    if (!items_s.empty()) {
        const char *arr = items_s.c_str();
        int i = 0;
        do {
            char item_id[256];
            int count;
            if (2 != sscanf(arr, "%255[^*] * %d", item_id, &count)) FATAL_ERROR
            item_id[255] = '\0';

            // validate item
            if (item_info.find(item_id) == item_info.end() || count <= 0) FATAL_ERROR

            Game::HudItem item{ item_id, count };
            ret.push_back(item);

            while (*arr != '\0' && *arr != ',')
                arr++;

            if (*arr == ',')
                arr++;

            if (*arr == '\0')
                break;
        } while (i++ < 100);
    }

    return ret;
}

Mouse *Mouse::closest_mouse(float x, float y, float min_distance, bool forced) {
    float distance_cheddar = (cheddar->is_down && !forced) ? FLT_MAX : distance_between_points(x, y, cheddar->x, cheddar->y);
    float distance_feta = (feta->is_down && !forced) ? FLT_MAX : distance_between_points(x, y, feta->x, feta->y);

    if (distance_cheddar < distance_feta) {
        if (min_distance < 0.1f || distance_cheddar < min_distance) {
            return cheddar;
        }
    } else if (min_distance < 0.1f || distance_feta < min_distance) {
        return feta;
    }

    return nullptr; // no close-enough mouse
}