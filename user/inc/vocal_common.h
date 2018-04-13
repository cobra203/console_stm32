#ifndef VOCAL_COMMON_H
#define VOCAL_COMMON_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <string.h>
#include <vocal_pin_def.h>

#define     BIT_ISSET(a, s) (((a) >> (s)) & 0x1)
#define     BIT_SET(a, s)   ((a) = (a) | 0x1 << (s))
#define     BIT_CLR(a, s)   ((a) = (a) & ~(0x1 << (s)))

#define     STM_SUCCESS     0
#define     STM_FAILURE     -1

#define     STM_TRUE        1
#define     STM_FALSE       0

#define     STM_ENABLE      STM_TRUE
#define     STM_DISABLE     STM_FALSE

#define     MAX_DEV_NUM     4
#define     MIC_DEV_NUM     2
#define     SPK_DEV_NUM     4

#define     MIC_CFG_NUM     2
#define     SPK_CFG_NUM     1

#define		ERR_LED_NUM		2

typedef union data_16_un {
    uint16_t i;
    uint8_t  s[2];
} DATA_16_UN;

typedef union data_32_un {
    uint32_t i;
    uint8_t  s[4];
} DATA_32_UN;

inline static int _check_endian(void) /* big reture true */
{
    DATA_16_UN c;

    c.i = 0x1234;
    return (0x12 == c.s[0]);
}

#define _SWAP16(A)  ((((uint16_t)(A) & 0xff00) >> 8) | \
                    (((uint16_t)(A) & 0x00ff) << 8))

#define _SWAP32(A)  ((((uint32_t)(A) & 0xff000000) >> 24) | \
                    (((uint32_t)(A) & 0x00ff0000) >> 8)  | \
                    (((uint32_t)(A) & 0x0000ff00) << 8)  | \
                    (((uint32_t)(A) & 0x000000ff) << 24))

#define TONET16(x)  (_check_endian() ? (x) : _SWAP16(x))
#define TONET32(x)  (_check_endian() ? (x) : _SWAP32(x))

#define TOHOST16(x) TONET16(x)
#define TOHOST32(x) TONET32(x)

inline static uint16_t non_align_data16(void *addr)
{
    uint8_t     *byte = addr;
    DATA_16_UN  data;

    memcpy(data.s, byte, sizeof(uint16_t));

    return data.i;
}

inline static uint32_t non_align_data32(void *addr)
{
    uint8_t     *byte = addr;
    DATA_32_UN  data;

    
    memcpy(data.s, byte, sizeof(uint32_t));

    return data.i;
}

inline static void error_reboot(void)
{
    *(uint32_t *)0 = 0;
}

typedef enum vocal_dev_type_e
{
    DEV_TYPE_SPK = 0,  
    DEV_TYPE_MIC,
    DEV_TYPE_BUTT,
} VOCAL_DEV_TYPE_E;

typedef struct record_volume_s
{
    uint8_t     volume      :7;
    uint8_t     mute        :1;
} RECORD_VOLUME_S;

#ifdef __cplusplus
}
#endif

#endif /* VOCAL_COMMON_H */
