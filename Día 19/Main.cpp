#include <array>
#include <unordered_map>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>

enum class Rating : uint16_t { X, M, A, S };
enum class RuleKind : uint16_t { REDIRECT, ACCEPT, REJECT };

typedef std::array<char, 4> NameBuf;

struct Part { uint16_t x, m, a, s; };

struct PartRange : std::array<std::pair<uint16_t, uint16_t>, 4>
{
    uint64_t GetCount() const noexcept
    {
        uint64_t res = 1;
        for (const auto& a : *this)
        {
            assert(a.second >= a.first);
            res *= a.second - a.first;
        }
        return res;
    }
};

struct Rule
{
    Rating rating : 2;
    uint16_t isConditionLower : 1;
    uint16_t value : 13;
    RuleKind kind;

    NameBuf workflowRef;

    [[nodiscard]] bool AcceptsPart(Part p) const noexcept
    {
        bool accepts;
        switch (rating)
        {
        case Rating::X: accepts = isConditionLower ? p.x < value : p.x > value; break;
        case Rating::M: accepts = isConditionLower ? p.m < value : p.m > value; break;
        case Rating::A: accepts = isConditionLower ? p.a < value : p.a > value; break;
        case Rating::S: accepts = isConditionLower ? p.s < value : p.s > value; break;
        default: assert(false);
        }
        return accepts;
    }
};

struct Workflow
{
    std::vector<Rule> rules;
};

uint64_t First(
    const std::vector<Part>& parts,
    const std::unordered_map<std::string, Workflow>& workflows
)
{
    uint64_t res = 0;

    for (const auto& part : parts)
    {
        Workflow workflow = workflows.at("in");

        RuleKind kind = RuleKind::REDIRECT;
        while (kind == RuleKind::REDIRECT)
        {
            for (const auto& rule : workflow.rules)
            {
                if (rule.AcceptsPart(part))
                {
                    if (rule.kind == RuleKind::REDIRECT)
                        workflow = workflows.at(rule.workflowRef.data());
                    else
                        kind = rule.kind;
                    break;
                }
            }
        }

        if (kind == RuleKind::ACCEPT) res += static_cast<uint64_t>(part.x) + part.m + part.a + part.s;
    }

    return res;
}

uint64_t CountAccepted(
    const PartRange& partRange,
    const Workflow& workflow,
    const std::unordered_map<std::string, Workflow>& workflows
)
{
    PartRange remaining = partRange;
    uint64_t accepted = 0;

    const auto GetCount = [&workflows](const PartRange& range, const Rule& rule) -> uint64_t {
        switch (rule.kind)
        {
        case RuleKind::ACCEPT:
            return range.GetCount();
        case RuleKind::REDIRECT:
            return CountAccepted(range, workflows.at(rule.workflowRef.data()), workflows);
        default:
            return 0;
        }
    };

    for (const auto& rule : workflow.rules)
    {
        size_t i;

        switch (rule.rating)
        {
        case Rating::X: i = 0; break;
        case Rating::M: i = 1; break;
        case Rating::A: i = 2; break;
        case Rating::S: i = 3; break;
        default: assert(false);
        }

        assert(remaining[i].first != remaining[i].second);

        PartRange acceptedRange = remaining;
        if (rule.isConditionLower)
        {
            if (remaining[i].first >= rule.value)
                continue;

            if (acceptedRange[i].second < rule.value) {
                accepted += GetCount(acceptedRange, rule);
                break;
            }
            else
            {
                acceptedRange[i].second = rule.value;
                accepted += GetCount(acceptedRange, rule);
                remaining[i].first = rule.value;
            }
        }
        else
        {
            if (remaining[i].second <= rule.value)
                continue;

            if (acceptedRange[i].first > rule.value) {
                accepted += GetCount(acceptedRange, rule);
                break;
            }
            else
            {
                acceptedRange[i].first = rule.value + 1;
                accepted += GetCount(acceptedRange, rule);
                remaining[i].second = rule.value + 1;
            }
        }
    }

    return accepted;
}

uint64_t Second(const std::unordered_map<std::string, Workflow>& workflows)
{
    constexpr auto range = std::make_pair<uint16_t, uint16_t>(1, 4001);
    PartRange partRange = { range, range, range, range };
    return CountAccepted(partRange, workflows.at("in"), workflows);
}

