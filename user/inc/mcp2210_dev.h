#ifndef _MCP2210_DEV_H_
#define _MCP2210_DEV_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <mcp2210_uaif.h>
#include <vocal_sys.h>

typedef struct volume_info_s
{
    uint8_t         active;
    uint8_t         mcp_req;
    RECORD_VOLUME_S volume[SPK_CFG_NUM + MIC_CFG_NUM];
} VOLUME_INFO_S;

typedef enum pc_pair_cmd_e
{
    PC_PAIR_NONE = 0,
    PC_PAIR_SPK,
    PC_PAIR_MIC,
} PC_PAIR_CMD_E;

typedef struct mcp2210_dev_s
{
    VOCAL_SYS_S     *vocal_sys;
    UAIF_STATUS_S   *status;
    UAIF_STATUS_S   *status_commit;
    int             pair;
    VOLUME_INFO_S   info;
    MCP2210_UAIF_S  uaif;
    void            (*set_info)     (struct mcp2210_dev_s *, int, RECORD_VOLUME_S *);
    void            (*clr_info)     (struct mcp2210_dev_s *, VOCAL_DEV_TYPE_E);
    void            (*commit)       (struct mcp2210_dev_s *);
} MCP2210_DEV_S;

void mcp_dev_init(VOCAL_SYS_S *sys_status);


#ifdef __cplusplus
}
#endif

#endif /* _MCP2210_DEV_H_ */
