#include <string.h>

#include <vocal_sys.h>
#include <cc85xx_pair.h>
#include <cc85xx_dev.h>
#include <mcp2210_dev.h>
#include <vocal_record.h>
#include <vocal_led.h>
#include <debug.h>

static void vocal_sync_nwk_dev(VOCAL_SYS_S *vocal_sys, VOCAL_DEV_TYPE_E type)
{
    int                     i, idx;
    int                     flag[MAX_DEV_NUM] = {0};
    RECORD_VOLUME_S         volume;
    uint8_t                 dev_num     = (DEV_TYPE_SPK == type) ? SPK_DEV_NUM: MIC_DEV_NUM;
    CC85XX_DEV_S            *dev        = (DEV_TYPE_SPK == type) ? vocal_sys->spk_dev : vocal_sys->mic_dev; 
    NWK_DEV_INFO_S          *nwk_dev    = &dev->nwk_dev[0];
    NWK_INFO_S              *nwk_info   = &dev->nwk_info[0];
    VOCAL_RECORD_S          *record     = vocal_sys->record;
    MCP2210_DEV_S           *mcp_dev    = vocal_sys->mcp_dev;

    memset(nwk_dev, 0, sizeof(NWK_DEV_INFO_S) * MAX_DEV_NUM);
    
    for(i = 0; i < dev_num; i++) {
        if(nwk_info[i].device_id) {
            if(STM_SUCCESS == record->get(record, nwk_info[i].device_id, type, &volume, &idx)) {
                nwk_dev[idx].device_id  = nwk_info[i].device_id;
                nwk_dev[idx].ach        = nwk_info[i].ach_used;
                nwk_dev[idx].slot       = nwk_info[i].slot;
                nwk_dev[idx].volume     = volume.volume;
                nwk_dev[idx].mute       = volume.mute; 
                DEBUG("[old]i=%d, ach=0x%04x, device_id=0x%08x", i, nwk_dev[idx].ach, nwk_dev[idx].device_id);
                mcp_dev->set_info(mcp_dev, (DEV_TYPE_SPK == type) ? 0 : idx + SPK_CFG_NUM, &volume);
            }
            else {
                mcp_dev->clr_info(mcp_dev, (DEV_TYPE_SPK == type) ? 0 : idx + SPK_CFG_NUM);
                flag[i] = STM_TRUE;
            }
        }
    }

    for(i = 0; i < dev_num; i++) {
        if(STM_TRUE == flag[i]) {
            volume.volume   = 80;
            volume.mute     = 0;
            if(DEV_TYPE_SPK == type && record->active & 0xf0) {
                volume.volume   = record->spk_vl.volume;
                volume.mute     = record->spk_vl.mute;
            }
            record->add(record, nwk_info[i].device_id, type, &volume, &idx);
            nwk_dev[idx].device_id  = nwk_info[i].device_id;
            nwk_dev[idx].ach        = nwk_info[i].ach_used;
            nwk_dev[idx].slot       = nwk_info[i].slot;
            nwk_dev[idx].volume     = volume.volume;
            nwk_dev[idx].mute       = volume.mute;
            DEBUG("[new]i=%d, ach=0x%04x, device_id=0x%08x", i, nwk_dev[idx].ach, nwk_dev[idx].device_id);
            mcp_dev->set_info(mcp_dev, (DEV_TYPE_SPK == type) ? 0 : SPK_CFG_NUM + idx, &volume);
            record->evt.rcd_evt_dev_id_chg  = STM_TRUE;
            record->evt.rcd_evt_volume_chg  = STM_TRUE;
        }
    }
    
    if(DEV_TYPE_SPK == type) { 
        vocal_sys->sys_evt.req_sync_spk_nwk = STM_FALSE;
    }
    else {    
        vocal_sys->sys_evt.req_sync_mic_nwk = STM_FALSE;
    }
}

static void vocal_sync_rc_cmd(VOCAL_SYS_S *vocal_sys)
{
    int i, idx;
    NWK_DEV_INFO_S  *nwk_dev    = &vocal_sys->mic_dev->nwk_dev[0];
    VOCAL_RECORD_S  *record     = vocal_sys->record;
    MCP2210_DEV_S   *mcp_dev    = vocal_sys->mcp_dev;
    RECORD_VOLUME_S volume = {0};

    for(i = 0; i < MIC_DEV_NUM; i++) {
        if(nwk_dev[i].device_id && nwk_dev[i].cmd_rc) {
            switch(nwk_dev[i].cmd_rc) {
            case OUTPUT_VOLUME_INCREMENT:
                if(nwk_dev[i].volume + 5 >= 100) {
                    nwk_dev[i].volume = 100;
                }
                else {
                    nwk_dev[i].volume += 5;
                }
                DEBUG("mic[%d] : ========> db=%d", i, nwk_dev[i].volume);
                break;
            case OUTPUT_VOLUME_DECREASE:
                if(nwk_dev[i].volume - 5 <= 0) {
                    nwk_dev[i].volume = 0;
                }
                else {
                    nwk_dev[i].volume -= 5;
                }
                DEBUG("mic[%d] : ========> db=%d", i, nwk_dev[i].volume);
                break;
            case OUTPUT_VOLUME_MUTE:
                nwk_dev[i].mute = nwk_dev[i].mute ? STM_FALSE : STM_TRUE;
                DEBUG("mic[%d] : volume mute=%d", i, nwk_dev[i].mute);
                break;
            }
            
            volume.volume  = nwk_dev[i].volume;
            volume.mute    = nwk_dev[i].mute;
            
            record->set(record, nwk_dev[i].device_id, DEV_TYPE_MIC, &volume, &idx);
            mcp_dev->set_info(mcp_dev, SPK_CFG_NUM + idx, &volume);

            record->evt.rcd_evt_volume_chg = STM_TRUE;
        }
    }
    vocal_sys->sys_evt.req_sync_rc_cmd = STM_FALSE;
}

