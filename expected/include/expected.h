#pragma once

#include <exception>
#include <iostream>
#include <type_traits>
#include <utility>

namespace pstd {

namespace detail {

class bad_optional_access : public std::exception {
  public:
    explicit bad_optional_access(const char* const error) : error_(error) {
    }

    virtual const char *what() const noexcept override {
        return error_;
    }

  private:
    const char* const error_;
};

template <typename ExceptionType>
void throw_exception(__attribute__((unused)) const bool condition,
                     __attribute__((unused)) const char *error) {
#ifndef PSTD_EXPECTED_DISABLE_EXCEPTIONS
    if (condition) {
        throw ExceptionType(error);
    }
#endif
}

template <typename A, typename B = A, typename = std::void_t<>>
struct is_equality_comparable : std::false_type {};

template <typename A, typename B>
struct is_equality_comparable<A, B,
    std::void_t<decltype(std::declval<A>() == std::declval<B>())>>
    : std::true_type {};

template <typename A, typename B = A>
constexpr bool is_equality_comparable_v = is_equality_comparable<A, B>::value;

template <typename A, typename B = A, typename = std::void_t<>>
struct is_comparable : std::false_type {};

template <typename A, typename B>
struct is_comparable<A, B,
    std::void_t<decltype(std::declval<A>() < std::declval<B>())>> : std::true_type {};

template <typename A, typename B = A>
constexpr bool is_comparable_v = is_comparable<A, B>::value;

} // namespace detail

/// Represents an "unexpected" object or the E / Error of an [expected] object
template <typename ErrorType,
            std::enable_if_t<!std::is_reference_v<ErrorType>> * = nullptr,
            std::enable_if_t<!std::is_void_v<ErrorType>> * = nullptr>
class unexpected {
  public:
    using Type = ErrorType;
    using SelfType = unexpected<ErrorType>;

    /// Disable default constructor to prevent implicit conversions
    unexpected() = delete;

    /// Member copy constructor
    constexpr explicit unexpected(const ErrorType &error) : error_(error) {}
    constexpr explicit unexpected(ErrorType &&error) : error_(std::move(error)) {}

    /// Perfect forwarding constructor
    template <typename ... Args>
    constexpr unexpected(Args && ... args) : error_(std::forward<Args>(args)...) {}

    /// Allow copy, move, assignment
    constexpr unexpected(const SelfType &other) = default;
    constexpr unexpected(SelfType &&other) = default;
    constexpr SelfType &operator=(const SelfType &other) = default;

    constexpr ErrorType &value() { return error_; }
    constexpr const ErrorType &value() const { return error_; }

  private:
    /// Value of this object
    ErrorType error_;
};

template <typename ValueType, typename ErrorType,
            std::enable_if_t<std::is_nothrow_constructible_v<ErrorType>> * = nullptr>
class expected {
    using SelfType = expected<ValueType, ErrorType>;

  public:
    static_assert(std::is_default_constructible_v<ValueType>, "Value type must be default constructible");

    /// Tag to specify in place construction of [ValueType]
    struct in_place {};

    /// Tag to specify in place construction of [ErrorType]
    struct unexpect {};

    /// Must be implemented, otherwise default constructor is always implicitly deleted
    constexpr expected() {}

    /// Copy constructor
    constexpr expected(const SelfType &other) : has_value_(other.has_value_) {
        set(other);
    }

    /// [unexpected] copy constructor
    template <typename E = ErrorType,
                std::enable_if_t<std::is_same_v<E, ErrorType>> * = nullptr,
                std::enable_if_t<std::is_copy_constructible_v<E>> * = nullptr>
    constexpr expected(const unexpected<ErrorType> &error) : error_(error), has_value_(false){}

    /// [ValueType] move constructor
    template <typename V = ValueType,
                std::enable_if_t<std::is_same_v<V, ValueType>> * = nullptr,
                std::enable_if_t<std::is_move_constructible_v<V>> * = nullptr>
    constexpr expected(V &&value) : value_(std::move(value)), has_value_(true) {}

    /// [ValueType] copy constructor
    template <typename V = ValueType,
                std::enable_if_t<std::is_same_v<V, ValueType>> * = nullptr,
                std::enable_if_t<std::is_copy_constructible_v<V>> * = nullptr>
    constexpr expected(const V &value) : value_(value), has_value_(true) {}

