#include <string.h>
#include <debug.h>
#include <mcp2210_dev.h>
#include <mcp2210_uaif.h>

static MCP2210_DEV_S mcp2210_dev;

static void mcp_commit_status(MCP2210_DEV_S *mcp)
{
    memcpy(mcp->status, mcp->status_commit, sizeof(UAIF_STATUS_S));
    memset(mcp->status_commit, 0, sizeof(UAIF_STATUS_S));
    mcp->status->cmdreq_rdy = STM_TRUE;
    
}

static void mcp_set_volume_info(MCP2210_DEV_S *mcp, int idx, RECORD_VOLUME_S *volume)
{
    int cfg_idx;

    mcp->status->cmdreq_rdy = STM_FALSE;
    
    if(idx < SPK_DEV_NUM) {
        cfg_idx = 0;
        mcp->status_commit->evt_spk_chg    = STM_TRUE;
        mcp->status_commit->spk_wasp_conn  = STM_TRUE;
    }
    else {
        cfg_idx = idx - SPK_DEV_NUM + SPK_CFG_NUM;
        mcp->status_commit->evt_mic_chg    = STM_TRUE;
        mcp->status_commit->mic_wasp_conn  = STM_TRUE;
    }

    BIT_SET(mcp->info.active, cfg_idx);
    memcpy(&mcp->info.volume[cfg_idx], volume, sizeof(RECORD_VOLUME_S));
}

static void mcp_clr_volume_info(MCP2210_DEV_S *mcp, VOCAL_DEV_TYPE_E type)
{
    mcp->status->cmdreq_rdy = STM_FALSE;
    
    if(DEV_TYPE_SPK == type) {
        memset(&mcp->info.volume[0], 0, sizeof(RECORD_VOLUME_S));
        mcp->info.active &= 0xfe;
        mcp->status_commit->evt_spk_chg    = STM_TRUE;
        mcp->status_commit->spk_wasp_conn  = STM_FALSE;
    }
    else {
        memset(&mcp->info.volume[SPK_CFG_NUM], 0, sizeof(RECORD_VOLUME_S) * MIC_DEV_NUM);
        mcp->info.active &= 0xf9;
        mcp->status_commit->evt_mic_chg    = STM_TRUE;
        mcp->status_commit->mic_wasp_conn  = STM_FALSE;
    }
}

static void mcp2210_process(void *args)
{
    VOCAL_SYS_S     *vocal_sys = (VOCAL_SYS_S *)args;
    int             cfg_idx;
    uint8_t         pair_type;
    RECORD_VOLUME_S volume;
    UAIF_CMD_REQ_TYPE_E cmd_type = CMD_NULL;
  
    if(STM_SUCCESS != mcp2210_dev.uaif.get_cmd_type(&mcp2210_dev.uaif, &cmd_type)) {
        return;
    }

    switch(cmd_type) {
    case CMD_GET_VOLUME:
        if(STM_SUCCESS != mcp2210_dev.uaif.get_volume(&mcp2210_dev.uaif, mcp2210_dev.info.active, &mcp2210_dev.info.volume[0])) {
            return;
        }
        break;
    case CMD_SET_VOLUME:
        if(STM_SUCCESS != mcp2210_dev.uaif.set_volume(&mcp2210_dev.uaif, &cfg_idx, &volume)) {
            return;
        }
        BIT_SET(mcp2210_dev.info.mcp_req, cfg_idx);
        memcpy(&mcp2210_dev.info.volume[cfg_idx],
                    &volume, sizeof(RECORD_VOLUME_S));
        vocal_sys->sys_evt.req_sync_pc_cmd = STM_TRUE;
        break;
    case CMD_NWM_CONTROL_SIGNAL:
        if(STM_SUCCESS != mcp2210_dev.uaif.nwm_pairing(&mcp2210_dev.uaif, &pair_type)) {
            return;
        }
        BIT_SET(mcp2210_dev.info.mcp_req, SPK_CFG_NUM + MIC_CFG_NUM + pair_type);
        vocal_sys->sys_evt.req_sync_pc_cmd = STM_TRUE;
        break;
    default:
        break;
    }
}

void mcp_dev_init(VOCAL_SYS_S *vocal_sys)
{
    mcp2210_dev.vocal_sys       = vocal_sys;
    vocal_sys->mcp_dev          = &mcp2210_dev;   
    mcp2210_dev.status          = &mcp2210_dev.uaif.status;
    mcp2210_dev.status_commit   = &mcp2210_dev.uaif.status_commit;

    mcp2210_dev.set_info        = mcp_set_volume_info;
    mcp2210_dev.clr_info        = mcp_clr_volume_info;
    mcp2210_dev.commit          = mcp_commit_status;

    mcp2210_dev.uaif.init       = uaif_init;
    mcp2210_dev.uaif.init(&mcp2210_dev.uaif, mcp2210_process, (void *)vocal_sys);
}

