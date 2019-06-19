#include "catch.hpp"

#include "expected.h"

#include <string>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#pragma GCC diagnostic ignored "-Wunused-function"

namespace {

struct Data {
    int value = 5;
};

constexpr bool operator==(const Data &a, const Data &b) { return a.value == b.value; }
constexpr bool operator!=(const Data &a, const Data &b) { return a.value != b.value; }
constexpr bool operator<(const Data &a, const Data &b)  { return a.value <  b.value; }
constexpr bool operator<=(const Data &a, const Data &b) { return a.value <= b.value; }
constexpr bool operator>(const Data &a, const Data &b)  { return a.value >  b.value; }
constexpr bool operator>=(const Data &a, const Data &b) { return a.value >= b.value; }

enum class Error {
    Bad,
    VeryBad,
    Terrible,
};

using Expected = pstd::expected<Data, Error>;

static_assert(sizeof(Expected) == sizeof(Error) + 4);
static_assert(pstd::detail::is_comparable_v<Data>);
static_assert(pstd::detail::is_comparable_v<Error>);

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
        ~V() noexcept = default;
    };

    struct E {
        int x=0, y=0, z=0;
        E() noexcept = default;
        E(int x, int y, int z) noexcept : x(x), y(y), z(z){}
        ~E() noexcept = default;
    };

    using Type = pstd::expected<V, E>;

    constexpr int kX = 123;
    constexpr int kY = 456;
    constexpr int kZ = 789;

    SECTION("Expected Constructor") {
        Type v(Type::in_place{}, kX, kY, kZ);
        REQUIRE(has_value(v));
        REQUIRE(v.value().x == kX);
        REQUIRE(v.value().y == kY);
        REQUIRE(v.value().z == kZ);
    }
    SECTION("Unexpected Constructor") {
        Type e(Type::unexpect{}, kX, kY, kZ);
        REQUIRE(!has_value(e));
        REQUIRE(e.error().x == kX);
        REQUIRE(e.error().y == kY);
        REQUIRE(e.error().z == kZ);
    }
    SECTION("Expected Reconstruction") {
        Type v;
        v.emplace(Type::in_place{}, kX, kY, kZ);
        REQUIRE(has_value(v));
        REQUIRE(v.value().x == kX);
        REQUIRE(v.value().y == kY);
        REQUIRE(v.value().z == kZ);
    }
    SECTION("Unexpected Reconstruction") {
        Type e;
        e.emplace(Type::unexpect{}, kX, kY, kZ);
        REQUIRE(!has_value(e));
        REQUIRE(e.error().x == kX);
        REQUIRE(e.error().y == kY);
        REQUIRE(e.error().z == kZ);
    }
    SECTION("Alternating") {
        // Value
        Type e(Type::in_place{}, kX, kY, kZ);
        REQUIRE(has_value(e));
        REQUIRE(e.value().x == kX);
        REQUIRE(e.value().y == kY);
        REQUIRE(e.value().z == kZ);
        // Error
        e.emplace(Type::unexpect{}, kX, kY, kZ);
        REQUIRE(!has_value(e));
        REQUIRE(e.error().x == kX);
        REQUIRE(e.error().y == kY);
        REQUIRE(e.error().z == kZ);
        // Value
        e.emplace(Type::in_place{}, kX, kY, kZ);
        REQUIRE(has_value(e));
        REQUIRE(e.value().x == kX);
        REQUIRE(e.value().y == kY);
        REQUIRE(e.value().z == kZ);
        // Error
        e.emplace(Type::unexpect{}, kX, kY, kZ);
        REQUIRE(!has_value(e));
        REQUIRE(e.error().x == kX);
        REQUIRE(e.error().y == kY);
        REQUIRE(e.error().z == kZ);
    }
}

TEST_CASE("Factory", "expected") {
    constexpr auto x = pstd::make_unexpected<Error>(Error::Terrible);
    static_assert(std::is_same_v<decltype(x)::Type, Error>);
    REQUIRE(x.value() == Error::Terrible);

    Expected e = pstd::make_unexpected<Error>(Error::Terrible);
    REQUIRE(e.error() == Error::Terrible);
}

TEST_CASE("Dereference", "expected") {
    constexpr int kValue = 127127;
    constexpr int kOtherValue = 888888;
    Expected e = Data{.value = kValue};
    REQUIRE(!exception_thrown([&e] { *e; }));
    REQUIRE((*e).value == kValue);
    REQUIRE(e->value == kValue);
    e->value = kOtherValue;
    REQUIRE(e->value == kOtherValue);

    e = Error{};
    REQUIRE(exception_thrown([&e] { *e; }));
    REQUIRE(exception_thrown([&e] { e->value = 5; }));
}

