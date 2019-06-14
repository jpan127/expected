#pragma once

#include <exception>
#include <utility>
#include <iostream>

namespace pstd {

namespace detail {

class bad_optional_access : public std::exception {
  public:
    explicit bad_optional_access(char const* const error) : std::exception(error) {
    }
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

template <typename ErrorType>
class unexpected {
  public:
    constexpr unexpected(ErrorType error) : error_(std::move(error)) {
    }

    ErrorType error_;
};

} // namespace detail

/// @TODO : For now don't worry about if ValueType == ErrorType
template <typename ValueType, typename ErrorType>
class expected {
    using SelfType = expected<ValueType, ErrorType>;

  public:
    static_assert(std::is_default_constructible_v<ValueType>, "Value type must be default constructible");

    constexpr expected() {}

    template <typename ... Args>
    constexpr expected(Args && ... args) : value_(std::forward<Args>(args)...), has_value_(true) {}
    constexpr expected(ValueType value) : value_(std::move(value)), has_value_(true) {}
    constexpr expected(ErrorType error) : error_(std::move(error)), has_value_(false) {}
    constexpr expected(const SelfType &other) : has_value_(other.has_value_) {
        set(other);
    }

    ~expected() {
        if (has_value_) {
            value_.~ValueType();
        } else {
            error_.~decltype(error_)();
        }
    }

    SelfType &operator=(ValueType value) {
        set(std::move(value));
        return *this;
    }

    SelfType &operator=(ErrorType error) {
        set(std::move(error));
        return *this;
    }

    SelfType &operator=(SelfType other) {
        set(other);
        return *this;
    }

    // template <typename ... Args>
    // void emplace(Args && ... args) {
    //     ::new (std::addressof(value_)) ValueType(std::forward<Args>(args)...);
    // }

    /// Check for existence of value
    constexpr operator bool() const noexcept { return has_value(); }
    constexpr bool has_value() const noexcept { return has_value_; }

    /// Get the value
    ValueType &value() {
        detail::throw_exception<detail::bad_optional_access>(!has_value_, "Object does not have a value");
        return value_;
    }
    const ValueType &value() const {
        detail::throw_exception<detail::bad_optional_access>(!has_value_, "Object does not have a value");
        return value_;
    }

    /// Get the error
    ErrorType &error() {
        detail::throw_exception<detail::bad_optional_access>(has_value_, "Object does not have an error");
        return error_.error_;
    }
    const ErrorType &error() const {
        detail::throw_exception<detail::bad_optional_access>(has_value_, "Object does not have an error");
        return error_.error_;
    }

    // ValueType value_or(const ValueType &value) const {
    //     if (has_value_) {
    //         return value_;
    //     }

    //     return value;
    // }

  private:
    union {
        ValueType value_;
        detail::unexpected<ErrorType> error_;
    };

    bool has_value_ = false;

    void set(SelfType &other) {
        if (other.has_value_) {
            set(std::move(other.value_));
        } else {
            set(std::move(other.error_.error_));
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

} // namespace pstd
