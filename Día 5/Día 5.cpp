#include <array>
#include <set>
#include <cassert>
#include <fstream>
#include <iostream>

struct Node
{
    uint32_t toStart;
    uint32_t fromStart;
    uint32_t length;

    [[nodiscard]] bool IsBefore(const uint32_t a) const noexcept
    {
        return fromStart <= a;
    }

    [[nodiscard]] bool Contains(const uint32_t a) const noexcept
    {
        assert(IsBefore(a));
        return a - fromStart < length;
    }

    [[nodiscard]] uint32_t Translate(const uint32_t a) const noexcept
    {
        assert(Contains(a));
        return (a - fromStart) + toStart;
    }

    friend bool operator<(const Node& a, const Node& b) noexcept
    {
        return a.fromStart < b.fromStart;
    }
};

struct Almanac
{
    std::set<uint32_t> seeds;
    std::array<std::set<Node>, 7> maps;
};

uint32_t First(const Almanac& almanac)
{
    std::set<uint32_t> setA(almanac.seeds);
    std::set<uint32_t> setB;

    std::set<uint32_t>* from = &setA;
    std::set<uint32_t>* to = &setB;

    for (const auto& map : almanac.maps)
    {
        auto seed = from->begin();
        auto node = map.begin();

        while (seed != from->end() && node != map.end())
        {
            if (node->IsBefore(*seed))
            {
                if (node->Contains(*seed))
                {
                    to->emplace(node->Translate(*seed));
                    ++seed;
                }
                else
                {
                    ++node;
                }
            }
            else
            {
                to->emplace(*seed);
                ++seed;
            }
        }

        while (seed != from->end())
        {
            to->emplace(*seed);
            ++seed;
        }

        from->clear();

        std::set<uint32_t>* tmp = from;
        from = to;
        to = tmp;
    }

    return *from->begin();
}

int main()
{
    std::ifstream input("input", std::ios::binary);

    Almanac almanac;

    input.ignore(7);
    while (std::isdigit(input.peek()))
    {
        uint32_t seedNumber;
        input >> seedNumber >> std::ws;
        almanac.seeds.emplace(seedNumber);
    }

    for (auto& map : almanac.maps)
    {
        input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        while (std::isdigit(input.peek()))
        {
            uint32_t toStart, fromStart, length;
            input >> toStart >> fromStart >> length >> std::ws;
            map.emplace(toStart, fromStart, length);
        }
    }

    std::cout << First(almanac) << std::endl;
}
