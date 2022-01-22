#pragma once

#include <complex>
#include <filesystem>
#include <fstream>
#include <functional>
#include <vector>

#include "cbor11.h"
#include "tile.h"

class unit_type {
public:
  float move_velocity = .000001f;
  float turn_velocity = .00000001f;
};

class unit {
public:
  inline static uint64_t id_seq = 0;
  inline static const float s2 = 1.f / std::sqrt(2.f);
  using tfun = void (unit::*)(const long long, cbor::map &);
  unit(tile *t, const float a, const std::shared_ptr<unit_type> type)
      : type(std::move(type)), time(std::chrono::steady_clock::now()),
        move(&unit::stop), a(a), id(id_seq++), t(t) {
    x = tile::s * (t->x + 0.5f);
    y = tile::s * (t->y + 0.5f);
    t->set_unit(this);
  }
  std::shared_ptr<unit_type> type;
  tfun move;
  std::deque<tfun> p;
  std::chrono::steady_clock::time_point time;
  tile *t;
  uint64_t id;
  float x, y, a;
  void update(cbor::array& buf) {
    cbor::map m{};
    const std::chrono::steady_clock::time_point now =
        std::chrono::steady_clock::now();
    const auto delta =
        std::chrono::duration_cast<std::chrono::microseconds>(now - time)
            .count();
    if (delta > 5000) {
      (this->*move)(delta, m);
      time = now;
      if(!m.empty()) {
        buf.emplace_back(cbor::map{std::make_pair(id, m)});
      }
    }
  }

  void swap_tile(tile *st) {
    t->set_unit(nullptr);
    st->set_unit(this);
    t = st;
  }

  template <float B> bool clockwise() {
    if (a > B) {
      const float x = a - B;
      const float y = 1.f - a + B;
      return (x > y);
    }
    const float x = B - a;
    const float y = 1.f - B + a;
    return (x < y);
  }

