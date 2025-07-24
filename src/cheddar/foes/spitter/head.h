#ifndef FOE_SPITTER_HEAD_OBJ
#define FOE_SPITTER_HEAD_OBJ "foe_spitter_head"

#include "spitter.h"
#include "sprite.h"

class SpitterHead : public Object {
public:
    SpitterHead(Spitter *parent, std::shared_ptr<bool> deleted_ptr);
    ~SpitterHead();

    void step() override;

private:
    Spitter *parent;
    std::shared_ptr<bool> deleted_ptr;
    Sprite *sprite;
    SDL_FRect dst_rect;
};

class SpitterHeadFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int parent_id;
        sscanf(options.c_str(), "%d", &parent_id);

        Object *obj = game->get_object(parent_id);
        Spitter *spitter;

        if (obj != nullptr && (spitter = dynamic_cast<Spitter*>(obj)) != nullptr) {
            std::shared_ptr<bool> deleted_ptr = spitter->get_deleted_ptr();
            return new SpitterHead(spitter, deleted_ptr);
        } else {
            return nullptr;
        }
    }
};

#endif