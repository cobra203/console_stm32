#include <cc85xx_dev.h>
#include <stm32_timer.h>
#include <vocal_sys.h>
#include <vocal_record.h>
#include <debug.h>

#include <string.h>

static CC85XX_DEV_S mic_dev;
static CC85XX_DEV_S spk_dev;

#define VOLUME_IS_IN_VOL_OUTPUT     0
#define VOLUME_IS_IN_VOL_INPUT      1

#define VOLUME_IS_LOCAL_REMOTE      0
#define VOLUME_IS_LOCAL_LOCAL       1

static uint8_t _auido_channel_get(uint16_t ach_used)
{
    if(ach_used >> 12 & 0x1) {
        return 12;
    }
    if(ach_used >> 12 & 0x2) {
        return 13;
    }
    return 16;
}

static int16_t _volume_to_db(int16_t lv) {
    if(lv >= 100) {
        lv = 100;
    }
    if(lv <= 0) {
        lv = 0;
    }
    if(lv > 68) {
        return 368 + lv - 68;
    }
    if(lv > 44) {
        return 320 + (lv - 44)*2;
    }
    if(lv > 24) {
        return 240 + (lv - 24)*4;
    }
    if(lv > 8) {
        return 112 + (lv - 8)*8;
    }
    return lv * 14;
}

static void _spk_init_remote_output_volume(CC85XX_DEV_S *dev)
{

    EHIF_SET_VOLUME_S   set_volume  = {VOLUME_IS_LOCAL_REMOTE, VOLUME_IS_IN_VOL_OUTPUT};

    set_volume.mute_op      = 2;
    set_volume.set_op       = 1;
    set_volume.log_channel  = 0;
    set_volume.volume       = 0;

    if(dev->ehif.dev_type != DEV_TYPE_SPK) {
        return;
    }

    dev->ehif.vc_set_volume(&dev->ehif, &set_volume);
}

static void _dev_set_volume_alone(CC85XX_DEV_S *dev, VOCAL_DEV_TYPE_E type, int idx)
{
    /* MIC device control the volume of the MIC and SPK */
    CC85XX_DEV_S        *mic_dev = dev->vocal_sys->mic_dev;

    EHIF_SET_VOLUME_S   set_volume  = {VOLUME_IS_LOCAL_LOCAL, VOLUME_IS_IN_VOL_OUTPUT};

    set_volume.mute_op      = 2;
    set_volume.set_op       = (DEV_TYPE_SPK == type) ? 1 : 3;
    set_volume.log_channel  = (DEV_TYPE_SPK == type) ? 0 : dev->nwk_dev[idx].ach;
    set_volume.volume       = 0xfe70 + _volume_to_db(dev->nwk_dev[idx].volume);

    if(dev->nwk_dev[idx].mute || dev->nwk_dev[idx].volume == 0) {
        set_volume.volume       = 0xfc00;
    }

    //DEBUG("Set volume %s[%d]=0x%04x\n", type ? "MIC" : "SPK", idx, set_volume.volume);
    mic_dev->ehif.vc_set_volume(&mic_dev->ehif, &set_volume);
}

static void _dev_set_volume(CC85XX_DEV_S *dev, VOCAL_DEV_TYPE_E type, RANG_OF_SET_VOLUME_S rang)
{
    int     i = 0;
    uint8_t dev_num = (DEV_TYPE_SPK == type) ? SPK_DEV_NUM : MIC_DEV_NUM;

    for(i = 0; i < dev_num; i++) {
        if(dev->nwk_dev[i].device_id) {
            switch(rang) {
            case RANG_SET_RC_ONLY:
                if(0 == dev->nwk_dev[i].cmd_rc) {
                    break;
                }
                _dev_set_volume_alone(dev, type, i);
                dev->nwk_dev[i].cmd_rc = 0;
                break;
            case RANG_SET_PC_ONLY:
                if(0 == dev->nwk_dev[i].cmd_pc) {
                    break;
                }
                dev->nwk_dev[i].cmd_pc = 0;
            case RANG_SET_ALL_DEV:
                _dev_set_volume_alone(dev, type, i);
                break;
            }
        }
    }
}

