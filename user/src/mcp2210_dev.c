#include <string.h>

#include <mcp2210_dev.h>
#include <mcp2210_uaif.h>

static MCP2210_DEV_S mcp2210_dev;

typedef enum mcp_step_type_e{
    MCP_STEP_GAIN_CMD_REQ,
} MCP_STEP_TYPE_E;

#define MCP_STEP_CHECK(s)   BIT_ISSET(mcp2210_dev.step, s)
#define MCP_STEP_SET(s)     BIT_SET(mcp2210_dev.step, s)
#define MCP_STEP_CLR(s)     BIT_CLR(mcp2210_dev.step, s)

static void _mcp_status_updata(MCP2210_DEV_S *mcp, int idx)
{
    if(idx) {
        mcp->status->evt_mic_chg = STM_TRUE;
    }
    else {
        mcp->status->evt_spk_chg = STM_TRUE;
    }
    
    mcp->status->spk_wasp_conn = (mcp->info.active & 0xf) ? STM_TRUE : STM_FALSE;
    mcp->status->mic_wasp_conn = (mcp->info.active & 0xf0) ? STM_TRUE : STM_FALSE;
}

static void mcp_set_volume_info(MCP2210_DEV_S *mcp, int idx, RECORD_VOLUME_S *volume)
{
    memcpy(&mcp->info.volume[idx], volume, sizeof(RECORD_VOLUME_S));
    BIT_SET(mcp->info.active, idx);
    _mcp_status_updata(mcp, idx);
}

static void mcp_clr_volume_info(MCP2210_DEV_S *mcp, int idx)
{
    memset(&mcp->info.volume[idx], 0, sizeof(RECORD_VOLUME_S));
    BIT_SET(mcp->info.active, idx);
    _mcp_status_updata(mcp, idx);
}

static void mcp2210_process(void *args)
{
    VOCAL_SYS_S     *vocal_sys = (VOCAL_SYS_S *)args;
    int             idx;
    RECORD_VOLUME_S volume;
    
    static UAIF_CMD_REQ_TYPE_E cmd_type = CMD_NULL;

    if(!MCP_STEP_CHECK(MCP_STEP_GAIN_CMD_REQ)) {
        if(STM_SUCCESS == mcp2210_dev.uaif.get_cmd_type(&mcp2210_dev.uaif, &cmd_type)) {
            MCP_STEP_SET(MCP_STEP_GAIN_CMD_REQ);
            switch(cmd_type) {
            case CMD_SET_VOLUME:
                mcp2210_dev.uaif.set_volume(&mcp2210_dev.uaif, &idx, &volume);
                BIT_SET(mcp2210_dev.info.mcp_req, idx);
                vocal_sys->sys_evt.req_sync_pc_cmd = STM_TRUE;
                MCP_STEP_CLR(MCP_STEP_GAIN_CMD_REQ);
                cmd_type = CMD_NULL;
                break;
            default:
                break;
            }
        }
    }
    else {
        switch(cmd_type) {
        case CMD_GET_VOLUME:
            mcp2210_dev.uaif.get_volume(&mcp2210_dev.uaif, mcp2210_dev.info.active, &mcp2210_dev.info.volume[0]);      
            break;
        default:  
            break;
        }
        MCP_STEP_CLR(MCP_STEP_GAIN_CMD_REQ);
        cmd_type = CMD_NULL;
    }
}

void mcp_dev_init(VOCAL_SYS_S *vocal_sys)
{
    mcp2210_dev.vocal_sys   = vocal_sys;
    vocal_sys->mcp_dev      = &mcp2210_dev;   
    mcp2210_dev.status      = &mcp2210_dev.uaif.status;

    mcp2210_dev.set_info    = mcp_set_volume_info;
    mcp2210_dev.clr_info    = mcp_clr_volume_info;

    mcp2210_dev.uaif.init   = uaif_init;
    mcp2210_dev.uaif.init(&mcp2210_dev.uaif, mcp2210_process, (void *)vocal_sys);
}

