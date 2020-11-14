#include <tuple>
#include <iostream>
#include <glm/vec2.hpp>
#include <algorithm>
#include <list>

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

template<typename... T>
class Entity {
private:
    std::tuple<T...> data;
public:
    explicit Entity(T... args) : data{ args... } {}

    template<typename Component>
    Component get() {
        constexpr std::size_t index =
            tuple_element_index_v<Component, std::tuple<T...>>;
        return std::get<index>(data);
    }

    template<typename... Component>
    std::tuple<Component...> get_many() {
        return std::make_tuple(get<Component>()...);
    }

    template<typename Component>
    void set(Component component) {
        constexpr std::size_t index =
            tuple_element_index_v<Component, std::tuple<T...>>;
        std::get<index>(data) = component;
    }

    template<typename... Component>
    void set_many(Component... component) {
        (set<Component>(component), ...);
    }
};

template<typename T>
class Component : public T {
};

class Position : public Component<glm::vec2> {
};

class Velocity : public Component<glm::vec2> {
};

template<typename... T>
auto get_position_x(Entity<T...> entity) {
    return entity.get<Position>().x;
}


int main() {

    auto entity = Entity{
        Position{glm::vec2{0.0f, 0.0f}},
        Velocity{glm::vec2{2.0f, 1.0f}},
    };
    entity.set<Velocity>(Velocity{ glm::vec2{10, 10} });

    const auto x = get_position_x(entity);
    std::cout << x << std::endl;
    {
        auto element = entity.get<Velocity>();
        entity.set_many(
            Velocity{ glm::vec2{22.0f, 22.0f} },
            Position{ glm::vec2{244.0f, 44.0f} });
        auto [position, velocity] = entity.get_many<Position, Velocity>();
        std::cout << element.x << std::endl;
        std::cout << velocity.y << std::endl;
        std::cout << position.x << std::endl;
    }

}