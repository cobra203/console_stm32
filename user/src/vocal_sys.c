#include <string.h>

#include <vocal_sys.h>
#include <cc85xx_pair.h>
#include <cc85xx_dev.h>
#include <mcp2210_dev.h>
#include <vocal_record.h>
#include <vocal_led.h>
#include <debug.h>
#include <stm32_timer.h>
#include <stm32f0xx_spi.h>
#include <vocal_common.h>

static void vocal_sync_nwk_dev(VOCAL_SYS_S *vocal_sys, VOCAL_DEV_TYPE_E type)
{
    int                     i, idx;
    int                     flag[MAX_DEV_NUM] = {0};
    RECORD_VOLUME_S         volume;
    uint8_t                 dev_num         = (DEV_TYPE_SPK == type) ? SPK_DEV_NUM: MIC_DEV_NUM;
    CC85XX_DEV_S            *dev            = (DEV_TYPE_SPK == type) ? vocal_sys->spk_dev : vocal_sys->mic_dev; 
    NWK_DEV_INFO_S          *nwk_dev        = &dev->nwk_dev[0];
    NWK_INFO_S              *new_nwk_info   = &dev->new_nwk_info[0];
    VOCAL_RECORD_S          *record         = vocal_sys->record;
    MCP2210_DEV_S           *mcp_dev        = vocal_sys->mcp_dev;
    VOCAL_LED_S             *led            = vocal_sys->led;

    DEBUG("nwk sync\n");
    memset(nwk_dev, 0, sizeof(NWK_DEV_INFO_S) * MAX_DEV_NUM);
    mcp_dev->clr_info(mcp_dev, type);
    record->active &= ((DEV_TYPE_SPK == type) ? 0xf0 : 0xf);

    /* free pairing */
    if(dev->pair_status.task < TIMERS_NUM) {
        dev->nwk_pairing(dev, STM_DISABLE);
        timer_free(&dev->pair_status.task);
    }
    
    for(i = 0; i < dev_num; i++) {
        if(new_nwk_info[i].device_id) {
            if(STM_SUCCESS == record->get(record, new_nwk_info[i].device_id, type, &volume, &idx)) {
                nwk_dev[idx].device_id  = new_nwk_info[i].device_id;
                nwk_dev[idx].ach        = new_nwk_info[i].ach_used;
                nwk_dev[idx].slot       = new_nwk_info[i].slot;
                nwk_dev[idx].volume     = volume.volume;
                nwk_dev[idx].mute       = volume.mute; 
                DEBUG("[o] : %s[%d]|ach[0x%04x]|dev[0x%08x]\n", (DEV_TYPE_SPK == type) ? "SPK" : "MIC", 
                            i, nwk_dev[idx].ach, nwk_dev[idx].device_id);
                /* sync to mcp */
                mcp_dev->set_info(mcp_dev, (DEV_TYPE_SPK == type) ? idx : (SPK_DEV_NUM + idx), &volume);
                /* sync to led */
                led->set(led, type, idx, LED_STATUS_CONNECT);
            }
            else {
                flag[i] = STM_TRUE;
            }
        }
    }

    for(i = 0; i < dev_num; i++) {
        if(STM_TRUE == flag[i]) {
            volume.volume   = (DEV_TYPE_SPK == type) ? 80 : 80;
            volume.mute     = 0;
            if(DEV_TYPE_SPK == type && record->active & 0xf) {
                volume.volume   = record->spk_vl.volume;
                volume.mute     = record->spk_vl.mute;
            }
            /* sync to record */
            record->add(record, new_nwk_info[i].device_id, type, &volume, &idx);
            nwk_dev[idx].device_id  = new_nwk_info[i].device_id;
            nwk_dev[idx].ach        = new_nwk_info[i].ach_used;
            nwk_dev[idx].slot       = new_nwk_info[i].slot;
            nwk_dev[idx].volume     = volume.volume;
            nwk_dev[idx].mute       = volume.mute;
            DEBUG("[n] : %s[%d]|ach[0x%04x]|dev[0x%08x]\n", (DEV_TYPE_SPK == type) ? "SPK" : "MIC",
                        i, nwk_dev[idx].ach, nwk_dev[idx].device_id);
            /* sync to mcp */
            mcp_dev->set_info(mcp_dev, (DEV_TYPE_SPK == type) ? idx : (SPK_DEV_NUM + idx), &volume);
            /* sync to led */
            led->set(led, type, idx, LED_STATUS_CONNECT);

            record->evt.rcd_evt_dev_id_chg  = STM_TRUE;
            record->evt.rcd_evt_volume_chg  = STM_TRUE;
        }
    }

    mcp_dev->commit(mcp_dev);
    
    for(i = 0; i < dev_num; i++) {
        if(!BIT_ISSET(record->active, i + ((DEV_TYPE_SPK == type) ? 0 : SPK_DEV_NUM))) {
            led->set(led, type, i, LED_STATUS_CLOSED);
        }
    }
    
    if(DEV_TYPE_SPK == type) { 
        vocal_sys->sys_evt.req_sync_spk_nwk = STM_FALSE;
    }
    else {    
        vocal_sys->sys_evt.req_sync_mic_nwk = STM_FALSE;
    }
}

