#pragma once

#include <QtGlobal>

#define ELCH_NODISCARD Q_REQUIRED_RESULT
#define ELCH_DEPRECATED Q_DECL_DEPRECATED

/// \brief Registers some common types using qRegisterMetaType
/// \details Qt's queued connections require that types are registered using
///          qRegisterMetaType.  Q_DECLARE_METATYPE is not enough.
///          To avoid registering these types multiple times, this function is
///          called once on startup.
void registerAllMetaTypes();

// Partially from qt/QtCore/qglobal.h
// qAsConst was introduced in Qt 5.7, we support 5.6
// std::as_const was introduced in C++17, we support C++14

/// \brief Adds const to non-const objects (like std::as_const)
template<typename T>
constexpr typename std::add_const<T>::type& asConst(T& t) noexcept
{
    return t;
}
// prevent rvalue arguments:
template<typename T>
void asConst(const T&&) = delete;


// Partially copied from Qt: qtbase/src/corelib/global/qglobal.h
// Qt 5.5 does not support Overload so we have to provide it ourselves.
// This code can be removed when Qt 5.7 is required.
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


/// \brief Scope guard for QObject instances. Calls deleteLater() at the end of scope.
template<typename T>
class DeleteLaterScope
{
public:
    DeleteLaterScope(T* ptr) : m_ptr{ptr} {}
    ~DeleteLaterScope() { m_ptr->deleteLater(); }

private:
    T* m_ptr = nullptr;
};

template<typename T>
auto makeDeleteLaterScope(T* ptr)
{
    return DeleteLaterScope<T>(ptr);
}
