#pragma once

#include <algorithm>

namespace mediaelch {

/// _Very_ simplistic optional type. Requires that `T` is default constructible.
template<typename T>
class Optional
{
public:
    Optional() = default;

    /*implicit*/ Optional(T value) : value(std::move(value)), m_hasValue{true} {};

    bool hasValue() const { return m_hasValue; }

    T value{};

private:
    bool m_hasValue{false};
};

} // namespace mediaelch
