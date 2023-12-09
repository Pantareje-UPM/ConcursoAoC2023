#include <array>
#include <vector>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
constexpr size_t DEGREE = 20;
constexpr size_t TERMS = DEGREE + 1;

typedef std::array<int64_t, TERMS> Points;
typedef std::vector<Points> PointsVec;

static int64_t First(const PointsVec& pointsVec)
{
    int64_t res = 0;
    Points pointsBuff;
    for (const auto& points : pointsVec)
    {
        pointsBuff = points;
        size_t size = points.size() - 1;
        while (std::any_of(pointsBuff.begin(), pointsBuff.begin() + size, [](int64_t point) { return point != 0; }))
        {
            for (size_t i = 0; i < size; i++)
                pointsBuff[i] = pointsBuff[i + 1] - pointsBuff[i];

            size--;
        }

        // Nos podemos saltar la primera iteración, ya que siempre es 0
        for (size_t i = size + 1; i < points.size(); i++)
            pointsBuff[i] += pointsBuff[i - 1];

        res += pointsBuff.back();
    }

    return res;
}

static int64_t Second(const PointsVec& pointsVec)
{
    int64_t res = 0;
    Points pointsBuff;
    for (const auto& points : pointsVec)
    {
        pointsBuff = points;
        size_t offset = 0;
        while (std::any_of(pointsBuff.begin() + offset, pointsBuff.end(), [](int64_t point) { return point != 0; }))
        {
            for (size_t i = points.size() - 1; i > offset; i--)
                pointsBuff[i] -= pointsBuff[i - 1];

            offset++;
        }

        // Nos podemos saltar la primera iteración, ya que siempre es 0
        for (size_t i = offset - 1; i + 1 > 0; i--)
            pointsBuff[i] -= pointsBuff[i + 1];

        res += pointsBuff.front();
    }

    return res;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    PointsVec pointsVec;
    {
        std::ifstream input("input", std::ios::binary);

        while (!input.eof())
        {
            Points points;
            for (auto& point : points)
                input >> point >> std::ws;

            pointsVec.emplace_back(points);
        }
    }

    std::cout << "Primera parte: " << First(pointsVec) << std::endl;
    std::cout << "Segunda parte: " << Second(pointsVec) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
