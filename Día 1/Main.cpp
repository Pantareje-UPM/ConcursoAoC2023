#include <string>
#include <iostream>
#include <fstream>

// ----      Parte 1      ---- //

void First()
{
    size_t acc = 0;
    std::ifstream input("input");
    std::string line;

    while (!input.eof())
    {
        std::getline(input, line);
        if (line.empty()) continue;

        const auto first = std::ranges::find_if(line.begin(), line.end(), std::isdigit);
        const auto last = std::ranges::find_if(line.rbegin(), line.rend(), std::isdigit);

        acc += (*first - '0') * 10 + (*last - '0');
    }

    std::cout << "Parte 1: " << acc << std::endl;
}

// ----      Parte 2      ---- //

#include <regex>

constexpr size_t EngToInt(const std::string& str)
{
    if (str == "one")   return 1;
    if (str == "two")   return 2;
    if (str == "three") return 3;
    if (str == "four")  return 4;
    if (str == "five")  return 5;
    if (str == "six")   return 6;
    if (str == "seven") return 7;
    if (str == "eight") return 8;
    if (str == "nine")  return 9;

    return str[0] - '0';
}

void Second()
{
    size_t acc = 0;
    std::ifstream input("input");
    std::string line;

    while (!input.eof())
    {
        std::getline(input, line);
        if (line.empty()) continue;

        std::match_results<std::string::iterator> firstMatch;
        std::match_results<std::string::reverse_iterator> lastMatch;

        const static std::regex regexDirect("[1-9]|(one)|(two)|(three)|(four)|(five)|(six)|(seven)|(eight)|(nine)");
        const static std::regex regexReverse("[1-9]|(eno)|(owt)|(eerht)|(ruof)|(evif)|(xis)|(neves)|(thgie)|(enin)");

        std::regex_search(line.begin(), line.end(), firstMatch, regexDirect);
        std::regex_search(line.rbegin(), line.rend(), lastMatch, regexReverse);

        auto first = firstMatch.str();
        auto last = lastMatch.str();
        std::ranges::reverse(last.begin(), last.end());

        acc += EngToInt(first) * 10 + EngToInt(last);
    }

    std::cout << "Parte 2: " << acc << std::endl;
}

int main()
{
    First();
    Second();
}
