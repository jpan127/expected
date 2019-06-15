#include "catch.hpp"

#include "expected.h"

#include <string>

namespace {

struct Data {
    int value = 5;
};

enum class Error {
    Bad,
    VeryBad,
    Terrible,
};

using Expected = pstd::expected<Data, Error>;

static_assert(sizeof(Expected) == sizeof(Error) + 4);

template <typename F>
bool exception_thrown(F &&f) {
    try {
        f();
        return false;
    } catch (const pstd::detail::bad_optional_access &e) {
        return true;
    }
}

template <typename V, typename E>
bool has_value(const pstd::expected<V, E> &e) {
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
            const Expected e = Data{.value = kValue};
            REQUIRE(has_value(e));
            REQUIRE(e.value().value == kValue);
        }
        SECTION("Unexpected") {
            constexpr Error kError = Error::Terrible;
            const Expected e = kError;
            REQUIRE(!has_value(e));
            REQUIRE(e.error() == kError);
        }
        SECTION("EmptyBraces") {
            // Start with no value
            Expected e = {};
            REQUIRE(!has_value(e));

            // Give it value
            e = Data{};
            REQUIRE(has_value(e));

            // Check it resets to no value
            e = {};
            REQUIRE(!has_value(e));
        }
    }
}

TEST_CASE("CopyListInitialization", "expected") {
    Expected a = Data{};     REQUIRE( has_value(a));
    Expected b = Error{};    REQUIRE(!has_value(b));
    Expected c = {};         REQUIRE(!has_value(c));
    Expected d = Expected{}; REQUIRE(!has_value(d));
}

TEST_CASE("DirectListInitialization", "expected") {
    Expected e{Data{}};     REQUIRE( has_value(e));
    Expected f{Error{}};    REQUIRE(!has_value(f));
    Expected g{{}};         REQUIRE(!has_value(g));
    Expected h{Expected{}}; REQUIRE(!has_value(h));
}

TEST_CASE("InPlaceConstruction", "expected") {
    struct V {
        int x=0, y=0, z=0;
        V() = default;
        V(int x, int y, int z) noexcept : x(x), y(y), z(z){}
    };

    struct E {
        int x=0, y=0, z=0;
        E(int x, int y, int z) noexcept : x(x), y(y), z(z){}
    };

    using Type = pstd::expected<V, E>;

    constexpr int kX = 123;
    constexpr int kY = 456;
    constexpr int kZ = 789;

    Type v(Type::in_place{}, kX, kY, kZ);
    REQUIRE(has_value(v));
    REQUIRE(v.value().x == kX);
    REQUIRE(v.value().y == kY);
    REQUIRE(v.value().z == kZ);

    Type e(Type::unexpect{}, kX, kY, kZ);
    REQUIRE(!has_value(e));
    REQUIRE(e.error().x == kX);
    REQUIRE(e.error().y == kY);
    REQUIRE(e.error().z == kZ);
}

TEST_CASE("Factory", "expected") {
    constexpr auto x = pstd::make_unexpected<Error>(Error::Terrible);
    static_assert(std::is_same_v<decltype(x)::Type, Error>);
    REQUIRE(x.value() == Error::Terrible);

    Expected e = pstd::make_unexpected<Error>(Error::Terrible);
    REQUIRE(e.error() == Error::Terrible);
}

} // namespace
