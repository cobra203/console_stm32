#ifndef _MCP2210_DEV_H_
#define _MCP2210_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <mcp2210_uaif.h>
#include <vocal_sys.h>

typedef struct mcp_status_s
{
    uint8_t evt_spk_chg     :1;
    uint8_t evt_mic_chg     :1;
    uint8_t state_spk_conn  :1;
    uint8_t state_mic_conn  :1;
    uint8_t                 :4;
} MCP_STATUS_S;

typedef struct volume_info_s
{
    uint8_t         active;
    uint8_t         mcp_req;
    RECORD_VOLUME_S volume[SPK_CFG_NUM + MIC_CFG_NUM];
} VOLUME_INFO_S;

typedef struct mcp2210_dev_s
{
    VOCAL_SYS_S     *vocal_sys;
    UAIF_STATUS_S   *status;
    uint8_t         step;

    VOLUME_INFO_S   info;
    MCP2210_UAIF_S  uaif;
    void            (*set_info)     (struct mcp2210_dev_s *, int, RECORD_VOLUME_S *);
    void            (*clr_info)     (struct mcp2210_dev_s *, int);
} MCP2210_DEV_S;

void mcp_dev_init(VOCAL_SYS_S *sys_status);


#ifdef __cplusplus
}
#endif

#endif /* _MCP2210_DEV_H_ */
