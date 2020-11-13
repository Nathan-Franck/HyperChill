#include <tuple>
#include <iostream>
#include <glm/vec2.hpp>

template<typename T, typename Tuple>
struct tuple_element_index_helper;

template<typename T>
struct tuple_element_index_helper<T, std::tuple<>> {
    static constexpr std::size_t value = 0;
};

template<typename T, typename... Rest>
struct tuple_element_index_helper<T, std::tuple<T, Rest...>> {
    static constexpr std::size_t value = 0;
    using RestTuple = std::tuple<Rest...>;
    static_assert(
            tuple_element_index_helper<T, RestTuple>::value ==
            std::tuple_size_v<RestTuple>,
            "type appears more than once in tuple");
};

template<typename T, typename First, typename... Rest>
struct tuple_element_index_helper<T, std::tuple<First, Rest...>> {
    using RestTuple = std::tuple<Rest...>;
    static constexpr std::size_t value = 1 +
                                         tuple_element_index_helper<T, RestTuple>::value;
};

template<typename T, typename Tuple>
struct tuple_element_index {
    static constexpr std::size_t value =
            tuple_element_index_helper<T, Tuple>::value;
    static_assert(value < std::tuple_size_v<Tuple>,
                  "type does not appear in tuple");
};

template<typename T, typename Tuple>
inline constexpr std::size_t tuple_element_index_v
        = tuple_element_index<T, Tuple>::value;

template<typename T, typename Tuple>
auto get_element(Tuple tuple) {
    constexpr std::size_t index =
            tuple_element_index_v<T, Tuple>;
    return std::get<index>(tuple);
}

template<typename T, typename Tuple>
Tuple set_element(Tuple tuple, T value) {
    constexpr std::size_t index =
            tuple_element_index_v<T, Tuple>;
    std::get<index>(tuple) = value;
    return tuple;
}

constexpr std::size_t index =
        tuple_element_index_v<int, std::tuple<char, int, float>>;

constexpr std::size_t index3 =
        tuple_element_index_v<char, std::tuple<char, int, float>>;

template<typename T>
class Component : public T {
};

class Position : public Component<glm::vec2> {
};

class Velocity : public Component<glm::vec2> {
};

int main() {
    auto entity = std::tuple<Position, Velocity>{Position{glm::vec2{0.0f, 0.0f}},
                                                       Velocity{glm::vec2{2.0f, 1.0f}}};
    entity = set_element<Velocity>(entity, Velocity{glm::vec2{10, 10}});
    
    {
        auto element = get_element<Velocity>(entity);
        auto&[position, velocity] = entity;
        std::cout << element.x << std::endl;
        std::cout << velocity.y << std::endl;
        std::cout << position.x << std::endl;
    }

    std::tuple t{Position{glm::vec2{30, 30}}, Velocity{glm::vec2{-40, -40}}}; // Another C++17 feature: class template argument deduction
    std::apply([&entity](auto&&... args) {((entity = set_element(entity, args)), ...);}, t);

    {
        const auto element = get_element<Velocity>(entity);
        const auto&[position, velocity] = entity;
        std::cout << element.x << std::endl;
        std::cout << velocity.y << std::endl;
        std::cout << position.x << std::endl;
    }
}

// constexpr std::size_t index2 =
//         tuple_element_index_v<int, std::tuple<char, int, int>>;