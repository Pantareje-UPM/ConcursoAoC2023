#include <array>
#include <unordered_set>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cassert>

constexpr uint8_t MAP_SIZE = 140;

struct Node
{
    uint8_t up : 1;
    uint8_t down : 1;
    uint8_t left : 1;
    uint8_t right : 1;
};

struct Coord {
    uint8_t x, y;
    bool operator==(const Coord& o) const noexcept { return x == o.x && y == o.y; }
};

template<>
struct std::hash<Coord>
{
    size_t operator()(const Coord& a) const noexcept {
        return std::hash<uint16_t>{}((a.x << 8) | a.y);
    }
};

typedef std::array<std::array<Node, MAP_SIZE>, MAP_SIZE> Map;

static uint16_t First(const Map& map, std::unordered_set<Coord>& marked, uint8_t startX, uint8_t startY)
{
    // Al ser un ciclo sin ramificaciones, elegimos una dirección y tiramos

    uint16_t distance = 0;

    uint8_t prevX = startX;
    uint8_t prevY = startY;

    uint8_t x = startX;
    uint8_t y = startY;

    while (distance < MAP_SIZE * MAP_SIZE)
    {
        marked.emplace(x, y);
        const auto& node = map[y][x];

        // Comprobamos la conexión superior
        if (node.up && y > 0 && map[y - 1][x].down && prevY != y - 1)
        {
            prevX = x;
            prevY = y;
            y -= 1;
        }
        // Comprobamos la conexión inferior
        else if (node.down && y < MAP_SIZE - 1 && map[y + 1][x].up && prevY != y + 1)
        {
            prevX = x;
            prevY = y;
            y += 1;
        }
        // Comprobamos la conexión izquierda
        else if (node.left  && x > 0 && map[y][x - 1].right && prevX != x - 1)
        {
            prevX = x;
            prevY = y;
            x -= 1;
        }
        // Comprobamos la conexión derecha
        else if (node.right && x < MAP_SIZE - 1 && map[y][x + 1].left && prevX != x + 1)
        {
            prevX = x;
            prevY = y;
            x += 1;
        }

        distance++;

        if (x == startX && y == startY) {
            assert(distance % 2 == 0);
            return distance / 2;
        }
    }

    throw std::exception();
}

static uint16_t Second(const Map& map, const std::unordered_set<Coord>& marked)
{
    uint16_t res = 0;
    for (uint8_t i = 0; i < MAP_SIZE; i++)
    {
        const auto& row = map[i];
        bool inside = false;
        bool startedUp = false;

        for (uint8_t j = 0; j < MAP_SIZE; j++)
        {
            const auto& node = row[j];
            if (marked.contains({ j, i })) {
                if (node.up && node.down) {
                    inside = !inside;
                }
                else {
                    if (node.up || node.down)
                    {
                        if (node.right)
                            startedUp = node.up;
                        else if (startedUp == node.down)
                            inside = !inside;
                    }
                }
            }
            else {
                res += inside;
            }
        }
    }

    return res;
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

    uint8_t startX = 0;
    uint8_t startY = 0;

    {
        std::ifstream input("input", std::ios::binary);
        for (uint8_t i = 0; i < MAP_SIZE; i++)
        {
            for (uint8_t j = 0; j < MAP_SIZE; j++)
            {
                char c = static_cast<char>(input.get());
                switch (c)
                {
                case '|': g_map[i][j] = { 1, 1, 0, 0 }; break;
                case '-': g_map[i][j] = { 0, 0, 1, 1 }; break;
                case 'L': g_map[i][j] = { 1, 0, 0, 1 }; break;
                case 'J': g_map[i][j] = { 1, 0, 1, 0 }; break;
                case '7': g_map[i][j] = { 0, 1, 1, 0 }; break;
                case 'F': g_map[i][j] = { 0, 1, 0, 1 }; break;
                default:  g_map[i][j] = { 0, 0, 0, 0 }; break;

                case 'S':
                    g_map[i][j] = { 1, 1, 1, 1 };
                    startY = i;
                    startX = j;
                break;
                }
            }
            input.ignore();
        }
    }

    assert(startX > 0 && startX < MAP_SIZE - 1);
    assert(startY > 0 && startY < MAP_SIZE - 1);

    g_map[startY][startX] = {
        g_map[startY - 1][startX].down,
        g_map[startY + 1][startX].up,
        g_map[startY][startX - 1].right,
        g_map[startY][startX + 1].left,
    };

    std::unordered_set<Coord> marked;

    std::cout << "Primera parte: " << First(g_map, marked, startX, startY) << std::endl;
    std::cout << "Segunda parte: " << Second(g_map, marked) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
