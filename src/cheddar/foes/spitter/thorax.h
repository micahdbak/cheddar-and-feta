#ifndef FOE_SPITTER_THORAX_OBJ
#define FOE_SPITTER_THORAX_OBJ "foe_spitter_thorax"

#include "spitter.h"
#include "sprite.h"

class SpitterThorax : public Object {
public:
    SpitterThorax(Spitter *parent, std::shared_ptr<bool> deleted_ptr);
    ~SpitterThorax();

    void step() override;

private:
    Spitter *parent;
    std::shared_ptr<bool> deleted_ptr;
    Sprite *sprite;
    SDL_FRect dst_rect;
};

class SpitterThoraxFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int parent_id;
        sscanf(options.c_str(), "%d", &parent_id);

        Object *obj = game->get_object(parent_id);
        Spitter *spitter;

        if (obj != nullptr && (spitter = dynamic_cast<Spitter*>(obj)) != nullptr) {
            std::shared_ptr<bool> deleted_ptr = spitter->get_deleted_ptr();
            return new SpitterThorax(spitter, deleted_ptr);
        } else {
            return nullptr;
        }
    }
};

#endif