static void dev_nwk_pairing(CC85XX_DEV_S *dev, uint8_t enable)
{
    dev->ehif.nwm_control_signal(&dev->ehif, enable);
}

static void dev_nwk_chg_detect(CC85XX_DEV_S *dev, VOCAL_DEV_TYPE_E type)
{
    uint8_t                 nwm_dev_exist = STM_FALSE;
    int                     i;
    EHIF_NWM_GET_STATUS_S   nwm_status = {0};
    uint32_t                device_id  = 0;
    uint16_t                ach_used = 0;
    uint8_t                 dev_num = (DEV_TYPE_SPK == type) ? SPK_DEV_NUM : MIC_DEV_NUM;
    VOCAL_SYS_S             *vocal_sys = dev->vocal_sys;

    DEBUG("%s:up\n", type ? "MIC" : "SPK");

    memset(&dev->new_nwk_info, 0, sizeof(NWK_INFO_S) * MAX_DEV_NUM);

    dev->ehif.ehc_evt_clr(&dev->ehif, (1<<1));
    delay_ms(100);
    dev->ehif.nwm_get_status(&dev->ehif, &nwm_status);
    
    if(dev->nwk_stable) { 
        dev->nwk_stable = STM_FALSE;
    }
    
    for(i = 0; i < dev_num; i++) {
        device_id   = TOHOST32(non_align_data32(&nwm_status.dev_data[0 + i*16]));
        ach_used    = TOHOST16(non_align_data16(&nwm_status.dev_data[12 + i*16]));
        DEBUG("%s NS : ach[%04x]dev[%08x]\n", type ? "MIC" : "SPK", ach_used, device_id);

        if(device_id) {
            if(0 == ach_used || 0xffffffff == device_id) {
                dev->ehif.nwm_control_enable(&dev->ehif, STM_DISABLE);
                delay_ms(30);
                dev->ehif.nwm_control_enable(&dev->ehif, STM_ENABLE);
                return;
            }

            dev->new_nwk_info[i].device_id = device_id;
            dev->new_nwk_info[i].ach_used  = _auido_channel_get(ach_used);
            dev->new_nwk_info[i].slot      = (nwm_status.dev_data[15 + i*16] >> 1) & 0x7;
            nwm_dev_exist = STM_TRUE;
        }
    }

    dev->nwk_stable = STM_TRUE;
    if(nwm_dev_exist == STM_FALSE) {
        dev->ehif.pm_set_state(&dev->ehif, 4);
    }
    else {
        dev->ehif.pm_set_state(&dev->ehif, 5);
    }
    DEBUG("--stable--\n");
    
    if(DEV_TYPE_SPK == type) {
        vocal_sys->sys_evt.req_sync_spk_nwk = STM_TRUE;
        _spk_init_remote_output_volume(dev);
    }
    else {
        vocal_sys->sys_evt.req_sync_mic_nwk = STM_TRUE;
    }
}

static void dev_rc_cmd_detect(CC85XX_DEV_S *dev)
{
    uint8_t         rc_data[12] = {0};
    uint8_t         rc_count;
    int             i;
    static uint8_t  cmd_record[MIC_DEV_NUM] = {0};
    static uint8_t  cnt_record[MIC_DEV_NUM] = {0};
    static int      times = 0;
    VOCAL_SYS_S     *vocal_sys = dev->vocal_sys;

    for(i = 0; i < MIC_DEV_NUM; i++) {
        if(dev->nwk_dev[i].device_id) {
            dev->ehif.rc_get_data(&dev->ehif, dev->nwk_dev[i].slot, rc_data, &rc_count);
            if(rc_count && rc_data[0] >= OUTPUT_VOLUME_INCREMENT && rc_data[0] <= OUTPUT_VOLUME_MUTE) {
                if(!cnt_record[i] || cmd_record[i] != rc_data[0]) {
                    DEBUG("DEV[%d]:times[%d]cmd=%d, count=%d\n", i, times, rc_data[0], rc_count);
                    dev->nwk_dev[i].cmd_rc = rc_data[0];
                    vocal_sys->sys_evt.req_sync_rc_cmd = STM_TRUE;  
                }
                times++;
            }
            cnt_record[i] = rc_count;
            cmd_record[i] = rc_data[0];
        }
    }
}

