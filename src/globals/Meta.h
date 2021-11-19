#pragma once

#include <QtGlobal>
#include <limits>
#include <stdexcept>
#include <type_traits>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#    define ELCH_QHASH_RETURN_TYPE uint
#    define ELCH_MEDIA_PLAYBACK_STATE QMediaPlayer::State
// most size() and count() functions return int.
using elch_size_t = int;
#else
// With Qt 6, qHash uses size_t
#    define ELCH_QHASH_RETURN_TYPE size_t
#    define ELCH_MEDIA_PLAYBACK_STATE QMediaPlayer::PlaybackState
using elch_size_t = qsizetype;
#endif

#define ELCH_NODISCARD Q_REQUIRED_RESULT
#define ELCH_DEPRECATED Q_DECL_DEPRECATED

#define MediaElch_Expects(x) ((x) ? ((void)0) : throw std::runtime_error("MediaElch precondition failed (expects)"))
#define MediaElch_Ensures(x) ((x) ? ((void)0) : throw std::runtime_error("MediaElch postcondition failed (ensures)"))
#define MediaElch_Assert(x) ((x) ? ((void)0) : throw std::runtime_error("MediaElch assertion failed"))

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


namespace detail {

constexpr bool Signed = true;
constexpr bool Unsigned = false;

template<bool FromIsSigned, bool ToIsSigned, typename From, typename To>
struct int_conversion
{
};

template<typename From, typename To>
struct int_conversion<Signed, Signed, From, To>
{
    static constexpr To convert(From from)
    {
        // TODO: No check if sizeof(To) >= sizeof(From)
        MediaElch_Assert(from <= std::numeric_limits<To>::max());
        MediaElch_Assert(from >= std::numeric_limits<To>::min());
        return static_cast<To>(from);
    }
};

template<typename From, typename To>
struct int_conversion<Unsigned, Unsigned, From, To>
{
    static constexpr To convert(From from)
    {
        // TODO: No check if sizeof(To) >= sizeof(From)
        MediaElch_Assert(from < std::numeric_limits<To>::max());
        return static_cast<To>(from);
    }
};

template<typename From, typename To>
struct int_conversion<Unsigned, Signed, From, To>
{
    static constexpr To convert(From from)
    {
        // TODO: No check if sizeof(To) >= sizeof(From)
        MediaElch_Assert(from <= std::numeric_limits<To>::max());
        return static_cast<To>(from);
    }
};

template<typename From, typename To>
struct int_conversion<Signed, Unsigned, From, To>
{
    static constexpr To convert(From from)
    {
        MediaElch_Assert(from >= 0);
        MediaElch_Assert(from <= std::numeric_limits<To>::max());
        return static_cast<To>(from);
    }
};

} // namespace detail

template<typename To, typename From>
constexpr To safe_int_cast(From from)
{
    static_assert(!std::is_same<From, To>::value, "Unnecessary int cast: Both types are the same");
    static_assert(std::is_integral<From>::value, "safe_int_cast failed: From type is not integral");
    static_assert(std::is_integral<To>::value, "safe_int_cast failed: To type is not integral");
    return detail::int_conversion<std::is_signed<From>::value, std::is_signed<To>::value, From, To>::convert(from);
}

/// \brief Concerts qsizetype to int in a checked manner.
/// \note  For Qt 5, the input value is returned as all return types of size()
///        and count() return int as well.
inline constexpr int qsizetype_to_int(elch_size_t size)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return size;
#else
    return safe_int_cast<int>(size);
#endif
}
