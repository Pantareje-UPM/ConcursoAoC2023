#include <array>
#include <unordered_set>
#include <queue>
#include <chrono>
#include <fstream>
#include <iostream>

constexpr uint8_t MAP_SIZE = 141;
static_assert(MAP_SIZE <= UINT8_MAX);

typedef std::array<std::array<uint8_t, MAP_SIZE>, MAP_SIZE> Map;

enum Dir : uint16_t { LEFT = 0b00, RIGHT = 0b01, UP = 0b10, DOWN = 0b11 };

struct QueueElement
{
    int16_t x, y;
    Dir dir;
    uint16_t w;

    friend bool operator<(const QueueElement& a, const QueueElement& b) {
        return a.w > b.w;
    }
};

uint16_t First(const Map& m) {
    std::priority_queue<QueueElement> queue;
    std::unordered_set<uint32_t> visited;

    constexpr auto key = [](int16_t x, int16_t y, Dir d) {
        return ((uint32_t)x << 24) | ((uint32_t)y << 16) | ((uint32_t)(d & 0b10));
    };
    const auto add = [&](int16_t x, int16_t y, Dir d, uint16_t prev_w) {
        if (x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE)
        {
            uint16_t w = prev_w + m[y][x];
            if(!visited.contains(key(x, y, d))) queue.emplace(x, y, d, w);
            return w;
        }
        return (uint16_t)0;
    };

    {
        uint16_t w;
        w = 0;
        for (uint8_t i = 1; i <= 3; i++)
            w = add(i, 0, RIGHT, w);
        w = 0;
        for (uint8_t i = 1; i <= 3; i++)
            w = add(0, i, DOWN, w);
    }

    while (!queue.empty())
    {
        const QueueElement e = queue.top();
        queue.pop();

        if (e.x == MAP_SIZE - 1 && e.y == MAP_SIZE - 1) return e.w;

        uint32_t k = key(e.x, e.y, e.dir);
        if (visited.contains(k)) continue;
        visited.emplace(k);

        if (e.dir & 0b10)
        {
            uint16_t w;
            w = e.w;
            for (uint8_t i = 1; w && i <= 3; i++)
                w = add(e.x - i, e.y, LEFT, w);
            w = e.w;
            for (uint8_t i = 1; w && i <= 3; i++)
                w = add(e.x + i, e.y, RIGHT, w);
        }
        else
        {
            uint16_t w;
            w = e.w;
            for (uint8_t i = 1; w && i <= 3; i++)
                w = add(e.x, e.y - i, UP, w);
            w = e.w;
            for (uint8_t i = 1; w && i <= 3; i++)
                w = add(e.x, e.y + i, DOWN, w);
        }
    }

    throw std::exception();
}

uint16_t Second(const Map& m) {
    std::priority_queue<QueueElement> queue;
    std::unordered_set<uint32_t> visited;

    constexpr auto key = [](int16_t x, int16_t y, Dir d) {
        return ((uint32_t)x << 24) | ((uint32_t)y << 16) | ((uint32_t)(d & 0b10));
    };
    const auto increment_w = [&](int16_t x, int16_t y, uint16_t prev_w) {
        return (x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE)
            ? (uint16_t)(prev_w + m[y][x])
            : (uint16_t)0;
    };
    const auto add = [&](int16_t x, int16_t y, Dir d, uint16_t prev_w) {
        if (x >= 0 && x < MAP_SIZE && y >= 0 && y < MAP_SIZE)
        {
            uint16_t w = prev_w + m[y][x];
            if (!visited.contains(key(x, y, d))) queue.emplace(x, y, d, w);
            return w;
        }
        return (uint16_t)0;
    };

    {
        uint16_t w;

        w = 0;
        for (uint8_t i = 1; i < 4; i++)
            w = increment_w(i, 0, w);
        for (uint8_t i = 4; i <= 10; i++)
            w = add(i, 0, RIGHT, w);

        w = 0;
        for (uint8_t i = 1; i < 4; i++)
            w = increment_w(0, i, w);
        for (uint8_t i = 4; i <= 10; i++)
            w = add(0, i, DOWN, w);
    }

    while (!queue.empty())
    {
        const QueueElement e = queue.top();
        queue.pop();

        if (e.x == MAP_SIZE - 1 && e.y == MAP_SIZE - 1) return e.w;

        uint32_t k = key(e.x, e.y, e.dir);
        if (visited.contains(k)) continue;
        visited.emplace(k);

        if (e.dir & 0b10)
        {
            uint16_t w;

            w = e.w;
            for (uint8_t i = 1; w && i < 4; i++)
                w = increment_w(e.x - i, e.y, w);
            for (uint8_t i = 4; w && i <= 10; i++)
                w = add(e.x - i, e.y, LEFT, w);

            w = e.w;
            for (uint8_t i = 1; w && i < 4; i++)
                w = increment_w(e.x + i, e.y, w);
            for (uint8_t i = 4; w && i <= 10; i++)
                w = add(e.x + i, e.y, RIGHT, w);
        }
        else
        {
            uint16_t w;

            w = e.w;
            for (uint8_t i = 1; w && i < 4; i++)
                w = increment_w(e.x, e.y - i, w);
            for (uint8_t i = 4; w && i <= 10; i++)
                w = add(e.x, e.y - i, UP, w);

            w = e.w;
            for (uint8_t i = 1; w && i < 4; i++)
                w = increment_w(e.x, e.y + i, w);
            for (uint8_t i = 4; w && i <= 10; i++)
                w = add(e.x, e.y + i, DOWN, w);
        }
    }

    throw std::exception();
}

namespace
{
    Map g_map;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    {
        std::ifstream input("input", std::ios::binary);

        for (auto& e : g_map) {
            input.read(reinterpret_cast<char*>(e.data()), e.size());
            for (auto& i : e) i -= '0';
            input.ignore();
        }
    }

    std::cout << "Primera parte: " << First(g_map) << std::endl;
    std::cout << "Segunda parte: " << Second(g_map) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
