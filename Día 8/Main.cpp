#include <array>
#include <vector>
#include <unordered_map>
#include <numeric>
#include <ranges>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>

constexpr uint32_t MOVEMENTS_LENGTH = 293;

struct Node {
    char name[3];
    char _ = 0;

    static friend bool operator==(const Node& a, const Node& b) noexcept
    {
        return std::bit_cast<uint32_t>(a) == std::bit_cast<uint32_t>(b);
    }
};
static_assert(sizeof(Node) == sizeof(uint32_t));

template<>
struct std::hash<Node>
{
    std::size_t operator()(const Node& node) const noexcept
    {
        return std::hash<uint32_t>{}(std::bit_cast<uint32_t>(node));
    }
};

typedef std::unordered_map<Node, std::pair<Node, Node>> NodeMap;

static uint32_t First(const NodeMap& nodeMap, const std::vector<char>& movements) {
    constexpr Node start{ 'A', 'A', 'A' };
    constexpr Node end { 'Z', 'Z', 'Z' };

    Node currentNode = start;
    std::pair<Node, Node> destinations = nodeMap.at(currentNode);

    uint32_t steps = 0;
    while (steps < 1'000'000) {
        char movement = movements[steps % movements.size()];

        currentNode = movement == 'L' ? destinations.first : destinations.second;
        steps++;

        destinations = nodeMap.at(currentNode);
        if (currentNode == end) return steps;
    };

    throw std::exception();
}

static uint32_t GetDistanceToExit(const Node& start, const NodeMap& nodeMap, const std::vector<char>& movements)
{
    Node currentNode = start;
    std::pair<Node, Node> destinations = nodeMap.at(currentNode);

    uint32_t steps = 0;
    while (steps < 1'000'000) {
        char movement = movements[steps % movements.size()];
        
        currentNode = movement == 'L' ? destinations.first : destinations.second;
        steps++;

        destinations = nodeMap.at(currentNode);
        if (currentNode.name[2] == 'Z') return steps;
    };

    throw std::exception();
}

static uint64_t Second(
    const NodeMap& nodeMap,
    const std::vector<char>& movements,
    const std::vector<Node>& edgeNodes
)
{
    std::vector<uint32_t> distances;
    std::ranges::transform(edgeNodes, std::back_inserter(distances), [&](const Node& node) {
        return GetDistanceToExit(node, nodeMap, movements);
    });

    uint64_t res = 1;
    for (uint32_t distance : distances)
    {
        assert(distance % MOVEMENTS_LENGTH == 0);
        res = std::lcm(res, distance);
    }

    return res;
}

int main()
{
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;
    using std::chrono::high_resolution_clock;

    const auto start = high_resolution_clock::now();
    NodeMap nodeMap;
    std::vector<char> movements;
    std::vector<Node> edgeNodes;

    {
        std::ifstream input("input", std::ios::binary);

        movements.resize(MOVEMENTS_LENGTH);
        input.read(movements.data(), MOVEMENTS_LENGTH);
        input >> std::ws;

        while (!input.eof())
        {
            Node node;
            input.read(node.name, 3);
            input.ignore(4);

            Node left, right;
            input.read(left.name, 3);
            input.ignore(2);
            input.read(right.name, 3);
            input.ignore(2);

            nodeMap[node] = std::make_pair(left, right);

            if (node.name[2] == 'A' || node.name[2] == 'Z')
                edgeNodes.emplace_back(node);
        }
    }

    std::cout << "Primera parte: " << First(nodeMap, movements) << std::endl;
    std::cout << "Segunda parte: " << Second(nodeMap, movements, edgeNodes) << std::endl;

    const auto elapsed = high_resolution_clock::now() - start;
    std::cout << "Duración total: " << duration_cast<milliseconds>(elapsed) << std::endl;
}

