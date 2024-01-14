#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>

constexpr uint32_t MAX_STEPS = 10'000'000;

enum class Direction : uint32_t {
    RIGHT, DOWN, LEFT, UP
};

struct Instruction
{
    Direction dir : 2;
    uint32_t steps : 30;
};

uint64_t GetArea(const std::vector<Instruction>& instructions)
{
    // Asumimos la curva definida por las instrucciones siempre como una
    // curva cerrada simple con sentido horario (sentido negativo). Con-
    // sideramos el recinto sobre el eje x e integramos sobre él.
    // 
    // Las integrales en las que dividimos el recinto son triviales, al
    // ser siempre integrales de rectas paralelas a los ejes. También hay
    // que tener en cuenta que el borde no es puntual, por lo que hemos de
    // considerarlo en el cálculo. Al ser una curva cerrada, la distancia
    // que baja es la misma que sube, y lo mismo pasa con el desplazamiento
    // horizontal. Tenemos estos desplazamientos separados el uno del otro
    // ya que usamos sólo líneas horizontales o verticales.

    int64_t res = 0, y = 0;
    uint64_t p = 0;
    for (const auto& ins : instructions)
    {
        p += ins.steps;

        switch (ins.dir)
        {
        // No afecta a la integral, ya que x no cambia.
        case Direction::UP:    y += ins.steps; break;
        case Direction::DOWN:  y -= ins.steps; break;

        // Integramos el valor constante y a lo largo de x.
        case Direction::LEFT:  res -= y * ins.steps; break;
        case Direction::RIGHT: res += y * ins.steps; break;
        }
    }

    assert(res > 0);
    return res + p / 2 + 1;
}

inline uint64_t  First(const std::vector<Instruction>& instructions) { return GetArea(instructions); }
inline uint64_t Second(const std::vector<Instruction>& instructions) { return GetArea(instructions); }

namespace
{
    std::vector<Instruction> g_basicInstructions;
    std::vector<Instruction> g_hexInstructions;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    {
        std::ifstream input("input", std::ios::binary);
        while (!input.eof())
        {
            {
                char directionKey = static_cast<char>(input.get());

                Direction d;
                switch (directionKey)
                {
                case 'U': d = Direction::UP; break;
                case 'D': d = Direction::DOWN; break;
                case 'L': d = Direction::LEFT; break;
                case 'R': d = Direction::RIGHT; break;
                }

                long long steps;
                input >> steps;
                assert(steps > 0 && steps <= MAX_STEPS);

                g_basicInstructions.emplace_back(d, static_cast<uint32_t>(steps));
                input.ignore();
            }

            {
                char hex[9];
                input.read(hex, 9);

                assert(hex[7] >= '0' && hex[7] <= '3');
                Direction d = static_cast<Direction>(hex[7] - '0');

                hex[7] = '\0';
                unsigned long long steps = std::stoull(hex + 2, nullptr, 16);
                assert(steps <= MAX_STEPS);

                g_hexInstructions.emplace_back(d, static_cast<uint32_t>(steps));
            }

            input >> std::ws;
        }
    }

    std::cout << "Primera parte: " << First(g_basicInstructions) << std::endl;
    std::cout << "Segunda parte: " << Second(g_hexInstructions) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
