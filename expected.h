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
    unexpected(ErrorType error) : error_(std::move(error)) {
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

    explicit expected() = default;

    template <typename ... Args>
    explicit constexpr expected(Args && ... args) : state_(std::forward<Args>(args)...), has_value_(true) {
        // std::cout << "expected(Args && ... args)\n";
    }
    explicit constexpr expected(ValueType &&value) : state_(std::move(value)), has_value_(true) {
        // std::cout << "expected(ValueType &&value)\n";
    }
    explicit constexpr expected(const ValueType &value) : state_(value), has_value_(true) {
        // std::cout << "expected(const ValueType &value)\n";
    }
    explicit constexpr expected(ErrorType &&error) : state_(std::move(error)), has_value_(false) {
        // std::cout << "expected(ErrorType &&error)\n";
    }
    explicit constexpr expected(const ErrorType &error) : state_(error), has_value_(false) {
        // std::cout << "expected(const ErrorType &error)\n";
    }

    explicit constexpr expected(const SelfType &other) : has_value_(other.has_value_) {
        set(other);
    }
    explicit constexpr expected(SelfType &&other) : has_value_(other.has_value_) {
        set(other);
    }

    ~expected() {
        if (has_value_) {
            state_.value_.~ValueType();
        } else {
            state_.error_.~decltype(state_.error_)();
        }
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, SelfType>, int> = 0>
    SelfType &operator=(const T &other) {
        std::cout << "SelfType\n";
        return *this;
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, ValueType>, int> = 0>
    SelfType &operator=(const T &other) {
        std::cout << "ValueType\n";
        return *this;
    }

    template <typename T, std::enable_if_t<std::is_same_v<T, ErrorType>, int> = 0>
    SelfType &operator=(const T &other) {
        std::cout << "ErrorType\n";
        return *this;
    }

    template <typename T, std::enable_if_t<
        !std::is_same_v<T, SelfType> &&
        !std::is_same_v<T, ValueType> &&
        !std::is_same_v<T, ErrorType>
        , int> = 0>
    SelfType &operator=(const T &other) {
        std::cout << "other\n";
        return *this;
    }

    // template <typename T, std::enable_if_t<std::is_same<T, SelfType>{}, SelfType> = 0>
    // SelfType &operator=(T &&other) {
    //     return *this;
    // }

    SelfType &operator=(const SelfType &other) {
        std::cout << "non-template\n";
        set(other);
        return *this;
    }

//     SelfType &operator=(SelfType &&other) {
//         set(other);
//         return *this;
//     }

    // SelfType &operator=(ValueType &&value) {
    //     std::cout << "bye\n";
    //     set(std::move(value));
    //     return *this;
    // }

    // SelfType &operator=(ErrorType &&error) {
    //     std::cout << "hi\n";
    //     set(std::move(error));
    //     return *this;
    // }

//     // template <typename T>
//     SelfType &operator=(void *a) {
//         std::cout << "bye\n";
//         return *this;
//     }

    // template <typename ... Args>
    // void emplace(Args && ... args) {
    //     ::new (std::addressof(value_)) ValueType(std::forward<Args>(args)...);
    // }

    /// Check for existence of value
    operator bool() const noexcept { return has_value(); }
    bool has_value() const noexcept { return has_value_; }

    /// Get the value
    ValueType &value() {
        detail::throw_exception<detail::bad_optional_access>(!has_value_, "Object does not have a value");
        return state_.value_;
    }
    const ValueType &value() const {
        detail::throw_exception<detail::bad_optional_access>(!has_value_, "Object does not have a value");
        return state_.value_;
    }

    /// Get the error
    ErrorType &error() {
        detail::throw_exception<detail::bad_optional_access>(has_value_, "Object does not have an error");
        return state_.error_.error_;
    }
    const ErrorType &error() const {
        detail::throw_exception<detail::bad_optional_access>(has_value_, "Object does not have an error");
        return state_.error_;
    }

    // ValueType value_or(const ValueType &value) const {
    //     if (has_value_) {
    //         return value_;
    //     }

    //     return value;
    // }

  private:
    union State {
        State() : value_{} {}
        State(ValueType &&v) : value_(v) {}
        State(ErrorType &&v) : error_(v) {}
        ~State() {}
        ValueType value_;
        detail::unexpected<ErrorType> error_;
    };

    bool has_value_ = true;
    State state_;

    void set(const SelfType &other) {
        if (other.has_value_) {
            set(std::move(other.state_.value_));
        } else {
            set(std::move(other.state_.error_.error_));
        }
    }

    void set(ErrorType error) {
        if (has_value_) {
            // Destruct value
            state_.value_.~ValueType();
            // Set error
            ::new (std::addressof(state_.error_)) ErrorType(std::move(error));
            has_value_ = false;
        } else {
            state_.error_ = std::move(error);
        }
    }

    void set(ValueType value) {
        if (has_value_) {
            state_.value_ = std::move(value);
        } else {
            // Destruct error
            state_.error_.~decltype(state_.error_)();
            // Set value
            ::new (std::addressof(state_.value_)) ValueType(std::move(value));
            has_value_ = true;
        }
    }
};

} // namespace pstd
