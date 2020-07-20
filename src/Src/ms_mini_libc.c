/*********************************************************************************************************
**
**                                北京翼辉信息技术有限公司
**
**                                  微型安全实时操作系统
**
**                                      MS-RTOS(TM)
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: ms_mini_libc.h
**
** 创   建   人: Jiao.jinxing
**
** 文件创建日期: 2020 年 04 月 07 日
**
** 描        述: mini libc
*********************************************************************************************************/
#include "main.h"
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#if MS_CFG_BOOT_MINI_LIBC_EN > 0

void bzero(void *dst, size_t n)
{
    memset(dst, 0, n);
}

void *memcpy(void *dst, const void *src, size_t n)
{
    const char *p = src;
    char *q = dst;
#if defined(__i386__)
    size_t nl = n >> 2;
    asm volatile ("cld ; rep ; movsl ; movl %3,%0 ; rep ; movsb":"+c" (nl),
              "+S"(p), "+D"(q)
              :"r"(n & 3));
#elif defined(__x86_64__)
    size_t nq = n >> 3;
    asm volatile ("cld ; rep ; movsq ; movl %3,%%ecx ; rep ; movsb":"+c"
              (nq), "+S"(p), "+D"(q)
              :"r"((uint32_t) (n & 7)));
#else
    while (n--) {
        *q++ = *p++;
    }
#endif

    return dst;
}

int strcmp(const char *s1, const char *s2)
{
    const unsigned char *c1 = (const unsigned char *)s1;
    const unsigned char *c2 = (const unsigned char *)s2;
    unsigned char ch;
    int d = 0;

    while (1) {
        d = (int)(ch = *c1++) - (int)*c2++;
        if (d || !ch)
            break;
    }

    return d;
}

void *memchr(const void *s, int c, size_t n)
{
    const unsigned char *sp = s;

    while (n--) {
        if (*sp == (unsigned char)c)
            return (void *)sp;
        sp++;
    }

    return NULL;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *c1 = s1, *c2 = s2;
    int d = 0;

    while (n--) {
        d = (int)*c1++ - (int)*c2++;
        if (d)
            break;
    }

    return d;
}

void *memset(void *dst, int c, size_t n)
{
    char *q = dst;

#if defined(__i386__)
    size_t nl = n >> 2;
    asm volatile ("cld ; rep ; stosl ; movl %3,%0 ; rep ; stosb"
              : "+c" (nl), "+D" (q)
              : "a" ((unsigned char)c * 0x01010101U), "r" (n & 3));
#elif defined(__x86_64__)
    size_t nq = n >> 3;
    asm volatile ("cld ; rep ; stosq ; movl %3,%%ecx ; rep ; stosb"
              :"+c" (nq), "+D" (q)
              : "a" ((unsigned char)c * 0x0101010101010101U),
            "r" ((uint32_t) n & 7));
#else
    while (n--) {
        *q++ = c;
    }
#endif

    return dst;
}

size_t strlen(const char *s)
{
    const char *ss = s;
    while (*ss)
        ss++;
    return ss - s;
}

size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t bytes = 0;
    char *q = dst;
    const char *p = src;
    char ch;

    while ((ch = *p++)) {
        if (bytes + 1 < size)
            *q++ = ch;

        bytes++;
    }

    /* If size == 0 there is no space for a final null... */
    if (size)
        *q = '\0';

    return bytes;
}

char *strcpy(char *dst, const char *src)
{
    char *q = dst;
    const char *p = src;
    char ch;

    do {
        *q++ = ch = *p++;
    } while (ch);

    return dst;
}

char *strchr(const char *s, int c)
{
    while (*s != (char)c) {
        if (!*s)
            return NULL;
        s++;
    }

    return (char *)s;
}

#endif
/*********************************************************************************************************
  END
*********************************************************************************************************/
