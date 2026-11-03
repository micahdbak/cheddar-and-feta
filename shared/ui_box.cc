#include "ui_box.h"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <iostream>

#include "renderer.h"
#include "window.h"

#define UI_BOX_SIZE 4.0f

SDL_Texture* ui_box = nullptr;

static const unsigned char ui_box_bmp[] = {
#embed "ui_box.bmp"
};

void load_ui_box() {
  ui_box = thoom::Window::instance->create_texture_from_bmp_memory(
      ui_box_bmp, sizeof(ui_box_bmp), SDL_SCALEMODE_NEAREST);
  if (ui_box == nullptr) std::exit(1);
}

void draw_ui_box(int type, SDL_Texture* dst, SDL_FRect* rect) {
  if (ui_box == nullptr || dst == nullptr) {
    // TODO(micahdbak): warning
    return;
  }

  SDL_FRect _dst_rect{0.0f, 0.0f, float(dst->w), float(dst->h)};
  if (rect == nullptr) {
    rect = &_dst_rect;
  }

  // ui boxes are laid out as a 3x3 grid of cells within the ui_box texture
  float x = type * UI_BOX_SIZE * 3.0f;
  float cell = UI_BOX_SIZE;

  SDL_FRect src_rect, dst_rect;

  // top-left corner
  src_rect = {x, 0.0f, cell, cell};
  dst_rect = {rect->x, rect->y, cell, cell};
  thoom::Renderer::instance->draw_texture(dst, ui_box, &src_rect, &dst_rect);

  // top-right corner
  src_rect = {x + 2.0f * cell, 0.0f, cell, cell};
  dst_rect = {rect->x + rect->w - cell, rect->y, cell, cell};
  thoom::Renderer::instance->draw_texture(dst, ui_box, &src_rect, &dst_rect);

  // bottom-right corner
  src_rect = {x + 2.0f * cell, cell * 2.0f, cell, cell};
  dst_rect = {rect->x + rect->w - cell, rect->y + rect->h - cell, cell, cell};
  thoom::Renderer::instance->draw_texture(dst, ui_box, &src_rect, &dst_rect);

  // bottom-left corner
  src_rect = {x, cell * 2.0f, cell, cell};
  dst_rect = {rect->x, rect->y + rect->h - cell, cell, cell};
  thoom::Renderer::instance->draw_texture(dst, ui_box, &src_rect, &dst_rect);

  // top edge
  src_rect = {x + cell, 0.0f, cell, cell};
  dst_rect = {rect->x + cell, rect->y, rect->w - (2.0f * cell), cell};
  thoom::Renderer::instance->draw_texture_tiled(dst, ui_box, &src_rect, 1.0f,
                                                &dst_rect);

  // right edge
  src_rect = {x + 2.0f * cell, cell, cell, cell};
  dst_rect = {rect->x + rect->w - cell, rect->y + cell, cell,
              rect->h - (2.0f * cell)};
  thoom::Renderer::instance->draw_texture_tiled(dst, ui_box, &src_rect, 1.0f,
                                                &dst_rect);

  // bottom edge
  src_rect = {x + cell, cell * 2.0f, cell, cell};
  dst_rect = {rect->x + cell, rect->y + rect->h - cell, rect->w - (2.0f * cell),
              cell};
  thoom::Renderer::instance->draw_texture_tiled(dst, ui_box, &src_rect, 1.0f,
                                                &dst_rect);

  // left edge
  src_rect = {x, cell, cell, cell};
  dst_rect = {rect->x, rect->y + cell, cell, rect->h - (2.0f * cell)};
  thoom::Renderer::instance->draw_texture_tiled(dst, ui_box, &src_rect, 1.0f,
                                                &dst_rect);

  // middle
  src_rect = {x + cell, cell, cell, cell};
  dst_rect = {rect->x + cell, rect->y + cell, rect->w - (2.0f * cell),
              rect->h - (2.0f * cell)};
  thoom::Renderer::instance->draw_texture_tiled(dst, ui_box, &src_rect, 1.0f,
                                                &dst_rect);
}
