#ifndef DDSIMG_UTIL_H
#define DDSIMG_UTIL_H
#pragma once

#if defined(__clang__) || defined(__GNUC__)
#define MAX(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#else
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#endif

#endif //DDSIMG_UTIL_H
