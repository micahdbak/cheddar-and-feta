#include "game.h"
#include "inventory.h"
#include "item.h"
#include "keyboard.h"
#include "mouse.h"
#include "textbox.h"

#include <iostream>

Inventory *inventory = nullptr;

Inventory::Inventory() {
    if (inventory != nullptr) {
        std::cerr << "Inventory::Inventory error: another inventory exists..?" << std::endl;
        exit(1);
    }
    inventory = this;
}

Inventory::~Inventory() {
    inventory = nullptr;
}

void Inventory::step() {
    if (this->menu != NOT_DISPLAYING) {
        if (textbox != nullptr && textbox->owner == INVENTORY_OBJ) {
            textbox->step();
            if (textbox->done && keyboard.is_hit(SDLK_RETURN)) {
                delete textbox;
                textbox = nullptr;
                this->menu = BAG;
                this->render_status_menu();
                this->render_bag_menu();
            }

            return; // don't do any logic while textbox is running
        }

        if (keyboard.is_hit(SDLK_TAB) && textbox == nullptr) {
            this->menu = NOT_DISPLAYING;
            mouse->locked = false;
            game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
            return;
        }
    }

    switch (this->menu) {
    case NOT_DISPLAYING: {
        // another menu is displaying - do nothing
        if (mouse->locked)
            return;

        if (keyboard.is_hit(SDLK_TAB)) {
            this->menu = BAG;
            mouse->locked = true;
            this->render_status_menu();
            this->render_bag_menu();
        } else if (keyboard.is_hit(SDLK_T) && this->n_attack_items() > 0) {
            // attack item was probably used; let's set the selection back to zero
            if (this->sel_attack_item != 0 && this->attack_item == "")
                this->sel_attack_item = 0;

            // increment and make sure it doesn't pass the max attack items
            this->sel_attack_item++;
            if (this->sel_attack_item >= this->n_attack_items()+1)
                this->sel_attack_item = 0;

            // if zero, no attack item selected
            if (this->sel_attack_item == 0) {
                this->attack_item = "";
            } else {
                // select the corresponding attack item in inventory
                for (int i = 0, j = 0; i < this->items.size(); i++) {
                    if (item_info[this->items[i]].type == THROWABLE) {
                        if (j == this->sel_attack_item-1) {
                            this->attack_item = this->items[i];
                            break; // no need to iterate further
                        }
                        j++;
                    }
                }
            }

            this->render_cycle_attack_item();
            this->cycle_attack_item_ticks = game->ticks;
        }

        if (this->cycle_attack_item_ticks != 0 && game->ticks - this->cycle_attack_item_ticks > 1000) {
            game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
            this->cycle_attack_item_ticks = 0;
        }
    } break;

    case BAG: {
        if (keyboard.is_hit(SDLK_RETURN) && !this->items.empty()) {
            this->menu = ITEM;
            this->sel_action = 0;
            this->render_item_menu();
            return;
        }

        int y_change = keyboard.is_hit(SDLK_DOWN) - keyboard.is_hit(SDLK_UP);
        if (y_change != 0 && !this->items.empty()) {
            this->sel_item = cnf_clamp(this->sel_item + y_change, 0, this->items.size()-1);
            this->render_bag_menu();
        }
    } break;

    case ITEM: {
        // go back
        if (keyboard.is_hit(SDLK_BACKSPACE)) {
            this->menu = BAG;
            game->draw_rect(NULL, 0, 0, 0, 0, SDL_BLENDMODE_NONE);
            this->render_status_menu();
            this->render_bag_menu();
            return;
        }

        if (keyboard.is_hit(SDLK_RETURN)) {
            std::string item = this->items[this->sel_item];
            if (!item_info.contains(item)) {
                std::cerr << "Inventory::step error: '" << item << "' is not a valid item." << std::endl;
                exit(1);
            }

            std::string action_text;
            char buff[256];

            switch (this->sel_action) {
            case 0: /* use */
                switch (item_info[item].type) {
                case USEFUL:
                case EDIBLE:
                    action_text = "Used the item.";

                    break;

                case THROWABLE:
                    this->attack_item = item;
                    snprintf(buff, sizeof(buff), "The %s will be thrown on the next attack.", item_info[item].name.c_str());
                    action_text = buff;

                    break;

                case WEAPON:
                    this->equipped_weapon = item;
                    snprintf(buff, sizeof(buff), "Equipped the %s.", item_info[item].name.c_str());
                    action_text = buff;
                    
                    break;

                case ARMOUR:
                    this->equipped_armour = item;
                    snprintf(buff, sizeof(buff), "Equipped the %s.", item_info[item].name.c_str());
                    action_text = buff;

                    break;
                }

                break;

            case 1: /* describe */
                action_text = item_info[item].description;

                break;

            case 2: /* throw away */
                if (this->remove_item(item)) {
                    snprintf(buff, sizeof(buff), "%d,%d", int(mouse->x) + SDL_rand(5) - 2, int(mouse->y) + SDL_rand(5) - 2);
                    game->push_object(item + DROPPED_OBJ, buff);
                    
                    snprintf(buff, sizeof(buff), "Threw the %s away.", item_info[item].name.c_str());
                    action_text = buff;

                    this->sel_item = cnf_clamp(this->sel_item, 0, this->items.size()-1);
                }

                break;
            }

            textbox = new Textbox(action_text, INVENTORY_OBJ, DEFAULT_FONT, 50);
        }

        int y_change = keyboard.is_hit(SDLK_DOWN) - keyboard.is_hit(SDLK_UP);
        if (y_change != 0) {
            this->sel_action = cnf_clamp(this->sel_action + y_change, 0, NUM_ACTION_OPTIONS-1);
            this->render_item_menu();
        }
    } break;

    default:
        break;
    }
}

