#pragma once

#include <algorithm>

namespace mediaelch {

/// _Very_ simplistic optional type. Requires that `T` is default constructible.
template<typename T>
class Optional
{
public:
    Optional() = default;

    /*implicit*/ Optional(T val) : value(std::move(val)), m_hasValue{true} {};

    bool hasValue() const { return m_hasValue; }

    /**
     * Returns a copy of the stored value or the alternative if not available.
     */
    T getOrValue(T&& alternativeValue) { return m_hasValue ? value : std::forward<T>(alternativeValue); }

    T value{};

private:
    bool m_hasValue{false};
};

} // namespace mediaelch
