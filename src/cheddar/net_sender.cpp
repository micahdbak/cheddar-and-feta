#include "game.h"
#include "items/item.h"
#include "mouse.h"
#include "net_agent.h"
#include "net_sender.h"

NetSender *net_sender = nullptr;

void NetSender::step() {
    if (net_agent->get_state() != NetworkAgent::State::CONNECTED)
        return;

    std::string frame_msg = game->current_map + '\n';

    // view
    char line[1024];
    if (feta != nullptr) {
        snprintf(line, sizeof(line), "%d,%d\n", int(feta->x), int(feta->y));
    } else {
        snprintf(line, sizeof(line), "%d,%d\n", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    }
    frame_msg += line;

    // hud
    if (feta != nullptr) {
        std::string feta_items = Mouse::encode_items(feta->items);
        feta_items += '\n';
        frame_msg += feta_items;
        snprintf(line, sizeof(line), "%d,%d,%d\n", feta->sel_item, feta->health, feta->max_health);
    } else {

        snprintf(line, sizeof(line), "\n-1,0,0\n");
    }

    frame_msg += line;

    snprintf(line, sizeof(line), "%d\n", (int)game->sprites.size());
    frame_msg += line;

    // sprites
    for (Game::SpriteRender &sprite : game->sprites) {
        if (sprite.texture == nullptr || sprite.dst_rect == nullptr || sprite.tex_id.size() == 0)
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

    snprintf(line, sizeof(line), "%d\n", (int)game->audio.size());
    frame_msg += line;

    for (Game::AudioMsg &msg : game->audio) {
        snprintf(line, sizeof(line), "%s %d,%d,%d\n", msg.wav_path.c_str(),
            (int)(10.0f * msg.gain), (int)msg.x, (int)msg.y);
        frame_msg += line;
    }

    net_agent->send_message(frame_msg);
}
