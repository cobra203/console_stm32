#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __cplusplus
 extern "C" {
#endif

#define DEBUG_ENABLE1
#define DUMP_ENABLE1

#if defined(DEBUG_ENABLE)
#include <stdio.h>
#define DEBUG(fmt, ...)     printf(fmt, ##__VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#if defined(DUMP_ENABLE)
#include <stdio.h>
#include <stm32f0xx.h>
void datadump(char *title, void *data, uint32_t len);
#else
#define datadump(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _DEBUG_H_ */