  template <float B> void turn(const long long delta) {
    if (clockwise<B>()) {
      const float b = a + type->turn_velocity * delta;
      if constexpr (B == .0f) {
        a = (a < 1.f && b > 1.f) ? B : b;
      } else {
        a = (a < B && b > B) ? B : b;
      }
    } else {
      const float b = (a > type->turn_velocity * delta)
                          ? a - type->turn_velocity * delta
                          : 1.f + a - type->turn_velocity * delta;
      a = (a > B && b < B) ? B : b;
    }
  }
  void top(const long long delta, cbor::map &m) {
    if (a == .0f) {
      const int _y = static_cast<int>(y);
      const float y1 = y - type->move_velocity * delta;
      if (t->n.top != nullptr) {
        const float ym = t->y * tile::s;
        if (y > ym && y1 < ym)
          swap_tile(t->n.top);
      }
      const float yt = t->y * tile::s + tile::hs;
      if (y > yt && y1 < yt) {
        if (p.empty()) {
          y = yt;
          move = &unit::stop;
        } else {
          if (move == p.front()) {
            y = y1;
          } else {
            move = p.front();
            y = yt;
          }
          p.pop_front();
        }
      } else {
        y = y1;
      }
      if (_y != static_cast<int>(y)) {
        m.emplace('y', y);
      }
    } else {
      turn<.0f>(delta);
      if (a == .0f) {
        m.emplace('a', .0f);
      }                              
    }
  }
  void bottom(const long long delta, cbor::map &m) {
    if (a == .5f) {
      const int yp = static_cast<int>(y);
      const float yn = y + type->move_velocity * delta;
      if (t->n.bottom != nullptr) {
        const float ym = t->n.bottom->y * tile::s;
        if (y < ym && yn > ym)
          swap_tile(t->n.bottom);
      }
      const float yt = t->y * tile::s + tile::hs;
      if (y < yt && yn > yt) {
        y = yt;
        if (p.empty()) {
          move = &unit::stop;
        } else {
          move = p.front();
          p.pop_front();
        }
      } else {
        y = yn;
      }
      if (yp != static_cast<int>(y)) {
        m.emplace('y', y);
      }
    } else {
      turn<.5f>(delta);
      if (a == .5f) {
        m.emplace('a', .5f);
      }                        
    }
  }
  void left(const long long delta, cbor::map &m) {
    if (a == .75f) {
      const int _x = static_cast<int>(x);
      const float x1 = x - type->move_velocity * delta;
      if (t->n.left != nullptr) {
        const float xm = t->x * tile::s;
        if (x > xm && x1 < xm)
          swap_tile(t->n.left);
      }
      const float xt = t->x * tile::s + tile::hs;
      if (x > xt && x1 < xt) {
        x = xt;
        if (p.empty()) {
          move = &unit::stop;
        } else {
          move = p.front();
          p.pop_front();
        }
      } else {
        x = x1;
      }
      if (_x != static_cast<int>(x)) {
        m.emplace('x', x);
      }
    } else {
      turn<.75f>(delta);
      if (a == .75f) {
        m.emplace('a', .75f);
      }                  
    }
  }
  void right(const long long delta, cbor::map &m) {
    if (a == .25f) {
      const int _x = static_cast<int>(x);
      const float x1 = x + type->move_velocity * delta;
      if (t->n.right != nullptr) {
        const float xm = t->n.right->x * tile::s;
        if (x < xm && x1 > xm)
          swap_tile(t->n.right);
      }
      const float xt = t->x * tile::s + tile::hs;
      if (x < xt && x1 > xt) {
        x = xt;
        if (p.empty()) {
          move = &unit::stop;
        } else {
          move = p.front();
          p.pop_front();
        }
      } else {
        x = x1;
      }
      if (_x != static_cast<int>(x)) {
        m.emplace('x', x);
      }
    } else {
      turn<.25f>(delta);
      if (a == .25f) {
        m.emplace('a', .25f);
      }            
    }
  }
  void top_left(const long long delta, cbor::map &m) {
    if (a == .825f) {
      const int _y = static_cast<int>(y);
      const int _x = static_cast<int>(x);
      const float x1 = x - s2 * type->move_velocity * delta;
      const float y1 = y - s2 * type->move_velocity * delta;
      if (t->n.top_left != nullptr) {
        const float xm = t->x * tile::s;
        const float ym = t->y * tile::s;
        if (x > xm && x1 < xm && y > ym && y1 < ym)
          swap_tile(t->n.top_left);
      }
      const float xt = t->x * tile::s + tile::hs;
      const float yt = t->y * tile::s + tile::hs;
      if (x > xt && x1 < xt && y > yt && y1 < yt) {
        x = xt;
        y = yt;
        if (p.empty()) {
          move = &unit::stop;
        } else {
          move = p.front();
          p.pop_front();
        }
      } else {
        x = x1;
        y = y1;
      }
      if (_x != static_cast<int>(x)) {
        m.emplace('x', x);
      }
      if (_y != static_cast<int>(y)) {
        m.emplace('y', y);
      }
    } else {
      turn<.825f>(delta);
      if (a == .825f) {
        m.emplace('a', .825f);
      }
    }
  }
  void bottom_left(const long long delta, cbor::map &m) {
    if (a == .625f) {
      const int _y = static_cast<int>(y);
      const int _x = static_cast<int>(x);
      const float x1 = x - s2 * type->move_velocity * delta;
      const float y1 = y + s2 * type->move_velocity * delta;
      if (t->n.bottom_left != nullptr) {
        const float xm = t->x * tile::s;
        const float ym = t->n.bottom_left->y * tile::s;
        if (x > xm && x1 < xm && y < ym && y1 > ym)
          swap_tile(t->n.bottom_left);
      }
      const float xt = t->x * tile::s + tile::hs;
      const float yt = t->y * tile::s + tile::hs;
      if (x > xt && x1 < xt && y < yt && y1 > yt) {
        x = xt;
        y = yt;
        if (p.empty()) {
          move = &unit::stop;
        } else {
          move = p.front();
          p.pop_front();
        }
      } else {
        x = x1;
        y = y1;
      }
      if (_x != static_cast<int>(x)) {
        m.emplace('x', x);
      }
      if (_y != static_cast<int>(y)) {
        m.emplace('y', y);
      }
    } else {
      turn<.625f>(delta);
      if (a == .625f) {
        m.emplace('a', .625f);
      }      
    }
  }
  void top_right(const long long delta, cbor::map &m) {
    if (a == .125f) {
      const int _y = static_cast<int>(y);
      const int _x = static_cast<int>(x);
      const float x1 = x + s2 * type->move_velocity * delta;
      const float y1 = y - s2 * type->move_velocity * delta;
      if (t->n.top_right != nullptr) {
        const float xm = t->n.top_right->x * tile::s;
        const float ym = t->y * tile::s;
        if (x < xm && x1 > xm && y > ym && y1 < ym)
          swap_tile(t->n.top_right);
      }
      const float xt = t->x * tile::s + tile::hs;
      const float yt = t->y * tile::s + tile::hs;
      if (x < xt && x1 > xt && y > yt && y1 < yt) {
        x = xt;
        y = yt;
        if (p.empty()) {
          move = &unit::stop;
        } else {
          move = p.front();
          p.pop_front();
        }
      } else {
        x = x1;
        y = y1;
      }
      if (_x != static_cast<int>(x)) {
        m.emplace('x', x);
      }
      if (_y != static_cast<int>(y)) {
        m.emplace('y', y);
      }
    } else {
      turn<.125f>(delta);
      if (a == .125f) {
        m.emplace('a', .125f);
      }      
    }
  }
  void bottom_right(const long long delta, cbor::map &m) {
    if (a == .375f) {
      const int _y = static_cast<int>(y);
      const int _x = static_cast<int>(x);
      const float x1 = x + s2 * type->move_velocity * delta;
      const float y1 = y + s2 * type->move_velocity * delta;
      if (t->n.bottom_right != nullptr) {
        const float xm = t->n.bottom_right->x * tile::s;
        const float ym = t->n.bottom_right->y * tile::s;
        if (x < xm && x1 > xm && y < ym && y1 > ym)
          swap_tile(t->n.bottom_right);
      }
      const float xt = t->x * tile::s + tile::hs;
      const float yt = t->y * tile::s + tile::hs;
      if (x < xt && x1 > xt && y < yt && y1 > yt) {
        x = xt;
        y = yt;
        if (p.empty()) {
          move = &unit::stop;
        } else {
          move = p.front();
          p.pop_front();
        }
      } else {
        x = x1;
        y = y1;
      }
      if (_x != static_cast<int>(x)) {
        m.emplace('x', x);
      }
      if (_y != static_cast<int>(y)) {
        m.emplace('y', y);
      }
    } else {
      turn<.375f>(delta);
      if (a == .375f) {
        m.emplace('a', .375f);
      }
    }
  }
  void stop(const long long delta, cbor::map &m) {}

