#include <array>
#include <queue>
#include <unordered_set>
#include <chrono>
#include <fstream>
#include <iostream>

constexpr size_t MAP_SIZE = 110;
static_assert(MAP_SIZE <= INT8_MAX);

typedef std::array<std::array<char, MAP_SIZE>, MAP_SIZE> Map;

enum Dir : uint16_t { LEFT = 0b00, RIGHT = 0b01, UP = 0b10, DOWN = 0b11 };
struct Coord {
    int8_t x, y;

    friend bool operator==(const Coord&, const Coord&) = default;
};

struct Ray
{
    Coord coord;
    Dir dir;

    void move() {
        switch (dir)
        {
        case LEFT:  coord.x -= 1; break;
        case RIGHT: coord.x += 1; break;
        case UP:    coord.y -= 1; break;
        case DOWN:  coord.y += 1; break;
        }
    }

    friend bool operator==(const Ray&, const Ray&) = default;
};

template<>
struct std::hash<Coord> : std::hash<uint16_t>
{
    std::size_t operator()(const Coord& c) const noexcept {
        return std::hash<uint16_t>::operator()(std::bit_cast<uint16_t>(c));
    }
};

template<>
struct std::hash<Ray> : std::hash<uint32_t>
{
    std::size_t operator()(const Ray& c) const noexcept {
        return std::hash<uint32_t>::operator()(std::bit_cast<uint32_t>(c));
    }
};

uint64_t GetCharged(const Map& m, const Ray& start)
{
    std::unordered_set<Coord> charged;
    std::unordered_set<Ray> visited;

    std::queue<Ray> q;
    q.emplace(start);

    while (!q.empty())
    {
        Ray r = q.front();
        q.pop();

        if (visited.contains(r)) continue;
        visited.emplace(r);

        r.move();
        if (r.coord.x < 0 || r.coord.x >= MAP_SIZE || r.coord.y < 0 || r.coord.y >= MAP_SIZE) continue;

        charged.emplace(r.coord);
        switch (m[r.coord.y][r.coord.x])
        {
        case '.':
            q.emplace(r);
            break;
        case '/':
            q.emplace(r.coord, static_cast<Dir>(~r.dir & 0b11));
            break;
        case '\\':
            q.emplace(r.coord, static_cast<Dir>(r.dir ^ 0b10));
            break;
        case '|':
            if (r.dir & 0b10) {
                q.emplace(r);
            }
            else {
                q.emplace(r.coord, UP);
                q.emplace(r.coord, DOWN);
            }
            break;
        case '-':
            if (r.dir & 0b10) {
                q.emplace(r.coord, LEFT);
                q.emplace(r.coord, RIGHT);
            }
            else {
                q.emplace(r);
            }
            break;
        }
    }

    return charged.size();
}

uint64_t First(const Map& m)
{
    return GetCharged(m, { {-1, 0}, RIGHT });
}

uint64_t Second(const Map& m)
{
    // Increíblemente mal optimizado. Es mejor recordar el valor desde distintos caminos,
    // para así reutilizar los valores ya calculados desde una determinada posición.

    uint64_t res = 0;
    for (int8_t i = 0; i < MAP_SIZE; i++)
    {
        res = std::max(res, GetCharged(m, { { -1, i }, RIGHT }));
        res = std::max(res, GetCharged(m, { { MAP_SIZE, i }, LEFT }));
        res = std::max(res, GetCharged(m, { { i, -1 }, DOWN }));
        res = std::max(res, GetCharged(m, { { i, MAP_SIZE}, UP }));
    }

    return res;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    Map m;
    {
        std::ifstream input("input", std::ios::binary);

        for (auto& e : m) {
            input.read(e.data(), e.size());
            input.ignore();
        }
    }

    std::cout << "Primera parte: " << First(m) << std::endl;
    std::cout << "Segunda parte: " << Second(m) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
