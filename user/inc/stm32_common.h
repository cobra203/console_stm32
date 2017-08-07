#ifndef STM32_COMMON_H
#define STM32_COMMON_H

#ifdef __cplusplus
 extern "C" {
#endif

#define     BIT_ISSET(a, s)     (((a) >> (s)) & 0x1)
#define     BIT_SET(a, s)       ((a) = (a) | 0x1 << (s))
#define     BIT_CLR(a, s)       ((a) = (a) & ~(0x1 << (s)))

#ifdef __cplusplus
}
#endif

#endif /* STM32_COMMON_H */
