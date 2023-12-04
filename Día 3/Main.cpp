#include <array>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <fstream>
#include <ranges>


#pragma region Global
namespace
{
    /*
     * El tamaño de el mapa de las piezas es de 140x140. Esto nos permite
     * copiar las líneas directamente a la memoria y con una sola lectura
     * por línea.
     */

    constexpr int32_t DIM = 140;
    std::array<std::array<unsigned char, DIM>, DIM> g_engineMap;
}


/**
 * \brief Devuelve el elemento en dichas coordenadas, o un punto si
 * no está en el rango.
 * \param i la primera componente de la coordenada.
 * \param j la segunda componente de la coordenada.
 */
inline unsigned char GetElementOrDot(const int32_t i, const int32_t j)
{
    if (i < 0 || i >= DIM || j < 0 || j >= DIM) return '.';
    return g_engineMap[i][j];
}

/**
 * \brief Devuelve si el elemento en dichas coordenadas es un engranaje,
 * representado mediante un asterisco. 
 * \param i la primera componente de la coordenada.
 * \param j la segunda componente de la coordenada.
 * \return verdadero si es un engranaje y falso si no lo es o está
 * fuera de rango.
 */
inline bool IsSymbol(const int32_t i, const int32_t j)
{
    const auto c = GetElementOrDot(i, j);
    return c != '.' && !std::isdigit(c);
}

/**
 * \brief Devuelve si el elemento en dichas coordenadas es un símbolo.
 * Los números y puntos no se consideran símbolos.
 * \param i la primera componente de la coordenada.
 * \param j la segunda componente de la coordenada.
 * \return verdadero si es un símbolo y falso si no lo es o está
 * fuera de rango.
 */
inline bool IsGear(const int32_t i, const int32_t j)
{
    return GetElementOrDot(i, j) == '*';
}
#pragma endregion


#pragma region Parte 1
// ---------------------------------------- PARTE 1 ---------------------------------------- //
/*
 * Hay dos soluciones simples al problema planteado: la primera es fijar los símbolos
 * y buscar los números que los rodean y la segunda es fijar los números y buscar si
 * hay algún símbolo con el que esté en contacto.
 *
 * He elegido la segunda opción, ya que evita tener que mirar delante y detrás del número.
 * En vez de eso, sólo hace falta comprobar si está en contacto con algún símbolo y, si es
 * así, simplemente leer el número entero. También evita contar dos veces el mismo número.
 */

uint32_t First()
{
    uint32_t res = 0;

    for (int32_t i = 0; i < DIM; i++)
    {
        for (int32_t j = 0; j < DIM; j++)
        {
            // Si no es el inicio de un número, nos lo saltamos
            if (!std::isdigit(g_engineMap[i][j])) continue;

            bool isTouching = false;
            uint32_t acc = 0;

            // Lee hasta el final del número, estableciendo j al final de este.
            const auto completeNumber = [&]
            {
                // Evitamos realizar una primera comprobación, ya que el elemento
                // actual ha de ser un dígito para llamar a completeNumber.
                do
                {
                    acc = acc * 10 + g_engineMap[i][j] - '0';
                    j++;
                }
                while (j < DIM && std::isdigit(g_engineMap[i][j]));
            };

            // Si un número está en contacto con un símbolo por la izquierda, leemos el número
            // entero y lo aumentamos, ya que cumple el requisito.
            if (IsSymbol(i - 1, j - 1) || IsSymbol(i, j - 1) || IsSymbol(i + 1, j - 1))
            {
                completeNumber();
                isTouching = true;
            }
            else
            {
                // Mientras que el elemento sea un dígito y no se salga del rango,
                // vamos buscando si está en contacto con un símbolo arriba o debajo.
                do
                {
                    if (IsSymbol(i - 1, j) || IsSymbol(i + 1, j))
                    {
                        // Si está en contacto con un símbolo, completamos el número
                        // y terminamos la búsqueda.
                        completeNumber();
                        isTouching = true;
                        break;
                    }

                    // Vamos aumentando el valor acumulado mientras buscamos un símbolo.
                    acc = acc * 10 + g_engineMap[i][j] - '0';
                    j++;
                }
                while (j < DIM && std::isdigit(g_engineMap[i][j]));

                // Si no se ha encontrado un símbolo aún, comprobamos el lado derecho.
                if (!isTouching && (IsSymbol(i - 1, j) || IsSymbol(i, j) || IsSymbol(i + 1, j)))
                    isTouching = true;
            }

            // Si el número está en contacto con un símbolo, acumulamos su valor
            if (isTouching)
                res += acc;
        }
    }

    return res;
}
#pragma endregion