  static tfun neighbour(const tile *s, const tile *d) {
    if (s->n.bottom == d)
      return &unit::bottom;
    if (s->n.top == d)
      return &unit::top;
    if (s->n.left == d)
      return &unit::left;
    if (s->n.right == d)
      return &unit::right;
    if (s->n.top_left == d)
      return &unit::top_left;
    if (s->n.top_right == d)
      return &unit::top_right;
    if (s->n.bottom_left == d)
      return &unit::bottom_left;
    if (s->n.bottom_right == d)
      return &unit::bottom_right;
    return &unit::stop;
  }

  void move_to(tile *d) {
    const auto cmp = [](const tile *left, const tile *right) {
      return (left->a.f) > (right->a.f);
    };
    std::priority_queue<tile *, std::vector<tile *>, decltype(cmp)> q3(cmp);
    t->a.seq = tile::seq++;
    t->a.g = .0f;
    t->a.h = .0f;
    t->a.f = .0f;
    t->a.c = false;
    q3.push(t);

    while (!q3.empty()) {
      tile *ts = q3.top();
      q3.pop();

      ts->a.c = true;
      const auto find = [&](tile *td) {
        if (td != nullptr) {
          if (td->a.seq != tile::seq) {
            td->a.seq = tile::seq;
            td->a.g = std::numeric_limits<float>::max();
            td->a.h = std::numeric_limits<float>::max();
            td->a.f = std::numeric_limits<float>::max();
            td->a.c = false;
          }
          if (td == d) {
            p.clear();
            // p.emplace_back(&unit::stop);
            p.emplace_back(neighbour(ts, td));
            td = ts;
            while (td != t) {
              p.emplace_back(neighbour(td->a.t, td));
              td = td->a.t;
            }
            if (move == &unit::stop) {
              move = p.front();
              p.pop_front();
            }

            return true;
          } else if (td->a.c == false) {
            const double g = ts->a.g + 1.f;
            const double h = std::sqrt((td->x - d->x) * (td->x - d->x) +
                                       (td->y - d->y) * (td->y - d->y));
            const double f = g + h;
            if (td->a.f > f) {
              td->a.f = f;
              td->a.g = g;
              td->a.h = h;
              td->a.t = ts;
              q3.push(td);
            }
          }
        }
        return false;
      };

      if (find(ts->n.top))
        return;
      if (find(ts->n.bottom))
        return;
      if (find(ts->n.left))
        return;
      if (find(ts->n.right))
        return;

      if (find(ts->n.top_left))
        return;
      if (find(ts->n.top_right))
        return;
      if (find(ts->n.bottom_left))
        return;
      if (find(ts->n.bottom_right))
        return;
    }
  }
};