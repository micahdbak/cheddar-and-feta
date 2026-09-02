#ifndef ITEM_SAVE
#define ITEM_SAVE "item_save"
// an extra _ is used to make sure it's ordered before other items

#include "game.h"
#include "mouse.h"
#include "object.h"
#include "save_data.h"

class ItemSaveUse : public Object {
public:
    ItemSaveUse(bool is_feta): is_feta(is_feta) {}
    ~ItemSaveUse() = default;

    void step() {
        // feta shouldnt trigger saves - theyre a remote player, seems insecure
        if (this->is_feta) {
            game->delete_object = true;
            return;
        }

        if (cheddar != nullptr)
            cheddar->push_item(ITEM_SAVE);

        game->save_objects();
        game->display_notification("Saved to file " + std::to_string(save.geti(SAVE_FILE) + 1) + ".");
        save.write_file(save.geti(SAVE_FILE));

        // all done
        game->delete_object = true;
    }

private:
    bool is_feta;
};

class ItemSaveUseFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int obj_id;
        if (1 != sscanf(options.c_str(), "%*d,%*d,%*d,%*d,%d", &obj_id)) FATAL_ERROR

        bool is_feta = feta != nullptr && feta->id == obj_id;

        return new ItemSaveUse(is_feta);
    }
};

#endif