#include "catch.hpp"


#include <string>

// namespace {

struct Data {
    int value = 5;
};

enum class Error {
    Bad,
    VeryBad,
    Terrible,
};

#include "expected.h"

using Expected = pstd::expected<Data, Error>;

template <typename F>
bool exception_thrown(F &&f) {
    try {
        f();
        return false;
    } catch (const pstd::detail::bad_optional_access &e) {
        return true;
    }
}

bool has_value(const Expected &e) {
    return (e.has_value())                          &&
           (e)                                      &&
           (!exception_thrown([&e] { e.value(); })) &&
           (exception_thrown([&e] { e.error(); }));
}

TEST_CASE("Typical", "expected") {
    SECTION("DefaultConstruction") {
        SECTION("Mutable") {
            Expected e;
            REQUIRE(!has_value(e));
        }
        SECTION("Immutable") {
            const Expected e;
            REQUIRE(!has_value(e));
        }
    }
}

TEST_CASE("Assignment", "expected") {
    SECTION("AssignmentAfterConstruction") {
        SECTION("Expected") {
            constexpr int kValue = 99123;
            Expected e;
            e = Data{.value = kValue};
            REQUIRE(has_value(e));
            REQUIRE(e.value().value == kValue);
        }
        SECTION("Unexpected") {
            constexpr Error kError = Error::Terrible;
            Expected e;
            e = kError;
            REQUIRE(!has_value(e));
            REQUIRE(e.error() == kError);
        }
        SECTION("EmptyBraces") {
            // Start with no value
            Expected e;
            e = {};
            REQUIRE(!has_value(e));

            // Give it value
            e = Data{};
            REQUIRE(has_value(e));

            // Check it resets to no value
            e = {};
            REQUIRE(!has_value(e));
        }
    }
    SECTION("AssignmentOnConstruction") {
        SECTION("Expected") {
            constexpr int kValue = 99123;
            Expected e = Data{.value = kValue};
            REQUIRE(has_value(e));
            REQUIRE(e.value().value == kValue);
        }
    }
}

// } // namespace
