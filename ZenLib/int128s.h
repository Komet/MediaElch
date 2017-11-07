/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a zlib-style license that can
 *  be found in the License.txt file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// based on http://Tringi.Mx-3.cz
// Only adapted for ZenLib:
// - .hpp --> .h
// - Namespace
// - int128s alias
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifndef INT128_HPP
#define INT128_HPP

/*
  Name: int128.hpp
  Copyright: Copyright (C) 2005, Jan Ringos
  Author: Jan Ringos, http://Tringi.Mx-3.cz

  Version: 1.1
*/

#include <exception>
#include <cstdlib>
#include <cstdio>
#ifdef __cplusplus
    #include <new> //for size_t
#else /* __cplusplus */
    #include <stddef.h> //for size_t
#endif /* __cplusplus */
#include "ZenLib/Conf.h"

namespace ZenLib
{

// CLASS

class int128 {
    private:
        // Binary correct representation of signed 128bit integer
        int64u lo;
        int64s hi;

    protected:
        // Some global operator functions must be friends
        friend bool operator <  (const int128 &, const int128 &) throw ();
        friend bool operator == (const int128 &, const int128 &) throw ();
        friend bool operator || (const int128 &, const int128 &) throw ();
        friend bool operator && (const int128 &, const int128 &) throw ();

    public:
        // Constructors
        inline int128 () throw () : lo(0), hi(0) {};
        inline int128 (const int128 & a) throw () : lo (a.lo), hi (a.hi) {};

        inline int128 (const unsigned int & a) throw () : lo (a), hi (0ll) {};
        inline int128 (const signed int & a) throw () : lo (a), hi (0ll) {
            if (a < 0) this->hi = -1ll;
        };

        inline int128 (const int64u & a) throw () : lo (a), hi (0ll) {};
        inline int128 (const int64s & a) throw () : lo (a), hi (0ll) {
            if (a < 0) this->hi = -1ll;
        };

        int128 (const float a) throw ();
        int128 (const double & a) throw ();
        int128 (const long double & a) throw ();

        int128 (const char * sz) throw ();

        // TODO: Consider creation of operator= to eliminate
        //       the need of intermediate objects during assignments.

    private:
        // Special internal constructors
        int128 (const int64u & a, const int64s & b) throw ()
            : lo (a), hi (b) {};

    public:
        // Operators
        bool operator ! () const throw ();

        int128 operator - () const throw ();
        int128 operator ~ () const throw ();

        int128 & operator ++ ();
        int128 & operator -- ();
        int128 operator ++ (int);
        int128 operator -- (int);

        int128 & operator += (const int128 & b) throw ();
        int128 & operator *= (const int128 & b) throw ();

        int128 & operator >>= (unsigned int n) throw ();
        int128 & operator <<= (unsigned int n) throw ();

        int128 & operator |= (const int128 & b) throw ();
        int128 & operator &= (const int128 & b) throw ();
        int128 & operator ^= (const int128 & b) throw ();

        // Inline simple operators
        inline const int128 & operator + () const throw () { return *this; };

        // Rest of inline operators
        inline int128 & operator -= (const int128 & b) throw () {
            return *this += (-b);
        };
        inline int128 & operator /= (const int128 & b) throw () {
            int128 dummy;
            *this = this->div (b, dummy);
            return *this;
        };
        inline int128 & operator %= (const int128 & b) throw () {
            this->div (b, *this);
            return *this;
        };

        // Common methods
        int toInt () const throw () {  return (int) this->lo; };
        int64s toInt64 () const throw () {  return (int64s) this->lo; };

        const char * toString (unsigned int radix = 10) const throw ();
        float toFloat () const throw ();
        double toDouble () const throw ();
        long double toLongDouble () const throw ();

        // Arithmetic methods
        int128  div (const int128 &, int128 &) const throw ();

        // Bit operations
        bool    bit (unsigned int n) const throw ();
        void    bit (unsigned int n, bool val) throw ();
}
#ifdef __GNUC__
    __attribute__ ((__aligned__ (16), __packed__))
#endif
;


// GLOBAL OPERATORS

bool operator <  (const int128 & a, const int128 & b) throw ();
bool operator == (const int128 & a, const int128 & b) throw ();
bool operator || (const int128 & a, const int128 & b) throw ();
bool operator && (const int128 & a, const int128 & b) throw ();

// GLOBAL OPERATOR INLINES

inline int128 operator + (const int128 & a, const int128 & b) throw () {
    return int128 (a) += b; };
inline int128 operator - (const int128 & a, const int128 & b) throw () {
    return int128 (a) -= b; };
inline int128 operator * (const int128 & a, const int128 & b) throw () {
    return int128 (a) *= b; };
inline int128 operator / (const int128 & a, const int128 & b) throw () {
    return int128 (a) /= b; };
inline int128 operator % (const int128 & a, const int128 & b) throw () {
    return int128 (a) %= b; };

inline int128 operator >> (const int128 & a, unsigned int n) throw () {
    return int128 (a) >>= n; };
inline int128 operator << (const int128 & a, unsigned int n) throw () {
    return int128 (a) <<= n; };

inline int128 operator & (const int128 & a, const int128 & b) throw () {
    return int128 (a) &= b; };
inline int128 operator | (const int128 & a, const int128 & b) throw () {
    return int128 (a) |= b; };
inline int128 operator ^ (const int128 & a, const int128 & b) throw () {
    return int128 (a) ^= b; };

inline bool operator >  (const int128 & a, const int128 & b) throw () {
    return   b < a; };
inline bool operator <= (const int128 & a, const int128 & b) throw () {
    return !(b < a); };
inline bool operator >= (const int128 & a, const int128 & b) throw () {
    return !(a < b); };
inline bool operator != (const int128 & a, const int128 & b) throw () {
    return !(a == b); };


// MISC

//typedef int128 __int128;

typedef int128 int128s;
} //NameSpace

#endif
