#include <unordered_set>
#include <chrono>
#include <iostream>
#include <fstream>


#pragma region Común
// ----------------------------------------- COMÚN ----------------------------------------- //
/*
 * La única información que necesitamos por boleto es la cantidad de veces que un número de
 * boleto ha resultado ganador y la cantidad de copias que tenemos de ese boleto (necesario
 * para la  parte 2, lo iniciamos a una copia). Además, todos los boletos tienen la misma
 * cantidad de posibles números ganadores y de números de boleto.
 */

constexpr size_t WINNING_NUMBER_COUNT = 10;
constexpr size_t CARD_NUMBER_COUNT = 25;

struct Card
{
    uint32_t matches;
    uint32_t repetitions = 1;
};

/**
 * \brief Crea un vector con los boletos del problema, calculando la
 * cantidad de números ganadores del boleto y el número de copias.
 * \return El vector con los boletos.
 */
std::vector<Card> LoadCards()
{
    std::ifstream input("input", std::ios::binary);

    // Creamos el vector con los boletos y su puntuación.
    std::vector<Card> cards;
    while (!input.eof())
    {
        input.ignore(10);
        std::unordered_set<uint32_t> winningNumbers;
        for (size_t _ = 0; _ < WINNING_NUMBER_COUNT; _++)
        {
            uint32_t winningNumber;
            input >> winningNumber >> std::ws;
            winningNumbers.emplace(winningNumber);
        }

        // Calculamos la cantidad de números del boleto que son ganadores.
        uint32_t matches = 0;

        input.ignore(2);
        for (size_t _ = 0; _ < CARD_NUMBER_COUNT; _++)
        {
            uint32_t cardNumber;
            input >> cardNumber >> std::ws;
            if (winningNumbers.contains(cardNumber))
                matches += 1;
        }

        cards.emplace_back(static_cast<uint8_t>(matches));
    }

    // Calculamos el número de copias de cada boleto.
    for (size_t i = 0; i < cards.size(); i++)
    {
        const auto& [matches, repetitions] = cards[i];
        for (int32_t j = 0; j < static_cast<int32_t>(matches); j++)
            cards[i + 1 + j].repetitions += repetitions;
    }

    return cards;
}

#pragma endregion


#pragma region Parte 1
// ---------------------------------------- PARTE 1 ---------------------------------------- //
/*
 * La primera parte es trivial. Calculamos la puntuación de cada boleto y la acumulamos.
 */

uint32_t First(const std::vector<Card>& cards)
{
    uint32_t res = 0;

    // El primer número ganador vale un punto. Cada número ganador tras ese duplica la
    // puntuación. Duplicar en binario es equivalente a correr los bits a la izquierda.
    // La puntuación equivale a que el bit en la posición «matches - 1» esté activo,
    // siendo «0» la posición inicial y «matches» la cantidad de números ganadores.
    // Si no hay números ganadores la puntuación es cero.
    for (const auto& [matches, _] : cards)
        res += static_cast<uint32_t>(1) << (matches - 1);

    return res;
}

#pragma endregion


#pragma region Parte 2
// ---------------------------------------- PARTE 2 ---------------------------------------- //
/*
 * La segunda parte también es trivial. Solamente tenemos que acumular el número de copias.
 */

uint32_t Second(std::vector<Card>& cards)
{
    uint32_t res = 0;
    for (const auto& [_, repetitions] : cards)
        res += repetitions;

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

    auto cards = LoadCards();

    std::cout << "Primera parte: " << First(cards) << std::endl;
    std::cout << "Segunda parte: " << Second(cards) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::clog << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}

#pragma endregion
