#include "bmp_texture.h"
#include "game.h"
#include "net_agent.h"
#include "net_receiver.h"

#include <iostream>

#define BUFF_SIZE 1024

void NetReceiver::step() {
    std::string frame_msg = net_agent->last_message();

    // reuse last frame
    if (frame_msg.empty()) {
        for (Game::SpriteRender &sprite : this->sprites) {
            game->push_sprite(sprite.tex_id, sprite.texture, sprite.src_rect, sprite.dst_rect, sprite.y);
        }
    } else {
        // free stuff
        for (Game::SpriteRender &sprite : this->sprites) {
            delete sprite.src_rect;
            delete sprite.dst_rect;
        }
        this->sprites.clear();

        // read current map
        const char *line = frame_msg.c_str();
        if (*line == '\0') return;
        char buff[BUFF_SIZE];
        sscanf(line, "%1023[^\n]", buff);
        if (buff[0] == '\0') {
            if (!game->current_map.empty()) {
                // no map loaded, so unload the previous one
                game->unload();
            }

            return; // black screen
        } else if (game->current_map != buff) {
            game->map = buff;
            return; // ignore all sprites - need to load the map first
        }
        line = next_line(line);
        if (*line == '\0') return;

        // read view information
        int view_x, view_y;
        sscanf(line, "%d,%d", &view_x, &view_y);
        game->set_view(view_x, view_y);
        line = next_line(line);
        if (*line == '\0') return;

        // read hud information
        int item_count, health, max_health, cheese;
        sscanf(line, "%1023[^,],%d,%d,%d,%d", buff, &item_count, &health, &max_health, &cheese);
        game->draw_hud(game->ui, buff, item_count, health, max_health, cheese);
        line = next_line(line);
        if (*line == '\0') return;

        // for all subsequent lines
        while (*line != '\0') {
            SDL_Rect src_rect, dst_rect;
            int depth_offset;

            sscanf(line, "%1023s %d,%d,%d,%d %d,%d,%d,%d %d",
                buff,
                &src_rect.x, &src_rect.y, &src_rect.w, &src_rect.h,
                &dst_rect.x, &dst_rect.y, &dst_rect.w, &dst_rect.h,
                &depth_offset
            );

            Game::SpriteRender sprite;
            sprite.tex_id = buff; // buff is tex_id and sprite path
            sprite.texture = load_bmp_texture(sprite.tex_id);

            if (sprite.texture == nullptr) {
                std::cout << "FrameConsumer::step error: bad texture id from Cheddar: " << buff << std::endl;
                line = next_line(line);
                continue;
            }

            sprite.src_rect = new SDL_FRect(SDL_FRect{
                float(src_rect.x),
                float(src_rect.y),
                float(src_rect.w),
                float(src_rect.h)
            });

            sprite.dst_rect = new SDL_FRect(SDL_FRect{
                float(dst_rect.x - game->corner_x),
                float(dst_rect.y - game->corner_y),
                float(dst_rect.w),
                float(dst_rect.h)
            });

            sprite.y = depth_offset;

            game->push_sprite(sprite.tex_id, sprite.texture, sprite.src_rect, sprite.dst_rect, sprite.y);
            this->sprites.push_back(sprite);
            line = next_line(line);
        }
    }
}
