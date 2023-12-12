#include <array>
#include <queue>
#include <unordered_map>
#include <bit>
#include <algorithm>
#include <cassert>
#include <random>
#include <ranges>
#include <chrono>
#include <fstream>
#include <iostream>

constexpr bool READ_CACHE = true;
constexpr bool CREATE_CACHE = false;
constexpr size_t CACHE_REFINE = 20;
constexpr size_t CACHE_MAX_SIZE = 50'000;

template<typename T, uint8_t MaxSize>
struct SmallVec
{
private:
    typedef std::array<T, MaxSize> Arr;

    uint8_t m_size = 0;
    Arr m_arr;

public:
    void Add(const T& value) noexcept
    {
        assert(m_size < MaxSize);
        m_arr[m_size] = value;
        m_size++;
    }

    [[nodiscard]] size_t Size() const noexcept { return m_size; }
    [[nodiscard]] bool Empty() const noexcept { return m_size == 0; }

    void RemoveFirst() noexcept
    {
        assert(m_size > 0);
        m_size--;
        for (size_t i = 0; i < m_size; i++)
            m_arr[i] = m_arr[i + 1];
    }

    [[nodiscard]] auto begin() noexcept { return m_arr.begin(); }
    [[nodiscard]] auto begin() const noexcept { return m_arr.begin(); }
    [[nodiscard]] auto end() noexcept { return m_arr.begin() + m_size; }
    [[nodiscard]] auto end() const noexcept { return m_arr.begin() + m_size; }

    const T& operator[](size_t i) const noexcept
    {
        assert(i < m_size);
        return m_arr[i];
    }
};

template<uint8_t MaxSize>
using ConditionVec = SmallVec<uint8_t, MaxSize>;

struct Row
{
    std::string springData;
    ConditionVec<6> conditions;
};

namespace
{
    std::unordered_map<std::string, uint64_t> g_prevCalculated;
    [[maybe_unused]] std::unordered_map<std::string, uint64_t> g_timesAccessed;
}

template<uint8_t MaxSize>
std::string ToKey(std::string_view springData, const ConditionVec<MaxSize>& conditions)
{
    std::string key(springData);
    for (const auto& condition : conditions)
    {
        key += "-";
        key += static_cast<char>(condition);
    }

    return key;
}

template<uint8_t MaxSize>
uint64_t CountArrangements(std::string_view _springData, const ConditionVec<MaxSize>& _conditions)
{
    const std::string key = ToKey(_springData, _conditions);
    if (g_prevCalculated.contains(key))
    {
        if constexpr (CREATE_CACHE)
        {
            g_timesAccessed[key] += 10'000 / (g_timesAccessed[key] + 1);
        }

        return g_prevCalculated[key];
    }

    std::string_view springData = _springData;
    ConditionVec conditions = _conditions;

    uint64_t value = 0;
    while (true)
    {
        // Se han cumplido todas las condiciones.
        if (conditions.Empty())
        {
            // Solamente es una disposición válida si no hay más muelles estropeados.
            value = springData.find('#') == std::string::npos;
            break;
        }

        // Si está vacío y quedan grupos, no hay disposiciones posibles.
        if (springData.empty()) break;

        // Limpiamos los «.» iniciales hasta llegar a «#» y «?».
        if (springData[0] == '.')
        {
            springData.remove_prefix(std::min(springData.find_first_not_of('.'), springData.size()));
            // Comprobamos de nuevo que no sea vacío.
            if (springData.empty()) break;
        }

        // El principio debe ser «#» o «?», ya que eliminamos todos los puntos y palabras vacías.

        if (springData[0] == '#')
        {
            if (springData.size() < conditions[0]) break;
            if (springData.find_first_of('.') < conditions[0]) break;

            // El muelle estropeado debe ser cerrado por «.» o «?» en su defecto.
            if (conditions[0] < springData.size())
            {
                if (springData[conditions[0]] == '#') break;
                springData.remove_prefix(conditions[0] + 1);
            }
            else
            {
                springData.remove_prefix(std::min(static_cast<size_t>(conditions[0] + 1), springData.size()));
            }

            // Eliminamos el grupo y el espaciador (si tiene).
            conditions.RemoveFirst();
        }
        else if (springData[0] == '?')
        {
            // Consideramos que puede estar estropeado o no, y acumulamos las disposiciones.

            std::string branch(springData);
            branch[0] = '#';
            value = CountArrangements(branch, conditions);
            branch[0] = '.';
            value += CountArrangements(branch, conditions);
            break;
        }
    }

    g_prevCalculated[key] = value;
    if constexpr (CREATE_CACHE)
    {
        g_timesAccessed[key] = 0;
    }
    return value;
}

