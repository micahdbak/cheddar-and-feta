#ifndef FOE_SPITTER_ABDOMEN_OBJ
#define FOE_SPITTER_ABDOMEN_OBJ "foe_spitter_abdomen"

#include "spitter.h"
#include "sprite.h"

class SpitterAbdomen : public Object {
public:
    SpitterAbdomen(Spitter *parent, std::shared_ptr<bool> deleted_ptr);
    ~SpitterAbdomen();

    void step() override;

private:
    Spitter *parent;
    std::shared_ptr<bool> deleted_ptr;
    Sprite *sprite;
    SDL_FRect dst_rect;
};

class SpitterAbdomenFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int parent_id;
        sscanf(options.c_str(), "%d", &parent_id);

        Object *obj = game->get_object(parent_id);
        Spitter *spitter;

        if (obj != nullptr && (spitter = dynamic_cast<Spitter*>(obj)) != nullptr) {
            std::shared_ptr<bool> deleted_ptr = spitter->get_deleted_ptr();
            return new SpitterAbdomen(spitter, deleted_ptr);
        } else {
            return nullptr;
        }
    }
};

#endif