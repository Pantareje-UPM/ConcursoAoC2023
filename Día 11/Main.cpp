#include <vector>
#include <set>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>

constexpr uint8_t MAP_LENGTH = 140;

struct Coord { uint8_t x, y; };

static uint32_t GetDistance(
    Coord a, Coord b,
    const std::vector<uint8_t>& emptyRows,
    const std::vector<uint8_t>& emptyColumns,
    uint32_t scaleFactor
)
{
    const auto& [minX, maxX] = std::minmax(a.x, b.x);
    const auto& [minY, maxY] = std::minmax(a.y, b.y);

    uint32_t increment = 0;

    if (minX != maxX) {
        const auto& first = std::ranges::lower_bound(emptyColumns, minX);
        const auto& last = std::ranges::upper_bound(emptyColumns, maxX);

        assert(first <= last);
        increment += (scaleFactor - 1) * static_cast<uint32_t>(last - first);
    }

    if (minY != maxY) {
        const auto& first = std::ranges::lower_bound(emptyRows, minY);
        const auto& last = std::ranges::upper_bound(emptyRows, maxY);

        assert(first <= last);
        increment += (scaleFactor - 1) * static_cast<uint32_t>(last - first);
    }

    return maxX - minX + maxY - minY + increment;
}

static uint32_t First(
    const std::vector<Coord>& galaxies,
    const std::vector<uint8_t>& emptyRows,
    const std::vector<uint8_t>& emptyColumns
)
{
    uint32_t res = 0;
    for (size_t i = 0; i < galaxies.size() - 1; i++) {
        for (size_t j = i + 1; j < galaxies.size(); j++) {
            res += GetDistance(galaxies[i], galaxies[j], emptyRows, emptyColumns, 2);
        }
    }

    return res;
}

static uint64_t Second(
    const std::vector<Coord>& galaxies,
    const std::vector<uint8_t>& emptyRows,
    const std::vector<uint8_t>& emptyColumns
)
{
    uint64_t res = 0;
    for (size_t i = 0; i < galaxies.size() - 1; i++) {
        for (size_t j = i + 1; j < galaxies.size(); j++) {
            res += GetDistance(galaxies[i], galaxies[j], emptyRows, emptyColumns, 1000000);
        }
    }

    return res;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    std::vector<Coord> galaxies;
    std::vector<uint8_t> emptyRows;
    std::vector<uint8_t> emptyColumns;

    {
        std::ifstream input("input", std::ios::binary);

        std::set<uint8_t> usedColumns;
        for (uint8_t j = 0; j < MAP_LENGTH; j++) {
            bool rowIsEmpty = true;
            for (uint8_t i = 0; i < MAP_LENGTH; i++) {
                if (input.get() == '#') {
                    galaxies.emplace_back(i, j);
                    usedColumns.emplace(i);
                    rowIsEmpty = false;
                }
            }

            if (rowIsEmpty) emptyRows.emplace_back(j);
            input.ignore();
        }

        for (uint8_t i = 0; i < MAP_LENGTH; i++) {
            if (!usedColumns.contains(i))
                emptyColumns.emplace_back(i);
        }
    }

    std::cout << "Primera parte: " << First(galaxies, emptyRows, emptyColumns) << std::endl;
    std::cout << "Segunda parte: " << Second(galaxies, emptyRows, emptyColumns) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
