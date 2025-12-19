#include "bmp_texture.h"
#include "game.h"
#include "net_agent.h"
#include "net_receiver.h"
#include "audio_playback.h"

#include <iostream>
#include <vector>

#define BUFF_SIZE 1024

#define ITEM_NONE "item_none"

static std::vector<Game::HudItem> read_items(const char *arr) {
    std::vector<Game::HudItem> ret;

    if (arr[0] != '\0' && arr[0] != '\n') {
        int i = 0;
        do {
            char item_id[256];
            int count = 1;
            sscanf(arr, "%255[^*] * %d", item_id, &count);

            Game::HudItem item{ item_id, count };
            ret.push_back(item);

            while (*arr != '\0' && *arr != '\n' && *arr != ',')
                arr++;

            if (*arr == ',')
                arr++;

            if (*arr == '\0' || *arr == '\n')
                break;
        } while (i++ < 100);
    }

    return ret;
}

void NetReceiver::step() {
    std::string frame_msg = net_agent->last_message();

    NetworkAgent::State state = net_agent->get_state();

    if (state != NetworkAgent::State::CONNECTED) {
        // if currently in a map other than the init map, reset the network agent and go to that
        if (game->current_map != "maps/init") {
            // std::cout << "resetting" << std::endl;
            net_agent->try_reset();
            game->map = "maps/init";
            last_frame_ticks = 0;
        }

        return;
    }

    // reuse last frame
    if (frame_msg.empty()) {
        for (Game::SpriteRender &sprite : this->sprites) {
            game->push_sprite(sprite.tex_id, sprite.texture, sprite.src_rect, sprite.dst_rect, sprite.y);
        }

        // 5 seconds of no frames; disconnect
        if (game->ticks - last_frame_ticks > 5000 && last_frame_ticks != 0) {
            net_agent->try_reset();
            last_frame_ticks = 0;
        }
    } else {
        last_frame_ticks = game->ticks;

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
        int view_x = SCREEN_WIDTH / 2, view_y = SCREEN_HEIGHT / 2;
        sscanf(line, "%d,%d", &view_x, &view_y);
        game->set_view(view_x, view_y);
        set_listener(view_x, view_y);

        line = next_line(line);
        if (*line == '\0') return;

        std::vector<Game::HudItem> items = read_items(line);

        line = next_line(line);
        if (*line == '\0') return;

        // read hud information
        
        if (game->current_map != "maps/splash" &&
            game->current_map != "maps/init" &&
            game->current_map != "maps/dead" &&
            game->current_map != "maps/credits") {
        int sel_item, health, max_health;
            sscanf(line, "%d,%d,%d", &sel_item, &health, &max_health);
            game->draw_hud(game->ui, items, sel_item, health, max_health);
        }

        line = next_line(line);
        if (*line == '\0') return;

        int num_sprites = 0;
        sscanf(line, "%d", &num_sprites);

        // read sprites
        for (int i = 0; i < num_sprites; i++) {
            line = next_line(line);
            if (*line == '\0') return;

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
                // std::cout << "FrameConsumer::step error: bad texture id from Cheddar: " << buff << std::endl;
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
        }

        line = next_line(line);
        if (*line == '\0') return;

        int num_audio = 0;
        sscanf(line, "%d", &num_audio);

        // read audios
        for (int i = 0; i < num_audio; i++) {
            line = next_line(line);
            if (*line == '\0') return;

            int gain_i, x, y;
            float gain;

            sscanf(line, "%1023s %d,%d,%d", buff, &gain_i, &x, &y);
            gain = (float)gain_i / 10.0f;

            play_audio(std::string(buff), gain, (float)x, (float)y);
        }
    }
}