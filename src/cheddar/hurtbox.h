#ifndef HURTBOX_OBJ
#define HURTBOX_OBJ "hurtbox"

#include "object.h"
#include "foes/foe.h"
#include "hitbox.h"
#include "mouse.h"

#include <map>
#include <vector>

/*
 * Main ideas:
 * - A hurt box represents a fixed-size rectangle, offset from the object that created it
 * - Can be damaged with hurt(damage, hitbox_id, cooldown_ms) - this is done by hitboxes
 * - Can only be hurt by a hitbox once within a cooldown
 */
class HurtBox : public Object {
public:
    static std::string Options(int owner_id, int x_off, int y_off, int w, int h, bool absorbs) {
        char buff[256];
        snprintf(buff, sizeof(buff), "%d,%d,%d,%d,%d,%d", owner_id, x_off, y_off, w, h, absorbs);
        return std::string(buff);
    }

    HurtBox(int owner_id, int x_off, int y_off, int w, int h, bool absorbs);
    ~HurtBox();

    void hurt(int damage, HitBox *hitbox, int cooldown_ms);

    void step() override;

    SDL_Rect bounding_box;

    int owner_id;
    enum OwnerClass { MOUSE, FOE } owner_class;

    bool absorbs = false;

private:
    int x_off, y_off;

    Object *owner;
    Mouse *mouse;
    Foe *foe;

    std::shared_ptr<bool> owner_deleted;

    // maps a hitbox id to when it last hit (in ticks)
    std::map<int, uint64_t> previous_hits;

    SDL_FRect src_rect, dst_rect;
};

class HurtBoxFactory : public ObjectFactory {
public:
    Object *create(const std::string &options) {
        int owner_id, x_off, y_off, w, h, absorbs;
        sscanf(options.c_str(), "%d,%d,%d,%d,%d,%d", &owner_id, &x_off, &y_off, &w, &h, &absorbs);
        return new HurtBox(owner_id, x_off, y_off, w, h, absorbs == 1);
    }
};

extern std::vector<HurtBox *> hurtboxes;

#endif