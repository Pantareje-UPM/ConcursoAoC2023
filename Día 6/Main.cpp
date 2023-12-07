#include <array>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>

struct Record
{
    uint32_t duration;
    uint32_t distance;
};

uint32_t First(const std::array<Record, 4>& records)
{
    uint32_t res = 1;
    for (const auto& [duration, distance] : records)
    {
        const auto b = static_cast<float>(duration);
        const auto c = -static_cast<float>(distance);

        assert(b * b + 4 * c > 0);
        const float sqrtDiscriminant = std::sqrt(b * b + 4 * c);

        const float fRootA = (b - sqrtDiscriminant) * 0.5f;
        const float fRootB = (b + sqrtDiscriminant) * 0.5f;

        assert(fRootA > 0);
        assert(fRootB > fRootA);

        const auto rootA = static_cast<uint32_t>(std::ceil(fRootA));
        const auto rootB = static_cast<uint32_t>(std::ceil(fRootB));

        res *= rootB - rootA;
    }

    return res;
}

uint64_t Second(const uint64_t joinedDuration, const uint64_t joinedDistance)
{
    const auto b = static_cast<double>(joinedDuration);
    const auto c = -static_cast<double>(joinedDistance);

    const double discriminant = b * b + 4 * c;
    assert(discriminant > 0);

    const double fRootA = (-b + sqrt(discriminant)) * -0.5;
    const double fRootB = (-b + -sqrt(discriminant)) * -0.5;

    assert(fRootA > 0);
    assert(fRootB > fRootA);

    const auto rootA = static_cast<uint64_t>(std::ceil(fRootA));
    const auto rootB = static_cast<uint64_t>(std::ceil(fRootB));

    return rootB - rootA;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    std::array<Record, 4> records;

    uint64_t joinedDuration = 0;
    uint64_t joinedDistance = 0;

    {
        std::ifstream input("input", std::ios::binary);
        constexpr static int pow10[5]{ 1, 10, 100, 1000, 10000 };

        input.ignore(11);
        input >> std::ws;
        for (auto& [duration, _] : records)
        {
            const auto numberStart = input.tellg();
            input >> duration;
            const auto numberEnd = input.tellg();
            input >> std::ws;
            joinedDuration = joinedDuration * pow10[numberEnd - numberStart] + duration;
        }

        input.ignore(11);
        input >> std::ws;
        for (auto& [_, distance] : records)
        {
            const auto numberStart = input.tellg();
            input >> distance;
            const auto numberEnd = input.tellg();
            input >> std::ws;
            joinedDistance = joinedDistance * pow10[numberEnd - numberStart] + distance;
        }
    }


    std::cout << "Primera parte: " << First(records) << std::endl;
    std::cout << "Segunda parte: " << Second(joinedDuration, joinedDistance) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
