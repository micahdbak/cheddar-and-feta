#include "mouse.h"
#ifndef ITEM_COFFEE_BEAN
#define ITEM_COFFEE_BEAN "item_coffee_bean"

#include "item.h"

#include <iostream>

class DroppedCoffeeBean : public DroppedItem {
public:
    DroppedCoffeeBean(float x, float y):
        DroppedItem(x, y, "sprites/item_coffee_bean.bmp", 16, 16, 0) {}
    ~DroppedCoffeeBean() = default;

    void step() override { this->dropped_step(); }

    bool take(Mouse *mouse) override {
        return mouse->push_item(ITEM_COFFEE_BEAN);
    }
};

class DroppedCoffeeBeanFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y;
        sscanf(options.c_str(), "%d,%d", &x, &y);
        return new DroppedCoffeeBean(float(x), float(y));
    }
};

class UsedCoffeeBean : public Object {
public:
    UsedCoffeeBean(Mouse *mouse): mouse(mouse) {
        this->timer = game->ticks + 5000;

        if (mouse != nullptr)
            mouse->max_mov_speed = MOUSE_DEFAULT_SPEED * 2.0f;
    }
    ~UsedCoffeeBean() = default;

    void step() override {
        if (this->mouse == nullptr || game->ticks > this->timer) {
            this->mouse->max_mov_speed = MOUSE_DEFAULT_SPEED;
            game->delete_object = true;
        }
    }

private:
    Mouse *mouse;
    Uint32 timer = 0;
};

class UsedCoffeeBeanFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) override {
        int x, y, x_dir, y_dir, from_id;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir, &from_id);

        Mouse *mouse = nullptr;
        if (cheddar != nullptr && cheddar->id == from_id)
            mouse = cheddar;
        else if (feta != nullptr && feta->id == from_id)
            mouse = feta;

        return new UsedCoffeeBean(mouse);
    }
};

#endif
