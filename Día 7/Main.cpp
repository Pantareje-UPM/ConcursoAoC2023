#include <array>
#include <set>
#include <unordered_map>
#include <ranges>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>

enum class HandKind
{
    HighCard, OnePair, TwoPair, ThreeOfAKind, FullHouse, FourOfAKind, FiveOfAKind
};

struct Hand
{
    std::array<char, 5> cards;
    HandKind kind;
    uint32_t bid;

    friend std::strong_ordering operator<=>(const Hand& a, const Hand& b)
    {
        if (a.kind != b.kind) return a.kind <=> b.kind;
        return a.cards <=> b.cards;
    }

    Hand(const std::array<char, 5>& input, uint32_t bid) : cards(input), bid(bid), kind(HandKind::HighCard) {}
};

class HandWithoutJoker : public Hand
{
    HandKind CalculateHandKind()
    {
        std::unordered_map<char, uint32_t> cardMap;
        for (char card : cards)
            cardMap[card] += 1;

        switch (cardMap.size()) {
        case 5: return HandKind::HighCard;
        case 4: return HandKind::OnePair;
        case 1: return HandKind::FiveOfAKind;

        case 3:
            for (const auto& [_, reps] : cardMap) {
                if (reps == 3)
                    return HandKind::ThreeOfAKind;
            }
            return HandKind::TwoPair;

        case 2:
            if (cardMap.begin()->second == 4 || cardMap.begin()->second == 1)
                return HandKind::FourOfAKind;
            return HandKind::FullHouse;
        }

        throw std::exception();
    }

public:
    explicit HandWithoutJoker(const std::array<char, 5>& input, uint32_t bid) : Hand(input, bid) {
        for (char& c : cards)
        {
            switch (c) {
            case 'A': c = 14; break;
            case 'K': c = 13; break;
            case 'Q': c = 12; break;
            case 'J': c = 11; break;
            case 'T': c = 10; break;
            default:
                assert(std::isdigit(c));
                c -= '0';
                break;
            }
        }

        kind = CalculateHandKind();
    }
};

class HandWithJoker : public Hand
{
    HandKind CalculateHandKind()
    {
        std::unordered_map<char, uint32_t> cardMap;
        uint32_t jokers = 0;

        for (char card : cards)
            if (card != 1) cardMap[card] += 1;
            else jokers++;

        if (jokers == 5) return HandKind::FiveOfAKind;
        uint32_t maxReps = std::ranges::max(cardMap | std::ranges::views::values);

        switch (maxReps + jokers) {
        case 5: return HandKind::FiveOfAKind;
        case 4: return HandKind::FourOfAKind;
        case 1: return HandKind::HighCard;

        case 3:
            if (cardMap.size() == 2) return HandKind::FullHouse;
            return HandKind::ThreeOfAKind;

        case 2:
            if (jokers) return HandKind::OnePair;
            return cardMap.size() == 3 ? HandKind::TwoPair : HandKind::OnePair;
        }

        throw std::exception();
    }

public:
    explicit HandWithJoker(const std::array<char, 5>& input, uint32_t bid) : Hand(input, bid) {
        for (char& c : cards)
        {
            switch (c) {
            case 'A': c = 14; break;
            case 'K': c = 13; break;
            case 'Q': c = 12; break;
            case 'T': c = 10; break;
            case 'J': c = 1; break;
            default:
                assert(std::isdigit(c));
                c -= '0';
                break;
            }
        }

        kind = CalculateHandKind();
    }
};

typedef std::set<HandWithoutJoker, std::greater<Hand>> HandSetWithoutJokers;
typedef std::set<HandWithJoker, std::greater<Hand>> HandSetWithJokers;

static uint64_t First(const HandSetWithoutJokers& hands)
{
    uint32_t res = 0;

    uint32_t i = static_cast<uint32_t>(hands.size());
    for (const auto& hand : hands)
    {
        res = res + hand.bid * i;
        i--;
    }

    return res;
}

static uint64_t Second(const HandSetWithJokers& hands)
{
    uint32_t res = 0;

    uint32_t i = static_cast<uint32_t>(hands.size());
    for (const auto& hand : hands)
    {
        res = res + hand.bid * i;
        i--;
    }

    return res;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    HandSetWithoutJokers handSetWithoutJokers;
    HandSetWithJokers handSetWithJokers;

    {
        std::ifstream input("input", std::ios::binary);

        while (!input.eof())
        {
            std::array<char, 5> cards{};
            uint32_t bid;

            input.read(cards.data(), 5);
            input >> bid >> std::ws;

            handSetWithoutJokers.emplace(cards, bid);
            handSetWithJokers.emplace(cards, bid);
        }
    }

    std::cout << "Primera parte: " << First(handSetWithoutJokers) << std::endl;
    std::cout << "Segunda parte: " << Second(handSetWithJokers) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