static int vocal_sync_rc_cmd(VOCAL_SYS_S *vocal_sys)
{
    int i, idx;
    NWK_DEV_INFO_S  *nwk_dev    = &vocal_sys->mic_dev->nwk_dev[0];
    VOCAL_RECORD_S  *record     = vocal_sys->record;
    MCP2210_DEV_S   *mcp_dev    = vocal_sys->mcp_dev;
    RECORD_VOLUME_S volume      = {0};
    int             ret         = STM_FAILURE;

    for(i = 0; i < MIC_DEV_NUM; i++) {
        if(nwk_dev[i].device_id && nwk_dev[i].cmd_rc) {
            switch(nwk_dev[i].cmd_rc) {
            case OUTPUT_VOLUME_INCREMENT:
                if(nwk_dev[i].volume == 100) {
                    continue;
                }
                nwk_dev[i].volume = (nwk_dev[i].volume >= 95) ? 100 : (nwk_dev[i].volume + 5);
                DEBUG("mic[%d] : ========> db=%d\n", i, nwk_dev[i].volume);
                break;
            case OUTPUT_VOLUME_DECREASE:
                if(nwk_dev[i].volume == 0) {
                    continue;
                }
                nwk_dev[i].volume = (nwk_dev[i].volume <= 5) ? 0 : (nwk_dev[i].volume - 5);
                DEBUG("mic[%d] : ========> db=%d\n", i, nwk_dev[i].volume);
                break;
            case OUTPUT_VOLUME_MUTE:
                nwk_dev[i].mute = nwk_dev[i].mute ? STM_FALSE : STM_TRUE;
                DEBUG("mic[%d] : volume mute=%d\n", i, nwk_dev[i].mute);
                break;
            }
            
            volume.volume  = nwk_dev[i].volume;
            volume.mute    = nwk_dev[i].mute;
            
            if(STM_SUCCESS == record->set(record, nwk_dev[i].device_id, DEV_TYPE_MIC, &volume, &idx)) {
                record->evt.rcd_evt_volume_chg = STM_TRUE;
                mcp_dev->set_info(mcp_dev, SPK_DEV_NUM + idx, &volume);
                ret = STM_SUCCESS;
            }
        }
    }

    mcp_dev->commit(mcp_dev);
    vocal_sys->sys_evt.req_sync_rc_cmd = STM_FALSE;
    
    return ret;
}

static void vocal_sync_pc_cmd(VOCAL_SYS_S *vocal)
{
    int i, idx;
    MCP2210_DEV_S   *mcp_dev    = vocal->mcp_dev;
    CC85XX_DEV_S    *mic_dev    = vocal->mic_dev;
    CC85XX_DEV_S    *spk_dev    = vocal->spk_dev;
    VOCAL_RECORD_S  *record     = vocal->record;
    CC85XX_PAIR_S   *pair       = vocal->pair;

    if(BIT_ISSET(mcp_dev->info.mcp_req, SPK_CFG_NUM + MIC_CFG_NUM + DEV_TYPE_SPK)) {
        DEBUG("pc_cmd spk pair\n");
        pair->set_active(pair, DEV_TYPE_SPK);
        BIT_CLR(mcp_dev->info.mcp_req, SPK_CFG_NUM + MIC_CFG_NUM + DEV_TYPE_SPK);
    }

    if(BIT_ISSET(mcp_dev->info.mcp_req, SPK_CFG_NUM + MIC_CFG_NUM + DEV_TYPE_MIC)) {
        DEBUG("pc_cmd mic pair\n");
        pair->set_active(pair, DEV_TYPE_MIC);
        BIT_CLR(mcp_dev->info.mcp_req, SPK_CFG_NUM + MIC_CFG_NUM + DEV_TYPE_MIC);
    }
    
    if(BIT_ISSET(mcp_dev->info.mcp_req, 0)) {
        for(i = 0; i < SPK_DEV_NUM; i++) {
            if(spk_dev->nwk_dev[i].device_id) {
                spk_dev->nwk_dev[i].volume  = mcp_dev->info.volume[0].volume;
                spk_dev->nwk_dev[i].mute    = mcp_dev->info.volume[0].mute;
                spk_dev->nwk_dev[i].cmd_pc  = STM_TRUE;
                if(STM_SUCCESS == record->set(record,
                        spk_dev->nwk_dev[i].device_id, DEV_TYPE_SPK, &mcp_dev->info.volume[0], &idx)) {
                    record->evt.rcd_evt_volume_chg = STM_TRUE;
                }
            }
        }
        BIT_CLR(mcp_dev->info.mcp_req, 0);
    }

    for(i = 0; i < MIC_DEV_NUM; i++) {
        if(BIT_ISSET(mcp_dev->info.mcp_req, SPK_CFG_NUM + i) && mic_dev->nwk_dev[i].device_id) {
            mic_dev->nwk_dev[i].volume  = mcp_dev->info.volume[SPK_CFG_NUM + i].volume;
            mic_dev->nwk_dev[i].mute    = mcp_dev->info.volume[SPK_CFG_NUM + i].mute;
            mic_dev->nwk_dev[i].cmd_pc  = STM_TRUE;
            if(STM_SUCCESS == record->set(record,
                    mic_dev->nwk_dev[i].device_id, DEV_TYPE_MIC, &mcp_dev->info.volume[SPK_CFG_NUM + i], &idx)) {
                record->evt.rcd_evt_volume_chg = STM_TRUE;
            }
        }
        
        BIT_CLR(mcp_dev->info.mcp_req, SPK_CFG_NUM + i);
    }

    vocal->sys_evt.req_sync_pc_cmd = STM_FALSE;
}