#pragma region Parte 2
// ---------------------------------------- PARTE 2 ---------------------------------------- //
/*
 * Mi solución a la parte 2 es muy similar a la parte 1, aunque algo distinta. Primero creo
 * un mapa donde almacenar los engranajes y los números con los que está en contacto. Luego
 * es sólo cuestión de comprobar qué engranajes están en contacto con exactamente dos números
 * y acumular el producto de estos. Un engranaje sólo necesita dos campos, uno para cada
 * número. El valor de un campo no asignado se establece en -1, y si se intenta añadir otro
 * número a un engranaje que tiene ya 2 asignados, uno de los valores se establece en cero,
 * ya que a la hora de acumular los productos, no participa al ser su producto nulo.
 */

namespace
{
    struct GearRatioNumbers
    {
        int32_t first = -1;
        int32_t second = -1;
    };
    std::unordered_map<int64_t, GearRatioNumbers> g_gearNumbers;
}

uint32_t Second()
{
    for (int32_t i = 0; i < DIM; i++)
    {
        for (int32_t j = 0; j < DIM; j++)
        {
            // Si no es el inicio de un número, nos lo saltamos
            if (!std::isdigit(GetElementOrDot(i, j))) continue;

            // Los elementos son siempre únicos, ya que cada coordenada se comprueba
            // una única vez por número.
            std::vector<int64_t> gearKeys;

            // Si el elemento es un engranaje, lo añade a la lista de engranajes.
            const auto addIfGear = [&](const int32_t x, const int32_t y)
            {
                // El identificador de un engranaje son sus coordenadas, que son únicas.
                if (IsGear(x, y))
                    gearKeys.emplace_back(static_cast<int64_t>(x) << 32 | static_cast<int64_t>(y));
            };

            // Comprobamos el lado izquierdo.

            addIfGear(i - 1, j - 1);
            addIfGear(i, j - 1);
            addIfGear(i + 1, j - 1);

            uint32_t acc = 0;
            for (; j < DIM && std::isdigit(g_engineMap[i][j]); j++)
            {
                // Mientras acumulamos el valor del número, comprobamos
                // si hay engranajes arriba o debajo de la cifra actual.
                addIfGear(i - 1, j);
                addIfGear(i + 1, j);

                acc = acc * 10 + g_engineMap[i][j] - '0';
            }

            // Comprobamos el lado derecho.

            addIfGear(i - 1, j);
            addIfGear(i, j);
            addIfGear(i + 1, j);

            // Añadimos el valor del número al engranaje.
            for (auto& gearKey : gearKeys)
            {
                auto& [first, second] = g_gearNumbers[gearKey];

                if (first == -1) // Ningún adyacente
                    first = static_cast<int32_t>(acc);
                else if (second == -1) // Sólo un adyacente
                    second = static_cast<int32_t>(acc);
                else // Más de dos adyacentes
                    first = 0;
            }
        }
    }

    // Acumulamos el producto de los engranajes.
    uint32_t res = 0;
    for (const auto [first, second] : g_gearNumbers | std::views::values)
    {
        // Si no están ambos asignados, está en contacto con menos de
        // dos números y nos saltamos el engranaje.
        if (first < 0 || second < 0) continue;

        // Agregamos el valor del producto. Si uno de los engranajes
        // está en contacto con más de dos números, el producto es cero,
        // por lo que no afecta al resultado.
        res += first * second;
    }

    return res;
}
#pragma endregion


#pragma region Ejecución
// --------------------------------------- EJECUCIÓN --------------------------------------- //

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    {
        std::ifstream input("input", std::ios::binary);

        for (auto& row : g_engineMap)
        {
            input.read(reinterpret_cast<char*>(row.data()), DIM);
            input >> std::ws;
        }
    }

    std::cout << "Primera parte: " << First() << std::endl;
    std::cout << "Segunda parte: " << Second() << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::clog << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
#pragma endregion
