/**
    ReactiveX tester code
**/

#include <rxcpp/rx.hpp>
#include <random>
#include <regex>
#include "linmath.h"

namespace Rx {
    using namespace rxcpp;
    using namespace rxcpp::sources;
    using namespace rxcpp::operators;
    using namespace rxcpp::util;
    using namespace rxcpp::subjects;
}

namespace Std {
    using namespace std;
    using namespace std::chrono;
}

class Enemy {
    public:
        const Rx::subject<vec2> position {};

        Enemy() {
            //position.get_observable().
        }
};

int main() {
    Std::random_device rd;   // non-deterministic generator
    Std::mt19937 gen(rd());
    Std::uniform_int_distribution<> dist(4, 18);

    // for testing purposes, produce byte stream that from lines of text
    auto bytes = Rx::range(0, 10) |
        Rx::flat_map([&](int i){
            auto body = Rx::from((uint8_t)('A' + i)) |
                Rx::repeat(dist(gen)) |
                Rx::as_dynamic();
            auto delim = Rx::from((uint8_t)'\r');
            return Rx::from(body, delim) | Rx::concat();
        }) |
        Rx::window(17) |
        Rx::flat_map([](Rx::observable<uint8_t> w){
            return w |
                Rx::reduce(
                    Std::vector<uint8_t>(),
                    [](Std::vector<uint8_t> v, uint8_t b){
                        v.push_back(b);
                        return v;
                    }) |
                Rx::as_dynamic();
        }) |
        Rx::tap([](Std::vector<uint8_t>& v){
            // print input packet of bytes
            copy(v.begin(), v.end(), Std::ostream_iterator<long>(Std::cout, " "));
            Std::cout << Std::endl;
        });

    //
    // recover lines of text from byte stream
    //
    
    auto removespaces = [](Std::string s){
        s.erase(Std::remove_if(s.begin(), s.end(), ::isspace), s.end());
        return s;
    };

    // create strings split on \r
    auto strings = bytes |
        Rx::concat_map([](Std::vector<uint8_t> v){
            Std::string s(v.begin(), v.end());
            Std::regex delim(R"/(\r)/");
            Std::cregex_token_iterator cursor(&s[0], &s[0] + s.size(), delim, {-1, 0});
            Std::cregex_token_iterator end;
            Std::vector<Std::string> splits(cursor, end);
            return Rx::iterate(move(splits));
        }) |
        Rx::filter([](const Std::string& s){
            return !s.empty();
        }) |
        Rx::publish() |
        Rx::ref_count();

    // filter to last string in each line
    auto closes = strings |
        Rx::filter(
            [](const Std::string& s){
                return s.back() == '\r';
            }) |
        Rx::map([](const Std::string&){return 0;});

    // group strings by line
    auto linewindows = strings |
        Rx::window_toggle(closes | Rx::start_with(0), [=](int){return closes;});

    // reduce the strings for a line into one string
    auto lines = linewindows |
        Rx::flat_map([&](Rx::observable<Std::string> w) {
            return w | Rx::start_with<Std::string>("") | Rx::sum() | Rx::map(removespaces);
        });

    // print result
    lines |
        Rx::subscribe<Std::string>(Rx::println(Std::cout));


    Std::cout <<("Hey I'm done!") << Std::endl;
    return 0;
}