static void vocal_nwk_pairing_callback(void *args)
{
    CC85XX_DEV_S    *dev = (CC85XX_DEV_S *)args;
    VOCAL_SYS_S     *vocal = dev->vocal_sys;

    if(dev->pair_status.task >= TIMERS_NUM) {
        return;
    }
    timer_free(&dev->pair_status.task);
    dev->nwk_pairing(dev, STM_DISABLE);
 
    if(dev->pair_status.dev_idx < SPK_DEV_NUM) {
        vocal->led->set(vocal->led, DEV_TYPE_SPK, dev->pair_status.dev_idx, LED_STATUS_CLOSED);
    }
    else {
        vocal->led->set(vocal->led, DEV_TYPE_MIC, dev->pair_status.dev_idx - SPK_DEV_NUM, LED_STATUS_CLOSED);
    }
}

static void vocal_nwk_pairing(VOCAL_SYS_S *vocal)
{
    int             i, j;
    CC85XX_DEV_S    *dev;
    uint8_t         dev_num;
    CC85XX_PAIR_S   *pair       = vocal->pair;
    VOCAL_RECORD_S  *record     = vocal->record;
    VOCAL_LED_S     *led        = vocal->led;

    for(i = 0; i < sizeof(pair->btn_pair)/sizeof(BUTTON_S); i++) {
        if(pair->btn_pair[i].state.avtice) {
            pair->btn_pair[i].state.avtice = 0;
            dev     = (DEV_TYPE_SPK == i) ? vocal->spk_dev : vocal->mic_dev;
            dev_num = (DEV_TYPE_SPK == i) ? SPK_DEV_NUM : MIC_DEV_NUM;
            for(j = 0; j < dev_num; j++) {
                if(BIT_ISSET(record->active, j + ((DEV_TYPE_SPK == i) ? 0 : SPK_DEV_NUM))
                    || (dev->pair_status.task < TIMERS_NUM)) {
                    continue;
                }
                DEBUG("SYS    : piaring, %s[%d]\n", (DEV_TYPE_SPK == i) ? "SPK" : "MIC", j);
                dev->nwk_pairing(dev, STM_ENABLE);
                dev->pair_status.dev_idx = j + ((DEV_TYPE_SPK == i) ? 0 : SPK_DEV_NUM);
                led->set(led, (VOCAL_DEV_TYPE_E)i, j, LED_STATUS_PAIRING);
                timer_task(&dev->pair_status.task, TMR_ONCE, 10000, 0, vocal_nwk_pairing_callback, dev);
                break;
            }
        }
    }
    vocal->sys_evt.req_pairing = STM_FALSE;
}

static void debug_led_init(void)
{
    GPIO_InitTypeDef    gpio_struct;

    gpio_struct.GPIO_Speed  = GPIO_Speed_Level_3;
    gpio_struct.GPIO_Mode   = GPIO_Mode_OUT;
    gpio_struct.GPIO_PuPd   = GPIO_PuPd_UP;
    gpio_struct.GPIO_OType  = GPIO_OType_OD;
    gpio_struct.GPIO_Pin    = LED_PIN_MIC4;

    GPIO_Init(LED_GPIO_MIC4, &gpio_struct);
    GPIO_SetBits(LED_GPIO_MIC4, gpio_struct.GPIO_Pin);
}

void debug_led_bright(void)
{
    GPIO_ResetBits(LED_GPIO_MIC4, LED_PIN_MIC4);
}

