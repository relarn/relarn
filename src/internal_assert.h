// This file is part of ReLarn; Copyright (C) 1986 - 2020; GPLv2; NO WARRANTY!
// See Copyright.txt, LICENSE.txt and AUTHORS.txt for terms.

// Basic assertion.  Unlike assert.h, this version first resets the
// tty so that the message isn't lost.  It also provides ENSURE, which
// isn't disabled in release builds and which exits gracefully instead
// of using abort().


#ifndef HDR_GUARD_LARN_ASSERT_H
#define HDR_GUARD_LARN_ASSERT_H

#include <stdbool.h>

#define ASSERT(x) ASSERT_MSG(x,"")
#define ENSURE(x) ENSURE_MSG(x,"") 

#define ENSURE_MSG(x,msg) \
    internal_assert(!!(x), __FILE__, __LINE__, #x, msg, false)

#define FAIL(msg) \
    internal_assert(0, __FILE__, __LINE__, "", msg, true)

#ifdef DISABLE_ASSERT
#    define ASSERT_MSG(x,msg)
#else
#    define ASSERT_MSG(x,msg) \
        internal_assert(!!(x), __FILE__, __LINE__, #x, msg, true)
#endif

void internal_assert(int expr, const char *file, int line, const char *exprDesc,
                     const char *msg, bool coredump);

#endif