static void vocal_sync_pc_cmd(VOCAL_SYS_S *vocal)
{
    int i, idx;
    MCP2210_DEV_S   *mcp_dev    = vocal->mcp_dev;
    CC85XX_DEV_S    *mic_dev    = vocal->mic_dev;
    CC85XX_DEV_S    *spk_dev    = vocal->spk_dev;
    VOCAL_RECORD_S  *record     = vocal->record;
    
    if(BIT_ISSET(mcp_dev->info.mcp_req, 0)) {
        for(i = 0; i < SPK_DEV_NUM; i++) {
            if(spk_dev->nwk_dev[i].device_id) {
                spk_dev->nwk_dev[i].volume  = mcp_dev->info.volume[0].volume;
                spk_dev->nwk_dev[i].mute    = mcp_dev->info.volume[0].mute;
                spk_dev->nwk_dev[i].cmd_pc  = STM_TRUE;
                record->set(record, spk_dev->nwk_dev[i].device_id, DEV_TYPE_SPK, &mcp_dev->info.volume[0], &idx);
            }
        }
    }

    for(i = 0; i < MIC_DEV_NUM; i++) {
        if(BIT_ISSET(mcp_dev->info.mcp_req, SPK_CFG_NUM + i) && mic_dev->nwk_dev[i].device_id) {
            mic_dev->nwk_dev[i].volume  = mcp_dev->info.volume[SPK_CFG_NUM + i].volume;
            mic_dev->nwk_dev[i].mute    = mcp_dev->info.volume[SPK_CFG_NUM + i].mute;
            mic_dev->nwk_dev[i].cmd_pc  = STM_TRUE;
            record->set(record, mic_dev->nwk_dev[i].device_id, DEV_TYPE_MIC, &mcp_dev->info.volume[SPK_CFG_NUM + i], &idx);
        }
    }

    vocal->sys_evt.req_sync_pc_cmd = STM_FALSE;
}

void vocal_init(VOCAL_SYS_S *vocal)
{
    vocal->sync_nwk_dev = vocal_sync_nwk_dev;
    vocal->sync_rc_cmd  = vocal_sync_rc_cmd;
    vocal->sync_pc_cmd  = vocal_sync_pc_cmd;

    record_init(vocal);
    led_init(vocal);
    while(0) {

    };
    
    pair_init(vocal);
    mic_dev_init(vocal);
    spk_dev_init(vocal);
    mcp_dev_init(vocal);
}

void vocal_working(VOCAL_SYS_S *vocal)
{
    while(STM_TRUE) {
        spk_detect(vocal);
        if(vocal->sys_evt.req_sync_spk_nwk) {
            vocal->sync_nwk_dev(vocal, DEV_TYPE_SPK);
            vocal->spk_dev->execute(vocal->spk_dev, DEV_TYPE_SPK, RANG_SET_ALL_DEV);
            vocal->led->commit(vocal->led);
            vocal->record->commit(vocal->record);
        }
        
        mic_detect(vocal);
        if(vocal->sys_evt.req_sync_mic_nwk) {
            vocal->sync_nwk_dev(vocal, DEV_TYPE_MIC);
            vocal->mic_dev->execute(vocal->mic_dev, DEV_TYPE_MIC, RANG_SET_ALL_DEV);
            vocal->record->commit(vocal->record);
        }
        
        if(vocal->sys_evt.req_sync_rc_cmd) {
            vocal->sync_rc_cmd(vocal);
            vocal->mic_dev->execute(vocal->mic_dev, DEV_TYPE_MIC, RANG_SET_RC_ONLY);
            vocal->record->commit(vocal->record);
        }

        if(vocal->sys_evt.req_sync_pc_cmd) {
            vocal->sync_pc_cmd(vocal);
            vocal->spk_dev->execute(vocal->spk_dev, DEV_TYPE_SPK, RANG_SET_PC_ONLY);
            vocal->mic_dev->execute(vocal->mic_dev, DEV_TYPE_MIC, RANG_SET_PC_ONLY);
            vocal->record->commit(vocal->record);
        }
    }
}

