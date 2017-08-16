#ifndef _MCP2210_UAIF_H_
#define _MCP2210_UAIF_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <stm32f0xx.h>
#include <vocal_common.h>

/* UAIF Status Word Define */
typedef struct uaif_status_word_s
{
    uint16_t    evt_mic_chg         :1;
    uint16_t    evt_spk_chg         :1;
    uint16_t                        :6;
    uint16_t    mic_wasp_conn       :1;
    uint16_t    spk_wasp_conn       :1;
    uint16_t                        :5;
    uint16_t    cmdreq_rdy          :1;
} UAIF_STATUS_S;

typedef enum uaif_cmd_req_type_e
{
    CMD_NULL            = 0,
    CMD_GET_STATUS      = 0x01,
    CMD_GET_VOLUME      = 0x16,
    CMD_SET_VOLUME      = 0x17,
} UAIF_CMD_REQ_TYPE_E;

/* SPI Data Head Defines */
typedef struct uaif_head_CMD_REQ_s
{
    uint16_t    data_len            :8;
    uint16_t    cmd_type            :6;
    uint16_t    magic_num           :2;
} UAIF_HEAD_CMD_REQ_S;

typedef struct uaif_head_READ_WRITE_s
{
    uint16_t    data_len            :12;
    uint16_t    magic_num           :4;
} UAIF_HEAD_READ_WRITE_S;

typedef struct mcp2210_uaif_s
{
    UAIF_STATUS_S       status;
    void                (*init)             (struct mcp2210_uaif_s *, void (*)(void *), void *);
    int                 (*get_cmd_type)     (struct mcp2210_uaif_s *, UAIF_CMD_REQ_TYPE_E *);
    void                (*set_volume)       (struct mcp2210_uaif_s *, int *, RECORD_VOLUME_S *);
    int                 (*get_volume)       (struct mcp2210_uaif_s *, int, const RECORD_VOLUME_S *);
} MCP2210_UAIF_S;

void uaif_init(MCP2210_UAIF_S *uaif, void (*process)(void *), void *args);

#ifdef __cplusplus
}
#endif

#endif /* _MCP2210_UAIF_H_ */