uint64_t First(const std::vector<Row>& rows)
{
    uint64_t res = 0;
    for (const auto& [springData, conditions] : rows)
        res += CountArrangements(springData, conditions);

    return res;
}

uint64_t Second(const std::vector<Row>& rows)
{
    uint64_t res = 0;

    for (const auto& [springData, conditions] : rows)
    {
        std::string formattedSpring = springData;
        for (size_t _ = 0; _ < 4; _++)
            formattedSpring += "?" + springData;

        ConditionVec<30> formattedConditions;
        for (size_t _ = 0; _ < 5; _++)
            for (const auto& condition : conditions)
                formattedConditions.Add(condition);

        res += CountArrangements(formattedSpring, formattedConditions);
    }

    return res;
}

template<std::integral T>
constexpr T FromLE(T value)
{
    if constexpr (std::endian::native == std::endian::big)
    {
        auto valueRepresentation = std::bit_cast<std::array<std::byte, sizeof(value)>>(value);
        std::ranges::reverse(valueRepresentation);
        return std::bit_cast<T>(valueRepresentation);
    }

    return value;
}

template<std::integral T>
constexpr T ToLE(T value) { return FromLE(value); }

int Main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    std::vector<Row> rows;
    {
        std::ifstream input("input", std::ios::binary);

        std::string springData;
        while (!input.eof())
        {
            input >> springData >> std::ws;

            ConditionVec<6> conditions;
            while (std::isdigit(input.peek()))
            {
                uint32_t value;
                input >> value;
                input.ignore();

                conditions.Add(static_cast<uint8_t>(value));
            }

            assert(!conditions.Empty());
            rows.emplace_back(springData, conditions);
        }

        if constexpr (READ_CACHE)
        {
            std::ifstream cached("cached", std::ios::binary);
            if (cached)
            {
                while (!cached.eof())
                {
                    uint16_t length;
                    cached.read(reinterpret_cast<char*>(&length), sizeof(length));
                    length = FromLE(length);

                    std::string key;
                    key.resize(length);
                    cached.read(key.data(), length);

                    uint64_t value;
                    cached.read(reinterpret_cast<char*>(&value), sizeof(value));
                    value = FromLE(value);

                    g_prevCalculated[key] = value;
                }
            }
        }
    }

    const auto load = high_resolution_clock::now();
    std::cout << duration_cast<milliseconds>(load - start) << std::endl;

    std::cout << "Primera parte: " << First(rows);
    const auto first = high_resolution_clock::now();
    std::cout << " " << duration_cast<milliseconds>(first - load) << std::endl;


    std::cout << "Segunda parte: " << Second(rows);
    const auto second = high_resolution_clock::now();
    std::cout << " " << duration_cast<milliseconds>(second - first) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;


    return 0;
}

void CreateCache()
{
    struct CacheCreatorEntry
    {
        uint64_t repetitions;
        uint64_t value;
        std::string key;

        bool operator<(const CacheCreatorEntry& o) const noexcept
        {
            return
                300 *   repetitions + 100 *   key.size() / (std::ranges::count(  key, '?') + 1) + value >
                300 * o.repetitions + 100 * o.key.size() / (std::ranges::count(o.key, '?') + 1) + value;
        }
    };

    std::priority_queue<CacheCreatorEntry> queue;
    for (const auto& [key, repetitions] : g_timesAccessed)
        queue.emplace(repetitions, g_prevCalculated[key], key);

    std::ofstream cached("cached", std::ios::binary);

    size_t i = 0;
    for (; i < CACHE_MAX_SIZE && !queue.empty(); i++)
    {
        const auto& key = queue.top().key;
        const auto& value = g_prevCalculated[key];

        assert(key.length() <= UINT16_MAX);
        uint16_t length = ToLE(static_cast<uint16_t>(key.length()));
        cached.write(reinterpret_cast<const char*>(&length), sizeof(length));
        cached.write(key.data(), length);

        // Nos aseguramos de que la representación sea «little endian».
        uint64_t val = ToLE(value);
        cached.write(reinterpret_cast<const char*>(&val), sizeof(val));

        queue.pop();
    }

    std::clog << "Tamaño caché: " << i << std::endl;
}

int main()
{
    if constexpr (CREATE_CACHE)
    {
        std::mt19937 mt(std::random_device {}());
        std::uniform_int_distribution distribution(0, 100);

        (void)std::remove("cached");
        for (size_t i = 0; i < CACHE_REFINE; i++)
        {
            if (const int code = Main(); code != 0) return code;

            std::unordered_map<std::string, std::uint64_t> newMap;
            for (const auto& num : g_prevCalculated)
                if (distribution(mt) < 40 * (int)((float)(i + 1) / CACHE_REFINE)) newMap.emplace(num);

            g_prevCalculated = std::move(newMap);
        }

        CreateCache();
    }
    else
    {
        return Main();
    }
}