void Inventory::save_data() {
    // TODO
}

void Inventory::post_save_data() {
    // TODO
}

bool Inventory::push_item(const std::string &item_id) {
    if (this->items.size() == this->max_items)
        return false;

    if (!item_info.contains(item_id)) {
        std::cerr << "Inventory::push_item error: '" << item_id << "' is not a valid item." << std::endl;
        exit(1);
    }

    this->items.push_back(item_id);

    return true;
}

bool Inventory::remove_item(const std::string &item_id) {
    for (auto it = this->items.begin(); it != this->items.end(); ++it) {
        if (*it == item_id) {
            this->items.erase(it);
            return true;
        }
    }

    return false;
}

int Inventory::add_cheese(int amount) {
    int remaining_amount = this->max_cheese - this->cheese;

    if (remaining_amount < 1)
        return 0;

    if (remaining_amount < amount) {
        this->max_cheese += remaining_amount;
        return remaining_amount;
    }

    this->max_cheese += amount;
    return amount;
}

int Inventory::n_attack_items() {
    int count = 0;

    for (std::string item_id : this->items) {
        if (item_info[item_id].type == THROWABLE)
            count++;
    }

    return count;
}

void Inventory::render_cycle_attack_item() {
    int attack_items = this->n_attack_items();
    SDL_FRect bag_rect = { 4.0f, 4.0f, 80.0f, float(32 + 8*attack_items) };
    game->draw_ui_box(BOX_MENU_CONT, &bag_rect);
    game->draw_text("Next Attack:", SMALL_FONT, 12, 12, 0);

    SDL_FRect item_area = { 10.0f, 20.0f, 68.0f, 8.0f };
    game->draw_ui_box(this->sel_attack_item == 0 ? BOX_UNDER_SEL : BOX_UNDER, &item_area);
    game->draw_text("(Use Weapon)", this->sel_attack_item == 0 ? SM_BOLD_FONT : SMALL_FONT, 12, 20, 0);

    for (int i = 0, j = 0; j < items.size(); j++) {
        if (item_info[this->items[j]].type != THROWABLE)
            continue;

        i++; // corresponds with this->sel_attack_item as i starts at 0 - first item is 1

        int y = 20 + i*8;
        SDL_FRect item_area = { 10.0f, float(y), 68.0f, 8.0f };
        game->draw_ui_box(this->sel_attack_item == i ? BOX_UNDER_SEL : BOX_UNDER, &item_area);
        game->draw_text(item_info[this->items[j]].name, this->sel_attack_item == i ? SM_BOLD_FONT : SMALL_FONT, 12, y, 0);
    }
}