    /// [ErrorType] move constructor
    template <typename E = ErrorType,
                std::enable_if_t<std::is_same_v<E, ErrorType>> * = nullptr,
                std::enable_if_t<std::is_move_constructible_v<E>> * = nullptr>
    constexpr expected(E &&error) : error_(std::move(error)), has_value_(false) {}

    /// [ErrorType] copy constructor
    template <typename E = ErrorType,
                std::enable_if_t<std::is_same_v<E, ErrorType>> * = nullptr,
                std::enable_if_t<std::is_copy_constructible_v<E>> * = nullptr>
    constexpr expected(const E &error) : error_(error), has_value_(false) {}

    /// [ValueType] perfect forwarding constructor
    template <typename ... Args,
                typename V = ValueType,
                std::enable_if_t<std::is_nothrow_constructible_v<V, Args && ...>> * = nullptr>
    constexpr expected(in_place, Args && ... args) : value_(std::forward<Args>(args)...), has_value_(true) {}

    /// [ErrorType] perfect forwarding constructor
    template <typename ... Args,
                typename E = ErrorType,
                std::enable_if_t<std::is_nothrow_constructible_v<E, Args && ...>> * = nullptr>
    constexpr expected(unexpect, Args && ... args) : error_(std::forward<Args>(args)...), has_value_(false) {}

    /// Destructor
    ~expected() noexcept {
        if (has_value_) {
            value_.~ValueType();
        } else {
            error_.~decltype(error_)();
        }
    }

    /// [ValueType] copy assignment operator
    SelfType &operator=(ValueType value) {
        set(std::move(value));
        return *this;
    }

    /// [ErrorType] copy assignment operator
    SelfType &operator=(ErrorType error) {
        set(std::move(error));
        return *this;
    }

    /// Copy assignment operator
    SelfType &operator=(SelfType other) {
        set(other);
        return *this;
    }

    [[nodiscard]] constexpr ValueType &operator*() {
        detail::throw_exception<detail::bad_optional_access>(!has_value_, "Object does not have a value");
        return value_;
    }
    [[nodiscard]] constexpr const ValueType &operator*() const {
        detail::throw_exception<detail::bad_optional_access>(!has_value_, "Object does not have a value");
        return value_;
    }

    /// Check for existence of value
    [[nodiscard]] constexpr operator bool() const noexcept { return has_value(); }
    [[nodiscard]] constexpr bool has_value() const noexcept { return has_value_; }

    /// Get the value
    [[nodiscard]] ValueType &value() {
        detail::throw_exception<detail::bad_optional_access>(!has_value_, "Object does not have a value");
        return value_;
    }
    [[nodiscard]] const ValueType &value() const {
        detail::throw_exception<detail::bad_optional_access>(!has_value_, "Object does not have a value");
        return value_;
    }

    /// Get the error
    [[nodiscard]] ErrorType &error() {
        detail::throw_exception<detail::bad_optional_access>(has_value_, "Object does not have an error");
        return error_.value();
    }
    [[nodiscard]] const ErrorType &error() const {
        detail::throw_exception<detail::bad_optional_access>(has_value_, "Object does not have an error");
        return error_.value();
    }

    [[nodiscard]] constexpr ValueType value_or(ValueType &&alternative) const noexcept {
        return (has_value_) ? (value_) : (std::move(alternative));
    }

    [[nodiscard]] constexpr ValueType value_or(const ValueType &alternative) const noexcept {
        return (has_value_) ? (value_) : (alternative);
    }

    [[nodiscard]] constexpr ErrorType error_or(ErrorType &&alternative) const noexcept {
        return (!has_value_) ? (error_.value()) : (std::move(alternative));
    }

    [[nodiscard]] constexpr ErrorType error_or(const ErrorType &alternative) const noexcept {
        return (!has_value_) ? (error_.value()) : (alternative);
    }

  private:
    union {
        ValueType value_;
        unexpected<ErrorType> error_;
    };

    bool has_value_ = false;

    void set(SelfType &other) {
        if (other.has_value_) {
            set(std::move(other.value_));
        } else {
            set(std::move(other.error_.value()));
        }
    }

    void set(ErrorType error) {
        if (has_value_) {
            // Destruct value
            value_.~ValueType();
            // Set error
            ::new (std::addressof(error_)) ErrorType(std::move(error));
            has_value_ = false;
        } else {
            error_ = std::move(error);
        }
    }

