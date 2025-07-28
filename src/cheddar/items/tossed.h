#ifndef TOSSED_ITEM_OBJ
#define TOSSED_ITEM_OBJ "tossed_item"

#include "object.h"
#include "sprite.h"

class TossedItem : public Object {
public:
    static std::string Options(int x, int y, int x_dir, int y_dir, bool from_feta, std::string item_id) {
        char buff[256];
        snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d %s", x, y, x_dir, y_dir, from_feta == true ? 1 : 0, item_id.c_str());
        return std::string(buff);
    }

    TossedItem(float x, float y, int x_dir, int y_dir, bool from_feta, std::string item_id);
    ~TossedItem();

    void step() override;

private:
    float x, y;
    int x_dir, y_dir;
    bool from_feta;
    std::string item_id;
    Sprite *sprite;
    SDL_FRect dst_rect;
    Uint64 spawned_ticks;
};

class TossedItemFactory : public ObjectFactory {
public:
    Object *create(const std::string& options) {
        int x, y, x_dir, y_dir, from_feta;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d", &x, &y, &x_dir, &y_dir, &from_feta);

        const char *arr = options.c_str();
        while (*arr != ' ' && *arr != '\0')
            arr++;

        if (*arr == ' ')
            arr++;

        if (*arr == '\0')
            return nullptr;

        return new TossedItem(x, y, x_dir, y_dir, from_feta == 1, std::string(arr));
    }
};

#endif