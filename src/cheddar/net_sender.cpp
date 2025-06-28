#include "game.h"
#include "items/item.h"
#include "mouse.h"
#include "net_agent.h"
#include "net_sender.h"

void NetSender::step() {
    if (net_agent->get_state() != NetworkAgent::State::CONNECTED)
        return;

    std::string frame_msg = game->current_map + '\n';

    // view
    char line[1024];
    if (feta != nullptr) {
        snprintf(line, sizeof(line), "%d,%d\n", int(feta->x), int(feta->y));
    } else {
        snprintf(line, sizeof(line), "0,0\n");
    }
    frame_msg += line;

    // hud
    if (feta != nullptr) {
        std::string sel_item_id = feta->sel_item < 0 ? ITEM_NONE : feta->items[feta->sel_item].item_id;
        int sel_item_count = feta->sel_item < 0 ? 0 : feta->items[feta->sel_item].count;
        if (!item_info.contains(sel_item_id)) {
            feta->sel_item = -1;
            sel_item_id = ITEM_NONE; // wtf
        }

        snprintf(line, sizeof(line), "%s,%d,%d,%d,%d\n", sel_item_id.c_str(), sel_item_count, feta->health, feta->max_health, feta->cheese);
    } else {
        snprintf(line, sizeof(line), ITEM_NONE ",0,0,0\n");
    }
    frame_msg += line;

    // sprites
    for (Game::SpriteRender &sprite : game->sprites) {
        if (sprite.texture == nullptr || sprite.dst_rect == nullptr)
            continue;

        SDL_Rect src_rect;
        if (sprite.src_rect != nullptr) {
            src_rect = SDL_Rect{
                int(sprite.src_rect->x),
                int(sprite.src_rect->y),
                int(sprite.src_rect->w),
                int(sprite.src_rect->h)
            };
        } else {
            src_rect = SDL_Rect{
                int(0),
                int(0),
                int(sprite.texture->w),
                int(sprite.texture->h)
            };
        }
        SDL_Rect dst_rect = SDL_Rect{
            int(sprite.dst_rect->x) + game->corner_x,
            int(sprite.dst_rect->y) + game->corner_y,
            int(sprite.dst_rect->w),
            int(sprite.dst_rect->h)
        };

        snprintf(line, sizeof(line), "%s %d,%d,%d,%d %d,%d,%d,%d %d\n",
            sprite.tex_id.c_str(),
            src_rect.x, src_rect.y, src_rect.w, src_rect.h,
            dst_rect.x, dst_rect.y, dst_rect.w, dst_rect.h,
            sprite.y - dst_rect.y
        );

        frame_msg += line;
    }

    net_agent->send_message(frame_msg);
}
