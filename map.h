#pragma once

#include "tile.h"
#include <memory>
#include <vector>

class map {
  uint16_t w, h;
  std::vector<std::unique_ptr<tile>> tiles;

public:
  map(const uint16_t w, const uint16_t h) : w(w), h(h) {
    for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {
        tile::neighbours n;
        n.top = get(j, i - 1);
        n.bottom = get(j, i + 1);
        n.left = get(j - 1, i);
        n.right = get(j + 1, i);
        n.top_right = get(j + 1, i - 1);
        n.top_left = get(j - 1, i - 1);
        n.bottom_right = get(j + 1, i + 1);
        n.bottom_left = get(j - 1, i + 1);
        tiles.emplace_back(std::make_unique<tile>(j, i, n));
      }
    }
  }

  tile *get(const uint16_t x, const uint16_t y) {
    if (x >= w)
      return nullptr;
    if (y >= h)
      return nullptr;
    const uint16_t i = y * w + x;
    return (i < tiles.size()) ? tiles[i].get() : nullptr;
  }
};