static void dev_execute(CC85XX_DEV_S *dev, VOCAL_DEV_TYPE_E type, RANG_OF_SET_VOLUME_S rang)
{    
    switch(type) {
    case DEV_TYPE_SPK:
        dev->nwk_stable = STM_TRUE;
        _dev_set_volume(dev, type, rang);
        break;
        
    case DEV_TYPE_MIC:
        dev->nwk_stable = STM_TRUE;
        _dev_set_volume(dev, type, rang);
        break;
    
    default:
        break;
    }
}

void spk_dev_init(VOCAL_SYS_S *vocal_sys)
{
    spk_dev.vocal_sys       = vocal_sys;
    spk_dev.ehif.init       = ehif_init;
    
    spk_dev.ehif.init(&spk_dev.ehif, DEV_TYPE_SPK);

    spk_dev.nwk_chg_detect  = dev_nwk_chg_detect;
    spk_dev.execute         = dev_execute;
    spk_dev.nwk_pairing     = dev_nwk_pairing;

    vocal_sys->spk_dev      = &spk_dev;
}

void mic_dev_init(VOCAL_SYS_S *vocal_sys)
{
    mic_dev.vocal_sys       = vocal_sys;
    mic_dev.ehif.init       = ehif_init;
    
    mic_dev.ehif.init(&mic_dev.ehif, DEV_TYPE_MIC);

    mic_dev.nwk_chg_detect  = dev_nwk_chg_detect;
    mic_dev.rc_cmd_detect   = dev_rc_cmd_detect;
    mic_dev.execute         = dev_execute;
    mic_dev.nwk_pairing     = dev_nwk_pairing;

    vocal_sys->mic_dev      = &mic_dev;
}

void spk_detect(VOCAL_SYS_S *vocal_sys)
{
    spk_dev.ehif.get_status(&spk_dev.ehif);
    if(spk_dev.ehif.status.pwr_state > 5) {
        return;
    }

    if(!spk_dev.nwk_enable) {
        spk_dev.ehif.nwm_control_enable(&spk_dev.ehif, STM_DISABLE);
        delay_ms(30);
        spk_dev.ehif.nwm_control_enable(&spk_dev.ehif, STM_ENABLE);
        spk_dev.nwk_enable = STM_TRUE;
    }

    if(!spk_dev.nwk_stable || spk_dev.ehif.status.evt_nwk_chg) {
        spk_dev.nwk_chg_detect(&spk_dev, DEV_TYPE_SPK);
    }
}

void mic_detect(VOCAL_SYS_S *vocal_sys)
{
    mic_dev.ehif.get_status(&mic_dev.ehif);
    if(mic_dev.ehif.status.pwr_state > 5) {
        return;
    }

    if(!mic_dev.nwk_enable) {
        mic_dev.ehif.nwm_control_enable(&mic_dev.ehif, STM_DISABLE);
        delay_ms(30);
        mic_dev.ehif.nwm_control_enable(&mic_dev.ehif, STM_ENABLE);
        mic_dev.nwk_enable = STM_TRUE;
    }

    if(!mic_dev.nwk_stable || mic_dev.ehif.status.evt_nwk_chg) {
        mic_dev.nwk_chg_detect(&mic_dev, DEV_TYPE_MIC);
    }

    if(mic_dev.nwk_stable) {
        mic_dev.rc_cmd_detect(&mic_dev);
    }
}


