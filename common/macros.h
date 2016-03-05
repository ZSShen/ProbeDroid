/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _UTIL_MACROS_H_
#define _UTIL_MACROS_H_


#include <cstdint>


// Fire a warning when programmers forget to use the return value from callee method.
#define WARN_UNUSED __attribute__((warn_unused_result))

// Disallows the copy and operator= functions. It goes in the private:
// declarations in a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName)              \
    TypeName(const TypeName&);                          \
    void operator=(const TypeName&)

// A macro to disallow all the implicit constructors, namely the default
// constructor, copy constructor and operator= functions. This should be used in
// the private: declarations for a class that wants to prevent anyone from
// instantiating it. This is especially useful for classes containing only
// static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) 		\
  	TypeName();                                    		\
  	DISALLOW_COPY_AND_ASSIGN(TypeName)

#define PACKED(x) __attribute__ ((__aligned__(x), __packed__))

// Hint compiler to generate optimized code for branch prediction.
#define LIKELY(x)       __builtin_expect((x), true)
#define UNLIKELY(x)     __builtin_expect((x), false)

#ifndef NDEBUG
#define ALWAYS_INLINE
#else
#define ALWAYS_INLINE  __attribute__ ((always_inline))
#endif

// Return the number of leading zeros in x.
template<typename T>
static constexpr int CLZ(T x)
{
    return (sizeof(T) == sizeof(uint32_t))? __builtin_clz(x) : __builtin_clzll(x);
}

#endif