void Inventory::render_status_menu() {
    SDL_FRect status_rect = { 4.0f, 148.0f, 64.0f, 88.0f };
    game->draw_ui_box(BOX_MENU_CONT, &status_rect);

    game->draw_text(mouse->name, SM_BOLD_FONT, 12, 156, 0);

    char status[256];
    snprintf(status, sizeof(status),
        "Level 1\n"
        "HP %d/%d\n"
        "XP 0/0\n\n"
        "%s\n"
        "~Damage %d\n"
        "%s\n"
        "~Armour %d",

        /* level */
        mouse->health, mouse->max_health,
        /* xp */
        this->equipped_weapon.c_str(),
        mouse->damage,
        this->equipped_armour.c_str(),
        mouse->armour
    );
    game->draw_text(std::string(status), SMALL_FONT, 12, 164, 0);
}

void Inventory::render_bag_menu() {
    SDL_FRect bag_rect = { 4.0f, 4.0f, 80.0f, float(32 + 8*this->max_items) };
    game->draw_ui_box(BOX_MENU_CONT, &bag_rect);

    char leading_text[256];
    snprintf(leading_text, sizeof(leading_text), "Bag:\n%d/%d Cheese", this->cheese, this->max_cheese);
    game->draw_text(std::string(leading_text), SMALL_FONT, 12, 12, 0);

    for (int i = 0; i < this->max_items; i++) {
        int y = 28 + i*8;
        SDL_FRect item_area = { 10.0f, float(y), 68.0f, 8.0f };
        game->draw_ui_box(i == this->sel_item ? BOX_UNDER_SEL : BOX_UNDER, &item_area);

        if (i < this->items.size()) {
            if (!item_info.contains(this->items[i])) {
                std::cerr << "Inventory::render_bag_menu error: '" << this->items[i] << "' is not a valid item." << std::endl;
                exit(1);
            }

            std::string item = item_info[this->items[i]].name;
            game->draw_text(item, i == this->sel_item ? SM_BOLD_FONT : SMALL_FONT, 12, y, 0);
        }
    }
}

void Inventory::render_item_menu() {
    std::string item_id = this->items[this->sel_item];
    if (!item_info.contains(item_id)) {
        std::cerr << "Inventory::render_item_menu error: '" << item_id << "' is not a valid item." << std::endl;
        exit(1);
    }

    int y = 12 + 8*this->sel_item;
    SDL_FRect item_rect = { 76.0f, float(y), 80.0f, 48.0f };
    game->draw_ui_box(BOX_MENU_CONT, &item_rect);

    std::string action_text;

    switch (item_info[item_id].type) {
    case USEFUL: action_text = "Use"; break;
    case EDIBLE: action_text = "Eat"; break;
    case THROWABLE: action_text = "Next Attack"; break;
    case WEAPON: action_text = "Equip Weapon"; break;
    case ARMOUR: action_text = "Equip Armour"; break;
    }

    game->draw_text(item_info[item_id].name + ":", SMALL_FONT, 84, y+8, 0);

    for (int i = 0; i < NUM_ACTION_OPTIONS; i++) {
        std::string option_text;
        switch (i) {
        case 0: option_text = action_text; break;
        case 1: option_text = "Describe"; break;
        default: option_text = "Throw Away"; break;
        }

        int action_y = y + 16 + i*8;
        SDL_FRect option_rect = { 82.0f, float(action_y), 68.0f, 8.0f };
        game->draw_ui_box(this->sel_action == i ? BOX_UNDER_SEL : BOX_UNDER, &option_rect);
        game->draw_text(option_text, this->sel_action == i ? SM_BOLD_FONT : SMALL_FONT, 84, action_y, 0);
    }
}
