#include <array>
#include <vector>
#include <string>
#include <numeric>
#include <ranges>
#include <execution>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>

#define USE_SMALL_VEC 1

constexpr size_t SMALL_VEC_SIZE = 5;

template <typename T>
class SmallVec
{
    std::array<T, SMALL_VEC_SIZE> m_buf;
    size_t m_size;

public:
    SmallVec() : m_size(0) {}

    const T& operator[](size_t i) const noexcept
    {
        assert(i < m_size);
        return m_buf[i];
    }

    T& operator[](size_t i) noexcept
    {
        assert(i < m_size);
        return m_buf[i];
    }

    [[nodiscard]] size_t size() const noexcept { return m_size; }

    // «emplace_back» en la biblioteca estándar devuelve una referencia.
    // Sin embargo, no la necesitamos en este caso.
    template <typename... Args>
    void emplace_back(Args&&... args)
    {
        assert(m_size < SMALL_VEC_SIZE);
        new (m_buf.data() + m_size++) T(std::forward<Args>(args)...);
    }

    void erase(size_t pos) noexcept
    {
        assert(pos < m_size);
        for (size_t i = pos; i < m_size - 1; i++)
            m_buf[i] = m_buf[i + 1];

        m_size--;
    }
};

inline uint8_t Hash(std::string_view sv)
{
    uint8_t res = 0;
    for (const char c : sv)
    {
        res += c;
        res *= 17;
    }

    return res;
}

static uint32_t First(const std::vector<std::string>& values)
{
    const auto hashedValuesView = values | std::ranges::views::transform([](const std::string& s) { return (uint32_t)Hash(s); });
    return std::reduce(std::execution::par_unseq, hashedValuesView.begin(), hashedValuesView.end());
}

static uint32_t Second(const std::vector<std::string>& values)
{
    // Usar un vector de tamaño máximo fijo evita recrearlo innecesariamente y se puede crear
    // en la pila. Sin embargo, suele requerir un uso de memoria adicional comparado con vectores
    // dinámicos de tamaños similares. Es importante hacer pruebas para comprobar cuál es más veloz.

#if USE_SMALL_VEC
    typedef SmallVec<std::pair<std::string, uint8_t>> VecType;
#else
    typedef std::vector<std::pair<std::string, uint8_t>> VecType;
#endif

    std::array<VecType, 256> boxes;

    for (const auto& value : values)
    {
        if (value.ends_with('-'))
        {
            const std::string label = value.substr(0, value.length() - 1);
            const uint8_t hash = Hash(label);
            auto& box = boxes[hash];

#if USE_SMALL_VEC
            for (size_t i = 0; i < box.size(); i++)
                if (box[i].first == label) { box.erase(i); break; }
#else
            for (auto it = box.begin(); it != box.end(); ++it)
                if (it->first == label) { box.erase(it); break; }
#endif
        }
        else
        {
            const std::string label = value.substr(0, value.length() - 2);
            const uint8_t val = value.back() - '0';

            const uint8_t hash = Hash(label);
            auto& box = boxes[hash];

            size_t i = 0;
            for (; i < box.size(); i++)
            {
                if (box[i].first == label)
                {
                    box[i].second = val;
                    break;
                }
            }

            if (i == box.size())
                box.emplace_back(label, val);
        }
    }

    uint32_t res = 0;
    for (uint32_t i = 0; i < boxes.size(); i++)
    {
        const auto& box = boxes[i];
        for (uint32_t j = 0; j < box.size(); j++)
            res += (i + 1) * (j + 1) * box[j].second;
    }

    return res;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    std::vector<std::string> values;
    {
        std::ifstream input("input", std::ios::binary);

        std::string value;
        while(!input.eof())
        {
            std::getline(input, value, ',');
            values.emplace_back(value);
        }
    }

    std::cout << "Primera parte: " << First(values) << std::endl;
    std::cout << "Segunda parte: " << Second(values) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
