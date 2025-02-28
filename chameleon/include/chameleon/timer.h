/**
 *
 * @file chameleon_timer.h
 *
 * @copyright 2009-2014 The University of Tennessee and The University of
 *                      Tennessee Research Foundation. All rights reserved.
 * @copyright 2012-2021 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
 *                      Univ. Bordeaux. All rights reserved.
 *
 * @version 1.0.0
 *
 * @brief Chameleon timer
 *
 * Provide a simple timer for examples and runtimes which do not provide their
 * own timer.
 * @date 2020-03-03
 *
 */
#ifndef _chameleon_timer_h_
#define _chameleon_timer_h_

#if defined( _WIN32 ) || defined( _WIN64 )
#include <windows.h>
#include <time.h>
#include <sys/timeb.h>
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

static inline int
gettimeofday(struct timeval* tv, struct timezone* tz)
{
    FILETIME         ft;
    unsigned __int64 tmpres = 0;
    static int       tzflag;

    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft);
        tmpres |=  ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |=  ft.dwLowDateTime;

        /*converting file time to unix epoch*/
        tmpres /= 10;  /*convert into microseconds*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS;

        tv->tv_sec  = (long)(tmpres / 1000000UL);
        tv->tv_usec = (long)(tmpres % 1000000UL);
    }
    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime     = _daylight;
    }
    return 0;
}

#else  /* Non-Windows */
#include <sys/time.h>
#endif

/**
 * @brief Return a simple timestamp in s.
 *
 * @return The timestamp in s without any barriers.
 */
static inline double
CHAMELEON_timer(void)
{
    struct timeval tp;
    gettimeofday( &tp, NULL );
    return tp.tv_sec + 1e-6 * tp.tv_usec;
}

#endif /* _chameleon_timer_h_ */
