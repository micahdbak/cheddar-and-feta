#include "font.h"
#include "game.h"
#include "controller.h"
#include "net_agent.h"
#include "net_receiver.h"
#include "net_sender.h"

#include <iostream>
#include <string>

#define INIT_OBJ "init"

class Init : public Object {
public:
    Init() {
        this->ui = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, SCREEN_WIDTH, SCREEN_HEIGHT);
        SDL_SetTextureBlendMode(this->ui, SDL_BLENDMODE_BLEND);
        game->set_view(160, 120);
    }

    ~Init() {
        SDL_DestroyTexture(this->ui);
    }

    void step();

private:
    bool render = true, waiting_for_net_agent = false;
    int sel_c = 0;
    std::string code;
    SDL_Texture *ui = nullptr;
    SDL_FRect dst_rect = { 0.0f, 0.0f, 320.0f, 240.0f };
    NetworkAgent::State last_net_state = NetworkAgent::State::NO_CONNECTION;
    Uint64 waiting_ticks = 0;
};

class InitFactory : public ObjectFactory {
public:
    InitFactory() = default;
    ~InitFactory() = default;

    Object *create(const std::string &options) override {
        return new Init();
    }
};

void Game::init() {
    this->factories[INIT_OBJ] = new InitFactory();

    this->factories[FIRST_OBJ] = new NetReceiverFactory();
    this->factories[LAST_OBJ] = new NetSenderFactory();

    this->title = "Playing as Feta";

    this->load_map("maps/init");
    this->create_objects = false;

    net_agent = new NetworkAgent(true);

    this->create_object(FIRST_OBJ, "");
    this->create_object(LAST_OBJ, "");
}

