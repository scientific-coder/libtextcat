#ifndef _COMMON_H_
#define _COMMON_H_
/**
 * common.h
 *
 * Copyright (C) 2003 WiseGuys Internet B.V.
 *
 * THE BSD LICENSE
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 * 
 * - Neither the name of the WiseGuys Internet B.V. nor the names of
 * its contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <inttypes.h>
#include <time.h>

#include <sys/time.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t        uint4;
typedef uint16_t        uint2;
typedef uint8_t         uchar;

typedef int32_t         sint4;
typedef int16_t         sint2;
typedef int8_t          schar;

typedef int8_t          boole;

typedef struct wgtimer_s {
        struct timeval start;
        struct timeval stop;
} wgtimer_t;


extern void *wg_malloc( size_t size );
extern void *wg_calloc( size_t nmemb, size_t size );
extern void *wg_zalloc( size_t size );
extern char* wg_strdup( const char *s );
extern void* wg_realloc( void *ptr, size_t size ) ;
extern void wg_free( void *mem );

extern char *wg_getline( char *line, int size, FILE *fp );

extern void wg_timerstart(wgtimer_t *t);
extern uint4 wg_timerstop(wgtimer_t *t);

extern unsigned int wg_split( char **result, char *dest, char *src, int maxsegments );
extern char *wg_strgmov( char *dest, const char *src, const char *destlimit );
extern char *wg_trim( char *dest, const char *src );

#ifdef __cplusplus
}
#endif
      
#endif