    void set(ValueType value) {
        if (has_value_) {
            value_ = std::move(value);
        } else {
            // Destruct error
            error_.~decltype(error_)();
            // Set value
            ::new (std::addressof(value_)) ValueType(std::move(value));
            has_value_ = true;
        }
    }
};

template <typename ErrorType, typename ... Args>
constexpr unexpected<ErrorType> make_unexpected(Args && ... args) {
    using Type = unexpected<ErrorType>;
    return Type{std::forward<Args>(args)...};
}

template <typename ErrorType,
            std::enable_if_t<detail::is_equality_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator==(const unexpected<ErrorType> &a,
                          const unexpected<ErrorType> &b) {
    return (a.value() == b.value());
}

template <typename ErrorType,
            std::enable_if_t<detail::is_equality_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator!=(const unexpected<ErrorType> &a,
                          const unexpected<ErrorType> &b) {
    return !(operator==(a, b));
}

template <typename ErrorType,
            std::enable_if_t<detail::is_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator<(const unexpected<ErrorType> &a,
                         const unexpected<ErrorType> &b) {
    return (a.value() < b.value());
}

template <typename ErrorType,
            std::enable_if_t<detail::is_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator<=(const unexpected<ErrorType> &a,
                          const unexpected<ErrorType> &b) {
    return (operator<(a, b)) || (operator==(a, b));
}

template <typename ErrorType,
            std::enable_if_t<detail::is_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator>(const unexpected<ErrorType> &a,
                         const unexpected<ErrorType> &b) {
    return !(operator<(a, b));
}

template <typename ErrorType,
            std::enable_if_t<detail::is_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator>=(const unexpected<ErrorType> &a,
                          const unexpected<ErrorType> &b) {
    return (operator>(a, b)) || (operator==(a, b));
}

template <typename ValueType, typename ErrorType,
            std::enable_if_t<detail::is_equality_comparable_v<ValueType> &&
                             detail::is_equality_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator==(const expected<ValueType, ErrorType> &a,
                          const expected<ValueType, ErrorType> &b) {
    if (a.has_value() != b.has_value()) {
        return false;
    } else if (a.has_value()) {
        return (a.value() == b.value());
    } else {
        return (a.error() == b.error());
    }
}

template <typename ValueType, typename ErrorType,
            std::enable_if_t<detail::is_equality_comparable_v<ValueType> &&
                             detail::is_equality_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator!=(const expected<ValueType, ErrorType> &a,
                          const expected<ValueType, ErrorType> &b) {
    return !(operator==(a, b));
}

template <typename ValueType, typename ErrorType,
            std::enable_if_t<detail::is_comparable_v<ValueType> &&
                             detail::is_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator<(const expected<ValueType, ErrorType> &a,
                         const expected<ValueType, ErrorType> &b) {
    const bool a_has_value = a.has_value();
    const bool b_has_value = b.has_value();
    if (a_has_value && b_has_value) {
        return (a.value() < b.value());
    } else if (a_has_value && !b_has_value) {
        return true;
    } else if (!a_has_value && b_has_value) {
        return false;
    } else {
        return (a.error() < b.error());
    }
}

template <typename ValueType, typename ErrorType,
            std::enable_if_t<detail::is_comparable_v<ValueType> &&
                             detail::is_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator<=(const expected<ValueType, ErrorType> &a,
                          const expected<ValueType, ErrorType> &b) {
    return (operator<(a, b)) || (operator==(a, b));
}

template <typename ValueType, typename ErrorType,
            std::enable_if_t<detail::is_comparable_v<ValueType> &&
                             detail::is_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator>(const expected<ValueType, ErrorType> &a,
                         const expected<ValueType, ErrorType> &b) {
    return !(operator<(a, b));
}

template <typename ValueType, typename ErrorType,
            std::enable_if_t<detail::is_comparable_v<ValueType> &&
                             detail::is_comparable_v<ErrorType>> * = nullptr>
constexpr bool operator>=(const expected<ValueType, ErrorType> &a,
                          const expected<ValueType, ErrorType> &b) {
    return (operator>(a, b)) || (operator==(a, b));
}

} // namespace pstd