namespace
{
    std::unordered_map<std::string, Workflow> g_workflows;
    std::vector<Part> g_parts;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();

    {
        std::ifstream input("input", std::ios::binary);

        // Leemos los distintos flujos de trabajo.
        std::string line;
        while (std::getline(input, line) && !line.empty())
        {
            const size_t nameEnd = line.find('{');
            assert(nameEnd != std::string::npos);

            std::string name = line.substr(0, nameEnd);
            std::string_view rulesDefinitions(line.begin() + nameEnd + 1, line.end() - 1);
            assert(rulesDefinitions.size() > 0);

            std::vector<Rule> rules;

            size_t startPos = 0;
            size_t splitPos = rulesDefinitions.find_first_of(',');
            while (startPos != std::string::npos)
            {
                std::string_view def = rulesDefinitions.substr(startPos, splitPos - startPos);
                assert(def.length() > 0);

                if (def.length() == 1)
                {
                    switch (def[0])
                    {
                    case 'A': rules.emplace_back(Rating::A, (uint16_t)(0), (uint16_t)(0), RuleKind::ACCEPT); break;
                    case 'R': rules.emplace_back(Rating::A, (uint16_t)(0), (uint16_t)(0), RuleKind::REJECT); break;

                    default:
                        rules.emplace_back(
                            Rating::A,
                            (uint16_t)(0), (uint16_t)(0),
                            RuleKind::REDIRECT,
                            NameBuf{ def[0] }
                        );
                        break;
                    }
                }
                else if (def[1] != '<' && def[1] != '>')
                {
                    assert(def.length() <= 3);
                    NameBuf workflowNameBuf = {};
                    std::ranges::copy(def, workflowNameBuf.data());
                    rules.emplace_back(Rating::A, (uint16_t)(0), (uint16_t)(0), RuleKind::REDIRECT, workflowNameBuf);
                }
                else
                {
                    Rating rating;
                    switch (def[0])
                    {
                    case 'x': rating = Rating::X; break;
                    case 'm': rating = Rating::M; break;
                    case 'a': rating = Rating::A; break;
                    case 's': rating = Rating::S; break;
                    default: assert(false);
                    }

                    uint16_t isConditionLower = def[1] == '<';
                    size_t read;
                    long lValue = std::stoul(def.data() + 2, &read);
                    assert(lValue > 0 && lValue <= 4'000);

                    uint16_t value = static_cast<uint16_t>(lValue);

                    std::string_view workflowName = def.substr(3 + read);
                    assert(workflowName.length() > 0);

                    if (workflowName.length() == 1)
                    {
                        switch (workflowName[0])
                        {
                        case 'A':
                            rules.emplace_back(rating, isConditionLower, value, RuleKind::ACCEPT);
                            break;
                        case 'R':
                            rules.emplace_back(rating, isConditionLower, value, RuleKind::REJECT);
                            break;

                        default:
                            rules.emplace_back(
                                rating, isConditionLower, value, RuleKind::REDIRECT,
                                NameBuf{ workflowName[0] }
                            );
                            break;
                        }
                    }
                    else
                    {
                        assert(workflowName.length() <= 3);
                        NameBuf workflowNameBuf = {};
                        std::ranges::copy(workflowName, workflowNameBuf.data());
                        rules.emplace_back(rating, isConditionLower, value, RuleKind::REDIRECT, workflowNameBuf);
                    }
                }

                if (splitPos != std::string::npos)
                {
                    startPos = splitPos + 1;
                    splitPos = rulesDefinitions.find_first_of(',', splitPos + 1);
                }
                else
                {
                    startPos = std::string::npos;
                }
            }

            g_workflows[name] = { rules };
        }

        // Leemos las piezas iniciales.
        while (!input.eof())
        {
            int16_t x, m, a, s;
            input.ignore(3);
            input >> x;
            input.ignore(3);
            input >> m;
            input.ignore(3);
            input >> a;
            input.ignore(3);
            input >> s;

            g_parts.emplace_back(x, m, a, s);

            input.ignore();
            input >> std::ws;
        }
    }

    std::cout << "Primera parte: " << First(g_parts, g_workflows) << std::endl;
    std::cout << "Segunda parte: " << Second(g_workflows) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}