TEST_CASE("Alternatives", "expected") {
    SECTION("Has Value") {
        Expected e = Data{.value = 127};
        const auto value = e.value_or(Data{.value = 721});
        REQUIRE(value.value == 127);
    }
    SECTION("Alternative Value") {
        Expected e = Error::Bad;
        const auto value = e.value_or(Data{.value = 721});
        REQUIRE(value.value == 721);
    }
    SECTION("Has Error") {
        Expected e = Error::Bad;
        const auto value = e.error_or(Error::Terrible);
        REQUIRE(value == Error::Bad);
    }
    SECTION("Alternative Error") {
        Expected e = Data{.value = 127};
        const auto value = e.error_or(Error::Terrible);
        REQUIRE(value == Error::Terrible);
    }
}

TEST_CASE("Comparison", "expected") {
    SECTION("Unexpected") {
        using Unexpected = pstd::unexpected<Error>;

        // Equality
        {
            const Unexpected u1{Error::Bad};
            const Unexpected u2{Error::Bad};
            REQUIRE(u1 == u2);
        }
        {
            const Unexpected u1{Error::VeryBad};
            const Unexpected u2{Error::VeryBad};
            REQUIRE(u1 == u2);
        }
        {
            const Unexpected u1{Error::Terrible};
            const Unexpected u2{Error::Terrible};
            REQUIRE(u1 == u2);
        }
        {
            const Unexpected u1{Error::Bad};
            const Unexpected u2{Error::Terrible};
            REQUIRE(u1 != u2);
        }

        // Comparison
        {
            const Unexpected u1{Error::Bad};
            const Unexpected u2{Error::VeryBad};
            const Unexpected u3{Error::Terrible};

            REQUIRE(u1 < u2);
            REQUIRE(u2 < u3);
            REQUIRE(u1 < u3);

            REQUIRE(u1 <= u2);
            REQUIRE(u2 <= u3);
            REQUIRE(u1 <= u3);

            REQUIRE(u2 <= u2);
            REQUIRE(u3 <= u3);
            REQUIRE(u3 <= u3);

            REQUIRE(u2 > u1);
            REQUIRE(u3 > u2);
            REQUIRE(u3 > u1);

            REQUIRE(u2 >= u1);
            REQUIRE(u3 >= u2);
            REQUIRE(u3 >= u1);

            REQUIRE(u2 >= u2);
            REQUIRE(u3 >= u3);
            REQUIRE(u3 >= u3);
        }
    }
    SECTION("Expected") {
        // Equality
        {
            const Expected u1{Error::Bad};
            const Expected u2{Error::Bad};
            REQUIRE(u1 == u2);
        }
        {
            const Expected u1{Error::VeryBad};
            const Expected u2{Error::VeryBad};
            REQUIRE(u1 == u2);
        }
        {
            const Expected u1{Error::Terrible};
            const Expected u2{Error::Terrible};
            REQUIRE(u1 == u2);
        }
        {
            const Expected u1{Error::Bad};
            const Expected u2{Error::Terrible};
            REQUIRE(u1 != u2);
        }

        // Comparison
        {
            const Expected u1{Error::Bad};
            const Expected u2{Error::VeryBad};
            const Expected u3{Error::Terrible};

            REQUIRE(u1 < u2);
            REQUIRE(u2 < u3);
            REQUIRE(u1 < u3);

            REQUIRE(u1 <= u2);
            REQUIRE(u2 <= u3);
            REQUIRE(u1 <= u3);

            REQUIRE(u2 <= u2);
            REQUIRE(u3 <= u3);
            REQUIRE(u3 <= u3);

            REQUIRE(u2 > u1);
            REQUIRE(u3 > u2);
            REQUIRE(u3 > u1);

            REQUIRE(u2 >= u1);
            REQUIRE(u3 >= u2);
            REQUIRE(u3 >= u1);

            REQUIRE(u2 >= u2);
            REQUIRE(u3 >= u3);
            REQUIRE(u3 >= u3);
        }
    }
}

TEST_CASE("Swap", "expected") {
    SECTION("Expected") {
        constexpr int kValueA = 777'777;
        constexpr int kValueB = 555'555;
        Expected a(Data{.value = kValueA});
        Expected b(Data{.value = kValueB});
        REQUIRE(a->value == kValueA);
        REQUIRE(b->value == kValueB);
        pstd::swap(a, b);
        REQUIRE(a->value == kValueB);
        REQUIRE(b->value == kValueA);
    }
    SECTION("Unexpected") {
        constexpr Error kValueA = Error::VeryBad;
        constexpr Error kValueB = Error::Terrible;
        Expected a(kValueA);
        Expected b(kValueB);
        REQUIRE(a.error() == kValueA);
        REQUIRE(b.error() == kValueB);
        pstd::swap(a, b);
        REQUIRE(a.error() == kValueB);
        REQUIRE(b.error() == kValueA);
    }
}

} // namespace

#pragma GCC diagnostic pop