void debug_led_dark(void)
{
    GPIO_SetBits(LED_GPIO_MIC4, LED_PIN_MIC4);
}


static void vocal_reboot(void)
{
    __disable_irq();
    NVIC_SystemReset();
}

static void vocal_exti_init(void)
{
    EXTI_InitTypeDef    exti_cfg = {0};
    NVIC_InitTypeDef    nvic_cfg;
    
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource2);
    exti_cfg.EXTI_Line      = EXTI_Line2;
    exti_cfg.EXTI_Mode      = EXTI_Mode_Interrupt;
    exti_cfg.EXTI_Trigger   = EXTI_Trigger_Rising_Falling;
    exti_cfg.EXTI_LineCmd   = ENABLE;
    EXTI_Init(&exti_cfg);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource8);
    exti_cfg.EXTI_Line      = EXTI_Line8;
    exti_cfg.EXTI_Mode      = EXTI_Mode_Interrupt;
    exti_cfg.EXTI_Trigger   = EXTI_Trigger_Rising_Falling;
    exti_cfg.EXTI_LineCmd   = ENABLE;
    EXTI_Init(&exti_cfg);
#if 1
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4);
    exti_cfg.EXTI_Line      = EXTI_Line4;
    exti_cfg.EXTI_Mode      = EXTI_Mode_Interrupt;
    exti_cfg.EXTI_Trigger   = EXTI_Trigger_Falling;
    exti_cfg.EXTI_LineCmd   = ENABLE;
    EXTI_Init(&exti_cfg);
#endif   
    nvic_cfg.NVIC_IRQChannelPriority    = 3;
    nvic_cfg.NVIC_IRQChannel            = EXTI2_3_IRQn;
    nvic_cfg.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&nvic_cfg);

    nvic_cfg.NVIC_IRQChannelPriority    = 1;
    nvic_cfg.NVIC_IRQChannel            = EXTI4_15_IRQn;
    nvic_cfg.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&nvic_cfg);
}

void vocal_init(VOCAL_SYS_S *vocal)
{
    vocal->sync_nwk_dev = vocal_sync_nwk_dev;
    vocal->sync_rc_cmd  = vocal_sync_rc_cmd;
    vocal->sync_pc_cmd  = vocal_sync_pc_cmd;
    vocal->nwk_pairing  = vocal_nwk_pairing;

    timer_init();
    debug_led_init();
    
    led_init(vocal);
    pair_init(vocal);

    mic_dev_init(vocal);
    spk_dev_init(vocal);

    record_init(vocal);

    vocal->mic_dev->ehif.get_status(&vocal->mic_dev->ehif);
    vocal->spk_dev->ehif.get_status(&vocal->spk_dev->ehif);
    
    mcp_dev_init(vocal);

    vocal_exti_init();
}

void vocal_working(VOCAL_SYS_S *vocal)
{
    while(STM_TRUE) {
        #if 1
        spk_detect(vocal);
        if(vocal->sys_evt.req_sync_spk_nwk) {
            vocal->sync_nwk_dev(vocal, DEV_TYPE_SPK);
            vocal->spk_dev->execute(vocal->spk_dev, DEV_TYPE_SPK, RANG_SET_ALL_DEV);
            vocal->record->commit(vocal->record);
        }
        
        mic_detect(vocal);
        if(vocal->sys_evt.req_sync_mic_nwk) {
            vocal->sync_nwk_dev(vocal, DEV_TYPE_MIC);
            vocal->mic_dev->execute(vocal->mic_dev, DEV_TYPE_MIC, RANG_SET_ALL_DEV);
            vocal->record->commit(vocal->record);
        }
        
        if(vocal->sys_evt.req_sync_rc_cmd && STM_SUCCESS == vocal->sync_rc_cmd(vocal)) {
            vocal->mic_dev->execute(vocal->mic_dev, DEV_TYPE_MIC, RANG_SET_RC_ONLY);
            vocal->record->commit(vocal->record);
        }
        
        if(vocal->sys_evt.req_sync_pc_cmd) {
            vocal->sync_pc_cmd(vocal);
            vocal->spk_dev->execute(vocal->spk_dev, DEV_TYPE_SPK, RANG_SET_PC_ONLY);
            vocal->mic_dev->execute(vocal->mic_dev, DEV_TYPE_MIC, RANG_SET_PC_ONLY);
            vocal->record->commit(vocal->record);
        }
        #endif
        pair_detect();
        if(vocal->sys_evt.req_pairing) {
            vocal->nwk_pairing(vocal);
        }

        if(vocal->sys_evt.req_record_clean) {
            vocal->record->erase(vocal->record);
            debug_led_bright();
            delay_ms(3000);
            vocal_reboot();
        }

        timer_task_process();
    }
}