void Init::step() {
    NetworkAgent::State net_state = net_agent->get_state();

    if (net_state != this->last_net_state)
        this->render = true;

    this->last_net_state = net_state;

    if (this->waiting_for_net_agent) {
        if (net_state == NetworkAgent::State::NO_CONNECTION)
            net_state = NetworkAgent::State::WAITING_FOR_PEER;
        else {
            this->waiting_for_net_agent = false;
            this->waiting_ticks = game->ticks;
        }
    }

    switch (net_state) {
    case NetworkAgent::State::NO_CONNECTION: {
        static char code_charset[38] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ*";
        static const int cols = 10;
        static const int rows = 4;
        bool valid_code = this->code.size() == 6;

        int x_dir = local_controller.is_hit(Button::RIGHT) - local_controller.is_hit(Button::LEFT);
        int y_dir = local_controller.is_hit(Button::DOWN) - local_controller.is_hit(Button::UP);
        bool select = local_controller.is_hit(Button::SELECT);
        bool remove = local_controller.is_hit(Button::CANCEL);

        this->sel_c += x_dir;
        this->sel_c += y_dir * cols;
        this->sel_c = cnf_clamp(this->sel_c, 0, valid_code ? 36 : 35);

        if (x_dir != 0 || y_dir != 0 || select || remove) {
            this->render = true;
        }

        if (this->render) {
            this->render = false;

            game->draw_rect(this->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

            SDL_FRect code_ui_box = { 76.0f, 56.0f, 168.0f, 40.0f };
            game->draw_ui_box(this->ui, BOX_CHAR_CONT, &code_ui_box);

            SDL_FRect code_box = { 84.0f, 64.0f, 152.0f, 24.0f };
            game->draw_ui_box(this->ui, BOX_CHAR_DISP, &code_box);

            std::string prompt = "Please enter the connection code.";
            int prompt_w = game->fonts[DEFAULT_FONT]->text_width(prompt);
            game->draw_text(this->ui, prompt, DEFAULT_FONT, 160 - prompt_w / 2, 66, 0);

            game->draw_text(this->ui, "Code:", SMALL_FONT, 130, 78, 0);
            game->draw_text(this->ui, this->code, CODE_FONT, 154, 77, 0);

            SDL_FRect ui_box = { 76.0f, 100.0f, 168.0f, 96.0f };
            game->draw_ui_box(this->ui, BOX_CHAR_CONT, &ui_box);

            const int start_x = 100;
            const int start_y = 108;

            for (int i = 0; i < 36; i++) {
                int x = start_x + (i % cols) * 12;
                int y = start_y + (i / cols) * 12;

                SDL_FRect box_rect = { (float)x, (float)y, 12.0f, 12.0f };
                game->draw_ui_box(this->ui, i == this->sel_c ? BOX_CHAR_SEL : BOX_CHAR_BOX, &box_rect);
                char c = code_charset[i];
                SDL_FRect src_rect = game->fonts[CODE_FONT]->src_rect[c - ' '];

                box_rect.x = (float)(int)(x + 6 - (int)src_rect.w / 2);
                box_rect.y = (float)(int)(y + 6 - (int)src_rect.h / 2);
                box_rect.w = src_rect.w;
                box_rect.h = src_rect.h;

                SDL_SetRenderTarget(renderer, this->ui);
                SDL_RenderTexture(renderer, game->fonts[i == this->sel_c ? CODE_FONT : CODE_GRAY_FONT]->texture, &src_rect, &box_rect);
                SDL_SetRenderTarget(renderer, game->screen);
            }

            if (valid_code) {
                SDL_FRect enter_box = { 172.0f, 144.0f, 18.0f, 12.0f };
                game->draw_ui_box(this->ui, 36 == this->sel_c ? BOX_CHAR_SEL : BOX_CHAR_BOX, &enter_box);
                game->draw_text(this->ui, "OK", 36 == this->sel_c ? CODE_FONT : CODE_GRAY_FONT, 175, 145, 0);
            }

            game->draw_text(this->ui, "SEL:", SMALL_FONT, 100, 164, 0);
            game->draw_text(this->ui, "0/A/W", CONTROLS_FONT, 116, 160, 0);

            game->draw_text(this->ui, "DEL:", SMALL_FONT, 168, 164, 0);
            game->draw_text(this->ui, "1/B/X", CONTROLS_FONT, 184, 160, 0);

            game->draw_text(this->ui, "NAV:", SMALL_FONT, 122, 180, 0);
            game->draw_text(this->ui, "45/EF/[\\", CONTROLS_FONT, 140, 176, 0);

            if (this->sel_c == 36) {
                if (select && valid_code) {
                    net_agent->set_connection_code(this->code);
                    this->waiting_for_net_agent = true;
                    this->render = true;
                }
            } else if (select && this->code.size() < 6) {
                this->code += code_charset[this->sel_c];
                this->render = true; // render character on next frame
            }

            if (remove && !this->code.empty()) {
                this->code.pop_back();
                this->render = true; // remove character on next frame
            }
        }
    } break;

    // WAITING_FOR_PEER, CONNECTION
    default:
        if (this->render) {
            this->render = false;

            game->draw_rect(this->ui, NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);

            SDL_FRect waiting_box = { 76.0f, 100.0f, 168.0f, 40.0f };
            game->draw_ui_box(this->ui, BOX_CHAR_CONT, &waiting_box);

            SDL_FRect waiting_disp_box = { 84.0f, 108.0f, 152.0f, 24.0f };
            game->draw_ui_box(this->ui, BOX_CHAR_DISP, &waiting_disp_box);

            std::string waiting = "Connecting with code " + this->code + "...";
            int waiting_w = game->fonts[DEFAULT_FONT]->text_width(waiting);

            game->draw_text(this->ui, waiting, DEFAULT_FONT, 160 - waiting_w / 2, 116, 0);
        }

        // no connection after 15 seconds
        if (this->waiting_ticks != 0 && game->ticks - this->waiting_ticks > 15000) {
            net_agent->try_reset();
            game->display_notification("That code didn't work.");
            this->waiting_ticks = 0;
        }

        break;
    }

    game->push_sprite("", this->ui, nullptr, &this->dst_rect, 0);
}