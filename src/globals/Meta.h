#pragma once

#include <QtGlobal>

#define ELCH_NODISCARD Q_REQUIRED_RESULT
#define ELCH_DEPRECATED Q_DECL_DEPRECATED

// Partially copied from Qt: qtbase/src/corelib/global/qglobal.h
// Qt 5.5 does not support Overload so we have to provide it ourselves.
// This code can be removed when Qt 5.6 is required.
//
// Copyright (C) 2019 The Qt Company Ltd.
// Modified to avoid name clashes.

template<typename... Args>
struct NonConstOverload
{
    template<typename R, typename T>
    constexpr auto operator()(R (T::*ptr)(Args...)) const noexcept -> decltype(ptr)
    {
        return ptr;
    }
    template<typename R, typename T>
    static constexpr auto of(R (T::*ptr)(Args...)) noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template<typename... Args>
struct ConstOverload
{
    template<typename R, typename T>
    constexpr auto operator()(R (T::*ptr)(Args...) const) const noexcept -> decltype(ptr)
    {
        return ptr;
    }
    template<typename R, typename T>
    static constexpr auto of(R (T::*ptr)(Args...) const) noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template<typename... Args>
struct Overload : ConstOverload<Args...>, NonConstOverload<Args...>
{
    using ConstOverload<Args...>::of;
    using ConstOverload<Args...>::operator();
    using NonConstOverload<Args...>::of;
    using NonConstOverload<Args...>::operator();
    template<typename R>
    constexpr auto operator()(R (*ptr)(Args...)) const noexcept -> decltype(ptr)
    {
        return ptr;
    }
    template<typename R>
    static constexpr auto of(R (*ptr)(Args...)) noexcept -> decltype(ptr)
    {
        return ptr;
    }
};

template<typename... Args>
constexpr Overload<Args...> elchOverload = {};
template<typename... Args>
constexpr ConstOverload<Args...> elchConstOverload = {};
template<typename... Args>
constexpr NonConstOverload<Args...> elchNonConstOverload = {};
