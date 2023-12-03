#include <algorithm>
#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>

struct CubeSet
{
    uint8_t redCount;
    uint8_t greenCount;
    uint8_t blueCount;

    explicit CubeSet(std::istream& is)
        : redCount(0), greenCount(0), blueCount(0)
    {
        do
        {
            is.ignore(2);

            size_t count;
            is >> count;
            is.ignore(1);

            switch (is.peek())
            {
            case 'r':
                redCount += static_cast<uint8_t>(count);
                is.ignore(3);
                break;

            case 'g':
                greenCount += static_cast<uint8_t>(count);
                is.ignore(5);
                break;

            case'b':
                blueCount += static_cast<uint8_t>(count);
                is.ignore(4);
                break;

            default:
                const std::string message = "Letra desconocida: " + std::to_string(is.peek());
                throw std::exception(message.c_str());
            }
        }
        while (is && is.peek() == ',');
    }

    [[nodiscard]] bool IsValid(uint8_t maxRed, uint8_t maxGreen, uint8_t maxBlue) const
    {
        return redCount <= maxRed && greenCount <= maxGreen && blueCount <= maxBlue;
    }
};

class Game
{
    uint32_t gameId;
    std::vector<CubeSet> cubeSets;

public:
    explicit Game(std::istream& is)
    {
        is >> std::ws;
        is.ignore(5);

        is >> gameId;

        cubeSets.emplace_back(is);

        while (is && is.peek() == ';')
            cubeSets.emplace_back(is);
    }

    [[nodiscard]] bool IsValid(uint8_t maxRed, uint8_t maxGreen, uint8_t maxBlue) const
    {
        return std::ranges::all_of(
            cubeSets,
            [maxRed, maxGreen, maxBlue](const CubeSet& cubeSet)
            {
                return cubeSet.IsValid(maxRed, maxGreen, maxBlue);
            }
        );
    }

    [[nodiscard]] uint32_t GameId() const noexcept { return gameId; }

    [[nodiscard]] uint32_t MaxRed() const
    {
        uint32_t value = 0;
        for (const auto& cubeSet : cubeSets)
            if (cubeSet.redCount > value) value = cubeSet.redCount;

        return value;
    }

    [[nodiscard]] uint32_t MaxGreen() const
    {
        uint32_t value = 0;
        for (const auto& cubeSet : cubeSets)
            if (cubeSet.greenCount > value) value = cubeSet.greenCount;

        return value;
    }

    [[nodiscard]] uint32_t MaxBlue() const
    {
        uint32_t value = 0;
        for (const auto& cubeSet : cubeSets)
            if (cubeSet.blueCount > value) value = cubeSet.blueCount;

        return value;
    }
};

uint32_t First(const std::vector<Game>& games)
{
    uint32_t acc = 0;
    for (const auto& game : games)
        if (game.IsValid(12, 13, 14)) acc += game.GameId();

    return acc;
}

uint32_t Second(const std::vector<Game>& games)
{
    uint32_t acc = 0;
    for (const auto& game : games)
        acc += game.MaxRed() * game.MaxGreen() * game.MaxBlue();

    return acc;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();
    std::ifstream input("input");

    std::vector<Game> games;
    while (!input.eof())
    {
        games.emplace_back(input);
        input >> std::ws;
    }

    std::cout << "Parte 1: " << First(games) << std::endl;
    std::cout << "Parte 1: " << Second(games) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::clog << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
