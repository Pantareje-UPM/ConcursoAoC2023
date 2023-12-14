#include <vector>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <ranges>
#include <chrono>
#include <fstream>
#include <iostream>

class ReflectorDish
{
    std::vector<char> m_vec;
    size_t m_width;

public:
    ReflectorDish() noexcept = default;
    ReflectorDish(std::vector<char>&& vec, size_t width) : m_vec(vec), m_width(width) {
        assert(m_vec.size() % m_width == 0);
    }

    size_t Width() const noexcept { return m_width; }
    size_t Height() const noexcept { return m_vec.size() / m_width; }

    char Get(size_t x, size_t y) const noexcept {
        assert(x < m_width);
        return m_vec[y * m_width + x];
    }

    char& Get(size_t x, size_t y) noexcept {
        assert(x < m_width);
        return m_vec[y * m_width + x];
    }

    const char* DataRow(size_t row) const noexcept
    {
        assert(m_width * row < m_vec.size());
        return m_vec.data() + m_width * row;
    }
};

static uint32_t GetMirrorValue(const ReflectorDish& map)
{
    const size_t width = map.Width();
    const size_t height = map.Height();

    const size_t innerRowCount = height - 1;
    const size_t innerColumnCount = width - 1;

    for (size_t i = 0; i < innerRowCount; i++)
    {
        const size_t pairsToCheck = std::min(i + 1, innerRowCount - i);

        bool isMirror = true;
        for (size_t j = 0; isMirror && j < pairsToCheck; j++)
        {
            const char* firstRow = map.DataRow(i - j);
            const char* secondRow = map.DataRow(i + j + 1);

            if (!std::equal(firstRow, firstRow + width, secondRow, secondRow + width))
                isMirror = false;
        }

        if (isMirror) return 100 * static_cast<uint32_t>(i + 1);
    }

    for (size_t i = 0; i < innerColumnCount; i++)
    {
        const size_t pairsToCheck = std::min(i + 1, innerColumnCount - i);

        bool isMirror = true;
        for (size_t j = 0; isMirror && j < pairsToCheck; j++)
        {
            const size_t x1 = i - j;
            const size_t x2 = i + j + 1;

            for (size_t k = 0; isMirror && k < height; k++)
            {
                if (map.Get(x1, k) != map.Get(x2, k))
                    isMirror = false;
            }
        }

        if (isMirror) return static_cast<uint32_t>(i) + 1;
    }

    throw std::exception("No hay espejo.");
}

static uint32_t First(const std::vector<ReflectorDish>& maps)
{
    const auto values = maps | std::views::transform(GetMirrorValue);
    return std::accumulate(values.begin(), values.end(), (uint32_t)0);
}

template<size_t SmudgeCount>
static uint32_t GetMirrorValueSmudges(const ReflectorDish& map)
{
    const size_t width = map.Width();
    const size_t height = map.Height();

    const size_t innerRowCount = height - 1;
    const size_t innerColumnCount = width - 1;

    for (size_t i = 0; i < innerRowCount; i++)
    {
        const size_t pairsToCheck = std::min(i + 1, innerRowCount - i);

        size_t diff = 0;
        for (size_t j = 0; diff <= SmudgeCount && j < pairsToCheck; j++)
        {
            const size_t y1 = i - j;
            const size_t y2 = i + j + 1;

            for (size_t k = 0; diff <= SmudgeCount && k < width; k++)
            {
                if (map.Get(k, y1) != map.Get(k, y2))
                    diff++;
            }
        }

        if (diff == SmudgeCount) return 100 * static_cast<uint32_t>(i + 1);
    }

    for (size_t i = 0; i < innerColumnCount; i++)
    {
        const size_t pairsToCheck = std::min(i + 1, innerColumnCount - i);

        size_t diff = 0;
        for (size_t j = 0; diff <= SmudgeCount && j < pairsToCheck; j++)
        {
            const size_t x1 = i - j;
            const size_t x2 = i + j + 1;

            for (size_t k = 0; diff <= SmudgeCount && k < height; k++)
            {
                if (map.Get(x1, k) != map.Get(x2, k))
                    diff++;
            }
        }

        if (diff == SmudgeCount) return static_cast<uint32_t>(i) + 1;
    }

    throw std::exception("No hay espejo.");
}


static uint32_t Second(const std::vector<ReflectorDish>& maps)
{
    const auto values = maps | std::views::transform(GetMirrorValueSmudges<1>);
    return std::accumulate(values.begin(), values.end(), (uint32_t)0);
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();
    std::vector<ReflectorDish> maps;

    {
        std::ifstream input("input", std::ios::binary);

        while (!input.eof())
        {
            size_t width = 0;
            std::vector<char> map;

            char c;
            while ((c = (char)input.get()) != '\n')
            {
                map.emplace_back(c);
                width++;
            }

            while (!input.eof() && (c = (char)input.peek()) != '\n')
            {
                const size_t prevSize = map.size();
                if (map.capacity() < prevSize + width)
                    map.reserve((size_t)((float)map.capacity() * 1.5f));
                map.resize(prevSize + width);

                input.read(map.data() + prevSize, static_cast<std::streamsize>(width));
                input.ignore();
            }
            input.ignore();

            maps.emplace_back(std::move(map), width);
        }
    }

    std::cout << "Primera parte: " << First(maps) << std::endl;
    std::cout << "Segunda parte: " << Second(maps) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
