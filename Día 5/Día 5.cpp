#include <algorithm>
#include <vector>
#include <array>
#include <set>
#include <cassert>
#include <fstream>
#include <iostream>
#include <optional>

struct Node
{
    uint32_t toStart;
    uint32_t fromStart;
    uint32_t length;

    [[nodiscard]] bool IsBeforeOrEqual(const uint32_t a) const noexcept
    {
        return fromStart <= a;
    }

    [[nodiscard]] bool Contains(const uint32_t a) const noexcept
    {
        assert(IsBeforeOrEqual(a));
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

template<typename T>
struct Range
{
    T start;
    T length;

    friend bool operator<(const Range& a, const Range& b) noexcept
    {
        return a.start < b.start;
    }

    friend bool operator>(const Range& a, const Range& b) noexcept
    {
        return a.start > b.start;
    }
};

struct Almanac
{
    std::vector<uint32_t> seeds;

    std::vector<Range<uint32_t>> seedRanges;
    std::array<std::set<Node>, 7> maps;
};

uint32_t First(const Almanac& almanac)
{
    std::vector<uint32_t> setA(almanac.seeds);
    std::vector<uint32_t> setB;

    std::vector<uint32_t>* from = &setA;
    std::vector<uint32_t>* to = &setB;

    for (const auto& map : almanac.maps)
    {
        auto node = map.begin();
        std::optional seed = std::make_optional(from->front());

        while (seed.has_value() && node != map.end())
        {
            if (node->IsBeforeOrEqual(*seed))
            {
                if (node->Contains(*seed))
                {
                    to->emplace_back(node->Translate(*seed));
                    std::ranges::push_heap(*to, std::greater{});

                    std::ranges::pop_heap(*from, std::greater{});
                    from->resize(from->size() - 1);

                    seed = !from->empty() ? std::make_optional(from->front()) : std::nullopt;
                }
                else
                {
                    ++node;
                }
            }
            else
            {
                to->emplace_back(*seed);
                std::ranges::push_heap(*to, std::greater{});

                std::ranges::pop_heap(*from, std::greater{});
                from->resize(from->size() - 1);

                seed = !from->empty() ? std::make_optional(from->front()) : std::nullopt;
            }
        }

        while (seed.has_value())
        {
            to->emplace_back(*seed);
            std::ranges::push_heap(*to, std::greater{});

            std::ranges::pop_heap(*from, std::greater{});
            from->resize(from->size() - 1);

            seed = !from->empty() ? std::make_optional(from->front()) : std::nullopt;
        }

        std::vector<uint32_t>* tmp = from;
        from = to;
        to = tmp;
    }

    return from->front();
}

uint32_t Second(const Almanac& almanac)
{
    using RangeHeap = std::vector<Range<uint32_t>>;

    RangeHeap setA(almanac.seedRanges);
    RangeHeap setB;

    RangeHeap* from = &setA;
    RangeHeap* to = &setB;

    for (const auto& map : almanac.maps)
    {
        auto node = map.begin();
        std::optional seedRange = std::make_optional(from->front());

        while (seedRange.has_value() && node != map.end())
        {
            const auto seedStart = seedRange->start;
            const auto nodeStart = node->fromStart;

            const auto seedEnd = seedRange->start + seedRange->length;
            const auto nodeEnd = node->fromStart + node->length;

            if (nodeStart < seedEnd)
            {
                if (seedStart < nodeEnd)
                {
                    if (seedStart < nodeStart)
                    {
                        to->emplace_back(seedStart, nodeStart - seedStart);
                        std::ranges::push_heap(*to, std::greater{});
                    }

                    const auto firstToTranslate = std::max(nodeStart, seedStart);
                    const auto lastToTranslate = std::min(nodeEnd, seedEnd);

                    to->emplace_back(node->Translate(firstToTranslate), lastToTranslate - firstToTranslate);
                    std::ranges::push_heap(*to, std::greater{});

                    if (seedEnd > nodeEnd)
                    {
                        std::ranges::pop_heap(*from, std::greater{});
                        from->resize(from->size() - 1);

                        from->emplace_back(nodeEnd, seedEnd - nodeEnd);
                        std::ranges::push_heap(*from, std::greater{});

                        seedRange = !from->empty() ? std::make_optional(from->front()) : std::nullopt;
                    }
                    else
                    {
                        std::ranges::pop_heap(*from, std::greater{});
                        from->resize(from->size() - 1);

                        seedRange = !from->empty() ? std::make_optional(from->front()) : std::nullopt;
                    }
                }
                else
                {
                    ++node;
                }
            }
            else
            {
                to->emplace_back(*seedRange);
                std::ranges::push_heap(*to, std::greater{});

                std::ranges::pop_heap(*from, std::greater{});
                from->resize(from->size() - 1);

                seedRange = !from->empty() ? std::make_optional(from->front()) : std::nullopt;
            }
        }

        while (seedRange.has_value())
        {
            to->emplace_back(*seedRange);
            std::ranges::push_heap(*to, std::greater{});

            std::ranges::pop_heap(*from, std::greater{});
            from->resize(from->size() - 1);

            seedRange = !from->empty() ? std::make_optional(from->front()) : std::nullopt;
        }

        RangeHeap* tmp = from;
        from = to;
        to = tmp;
    }

    return from->front().start;
}

int main()
{
    std::ifstream input("input", std::ios::binary);

    Almanac almanac;

    input.ignore(7);
    while (std::isdigit(input.peek()))
    {
        uint32_t seedStart, seedLength;
        input >> seedStart >> seedLength >> std::ws;

        almanac.seeds.emplace_back(seedStart);
        almanac.seeds.emplace_back(seedLength);

        almanac.seedRanges.emplace_back(seedStart, seedLength);
    }

    std::ranges::make_heap(almanac.seeds, std::greater{});
    std::ranges::make_heap(almanac.seedRanges, std::greater{});

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
    std::cout << Second(almanac) << std::endl;
}
