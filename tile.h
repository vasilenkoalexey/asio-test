#pragma once
#include <cstdlib>
#include <queue>
#include <vector>

class unit;

class tile {
  unit *u;
public:
  inline static uint64_t seq = 0;
  struct neighbours {
    tile *top;
    tile *bottom;
    tile *left;
    tile *right;
    tile *top_left;
    tile *top_right;
    tile *bottom_left;
    tile *bottom_right;
  } n{};

  struct astar {
    tile* t;
    double g = std::numeric_limits<double>::max();
    double h = std::numeric_limits<double>::max();
    double f = std::numeric_limits<double>::max();
    bool c = false;
    uint64_t seq = 0;
  } a;

  void path(tile *t) {
    const auto cmp = [](const tile *left, const tile *right) {
      return (left->a.f) > (right->a.f);
    };
    std::priority_queue<tile *, std::vector<tile *>, decltype(cmp)> q3(cmp);

    a.seq = tile::seq++;
    a.g = .0f;
    a.h = .0f;
    a.f = .0f;
    a.c = false;

    q3.push(this);
    tile *t01 = nullptr;
    while (!q3.empty()) {
      tile *t0 = q3.top();
      q3.pop();

      t0->a.c = true;
      const auto find = [&](tile *t1) {
        if (t1 != nullptr) {
          if (t1->a.seq != tile::seq) {
            t1->a.seq = tile::seq;
            t1->a.g = std::numeric_limits<double>::max();
            t1->a.h = std::numeric_limits<double>::max();
            t1->a.f = std::numeric_limits<double>::max();
            t1->a.c = false;
          }
          if (t1 == t) {
              std::vector<tile*> p;
              p.emplace_back(t1);
              t1 = t0;
              while (t1 != nullptr) {
                  p.emplace_back(t1);
                  t1 = t1->a.t;
              }

            return true;
          } else if (t1->a.c == false) {
            double g = t0->a.g + 1.f;
            double h = std::sqrt((t1->x - t->x) * (t1->x - t->x) +
                                (t1->y - t->y) * (t1->y - t->y));
            double f = g + h;
            if (t1->a.f > f) {
              t1->a.f = f;
              t1->a.g = g;
              t1->a.h = h;
              t1->a.t = t0;
              q3.push(t1);
            }
          }
        }
        return false;
      };

      find(t0->n.top);
      find(t0->n.bottom);
      find(t0->n.left);
      find(t0->n.right);

      find(t0->n.top_left);
      find(t0->n.top_right);
      find(t0->n.bottom_left);
      find(t0->n.bottom_right);
    }
  }

  tile(const uint16_t x, const uint16_t y, const neighbours &_n)
      : x(x), y(y), u(nullptr) {
    if (_n.top != nullptr) {
      n.top = _n.top;
      n.top->n.bottom = this;
    }
    if (_n.bottom != nullptr) {
      n.bottom = _n.bottom;
      n.bottom->n.top = this;
    }
    if (_n.left != nullptr) {
      n.left = _n.left;
      n.left->n.right = this;
    }
    if (_n.right != nullptr) {
      n.right = _n.right;
      n.right->n.left = this;
    }
    if (_n.top_left != nullptr) {
      n.top_left = _n.top_left;
      n.top_left->n.bottom_right = this;
    }
    if (_n.top_right != nullptr) {
      n.top_right = _n.top_right;
      n.top_right->n.bottom_left = this;
    }
    if (_n.bottom_left != nullptr) {
      n.bottom_left = _n.bottom_left;
      n.bottom_left->n.top_right = this;
    }
    if (_n.bottom_right != nullptr) {
      n.bottom_right = _n.bottom_right;
      n.bottom_right->n.top_left = this;
    }
  }
  void set_unit(unit *u0) { u = u0; }
  const unit *get_unit() const { return u; }
  uint16_t x, y;
  inline static const float s = 16.f;
  inline static const float hs = 8.f;
};