#include <array>
#include <unordered_map>
#include <chrono>
#include <fstream>
#include <iostream>

constexpr size_t DISH_SIZE = 100;
constexpr size_t CYCLE_COUNT = 1000000000;

struct Coord
{
    size_t x, y;
};

enum class Tile : char
{
    EMPTY = '.',
    BLOCK = '#',
    BALL = 'O'
};

typedef std::array<std::array<Tile, DISH_SIZE>, DISH_SIZE> ReflectorDish;

template<>
struct std::hash<ReflectorDish>
{
    size_t operator()(const ReflectorDish& dish) const noexcept
    {
        // En verdad, solamente es necesario considerar las piedras que ruedan, pero esto es más simple de implementar.
        static constexpr std::hash<std::string_view> hasher;
        const std::string_view view(reinterpret_cast<const char*>(dish[0].data()), DISH_SIZE * DISH_SIZE);
        return hasher(view);
    }
};

static uint32_t First(const ReflectorDish& dish)
{
    ReflectorDish rolledDish(dish);

    uint32_t dishLoad = 0;
    for (size_t i = 0; i < DISH_SIZE; i++)
    {
        for (size_t j = 0; j < DISH_SIZE; j++)
        {
            if (rolledDish[i][j] == Tile::BALL)
            {
                size_t y = i;

                while (y > 0 && rolledDish[y - 1][j] == Tile::EMPTY)
                    y--;

                rolledDish[i][j] = Tile::EMPTY;
                rolledDish[y][j] = Tile::BALL;
                dishLoad += static_cast<uint32_t>(DISH_SIZE - y);
            }
        }
    }

    return dishLoad;
}

void RollNorth(ReflectorDish& dish)
{
    for (size_t i = 0; i < DISH_SIZE; i++)
    {
        for (size_t j = 0; j < DISH_SIZE; j++)
        {
            if (dish[i][j] == Tile::BALL)
            {
                size_t y = i;

                while (y > 0 && dish[y - 1][j] == Tile::EMPTY)
                    y--;

                dish[i][j] = Tile::EMPTY;
                dish[y][j] = Tile::BALL;
            }
        }
    }
}

void RollWest(ReflectorDish& dish)
{
    for (size_t j = 0; j < DISH_SIZE; j++)
    {
        for (size_t i = 0; i < DISH_SIZE; i++)
        {
            if (dish[i][j] == Tile::BALL)
            {
                size_t x = j;

                while (x > 0 && dish[i][x - 1] == Tile::EMPTY)
                    x--;

                dish[i][j] = Tile::EMPTY;
                dish[i][x] = Tile::BALL;
            }
        }
    }
}

void RollSouth(ReflectorDish& dish)
{
    for (size_t i = DISH_SIZE - 1; i != SIZE_MAX; i--)
    {
        for (size_t j = 0; j < DISH_SIZE; j++)
        {
            if (dish[i][j] == Tile::BALL)
            {
                size_t y = i;

                while (y < DISH_SIZE - 1 && dish[y + 1][j] == Tile::EMPTY)
                    y++;

                dish[i][j] = Tile::EMPTY;
                dish[y][j] = Tile::BALL;
            }
        }
    }
}

void RollEast(ReflectorDish& dish)
{
    for (size_t j = DISH_SIZE - 1; j != SIZE_MAX; j--)
    {
        for (size_t i = 0; i < DISH_SIZE; i++)
        {
            if (dish[i][j] == Tile::BALL)
            {
                size_t x = j;

                while (x < DISH_SIZE - 1 && dish[i][x + 1] == Tile::EMPTY)
                    x++;

                dish[i][j] = Tile::EMPTY;
                dish[i][x] = Tile::BALL;
            }
        }
    }
}

uint32_t GetLoad(const ReflectorDish& dish)
{
    uint32_t dishLoad = 0;
    for (size_t i = 0; i < DISH_SIZE; i++)
    {
        for (size_t j = 0; j < DISH_SIZE; j++)
        {
            if (dish[i][j] == Tile::BALL)
                dishLoad += static_cast<uint32_t>(DISH_SIZE - i);
        }
    }

    return dishLoad;
}

static uint32_t Second(ReflectorDish& dish)
{
    static constexpr std::hash<ReflectorDish> hasher;

    std::unordered_map<size_t, size_t> previouslyCalculated;
    size_t remaining = 0;

    for (size_t i = 0; i < CYCLE_COUNT; i++)
    {
        RollNorth(dish);
        RollWest(dish);
        RollSouth(dish);
        RollEast(dish);

        size_t key = hasher(dish);
        if (previouslyCalculated.contains(key))
        {
            remaining = (CYCLE_COUNT - previouslyCalculated[key] - 1) % (i - previouslyCalculated[key]);
            break;
        }

        previouslyCalculated[key] = i;
    }

    for (size_t i = 0; i < remaining; i++)
    {
        RollNorth(dish);
        RollWest(dish);
        RollSouth(dish);
        RollEast(dish);
    }

    return GetLoad(dish);
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    ReflectorDish dish;
    {
        std::ifstream input("input", std::ios::binary);

        for (size_t i = 0; i < DISH_SIZE; i++)
        {
            for (size_t j = 0; j < DISH_SIZE; j++)
                dish[i][j] = static_cast<Tile>(input.get());
            input.ignore();
        }
    }

    std::cout << "Primera parte: " << First(dish) << std::endl;
    std::cout << "Segunda parte: " << Second(dish) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
