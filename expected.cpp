#include "expected.h"
#include "expected.hpp"
#include <cassert>
#include <iostream>

struct Value {
    uint64_t x;
    bool operator==(const Value &other)  const {
        return x == other.x;
    }
};

using Expected = pstd::expected<Value, std::string>;

Expected transform() {
    return {};
}

int main(int, char **) {
    // {
    //     pstd::expected<std::string, Value> e;
    //     e = {};
    // }
    // {
    //     tl::expected<std::string, Value> e;
    //     e = {};
    // }
    // std::cout << sizeof(e) << std::endl;
    // assert(e.has_value());

    // Error
    Expected e;
    e = std::string("hello");
    // assert(!e.has_value());
    // assert(!e);
    // assert(false == e.error());
    // {
    //     bool exception_triggered = false;
    //     try {
    //         e.value();
    //     } catch (...) {
    //         exception_triggered = true;
    //     }
    //     assert(exception_triggered);
    // }

    // // Value
    e = Value{5};
    // assert(e.has_value());
    // assert(e);
    // assert(Value{5} == e.value());
    // {
    //     bool exception_triggered = false;
    //     try {
    //         e.error();
    //     } catch (...) {
    //         exception_triggered = true;
    //     }
    //     assert(exception_triggered);
    // }

    // e = false;

    // std::cout << "Testing {}\n";
    // e = {};
    // assert(e.has_value());
    // assert(e);

    e = {};
    e = Expected{};
    std::cout << e.has_value() << std::endl;

    return 0;
}
