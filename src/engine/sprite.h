#ifndef SPRITE_H
#define SPRITE_H

#include <SDL3/SDL.h>

class Sprite {
public:
    Sprite(const char *bmp_path, int frame_w, int frame_h, int interval_ms);
    ~Sprite();

    void set_animation(int animation);
    void update_frame();
    void set_frame(int frame_i);

    SDL_FRect frame;
    SDL_Texture *texture;
    int frame_w, frame_h, interval_ms, animation = 0;
private:
    int sheet_w, sheet_h, frame_i;
    Uint64 frame_last_set;
};

#endif
