#include <string.h>

#include <vocal_record.h>
#include <stm32f0xx_flash.h>
#include <debug.h>
#include <cc85xx_dev.h>
#include <vocal_common.h>

#define STM32_TAIL_ADDR         0x08008000
#define STM32_PAGE_SIZE         0x400

#define RCD_ID_PAGE_NUM         1
#define RCD_VL_PAGE_NUM         15

#define RCD_ID_PAGE_ADDR        (STM32_TAIL_ADDR - RCD_ID_PAGE_NUM * STM32_PAGE_SIZE)
#define RCD_VL_PAGE_ADDR        (RCD_ID_PAGE_ADDR - RCD_VL_PAGE_NUM * STM32_PAGE_SIZE)

#define RCD_ID_SIZE             (sizeof(uint32_t) * (4 + 2) + 2)
#define RCD_VL_SIZE             (sizeof(uint8_t) * 3 + 1)

#define _ITRM_ADDR(idx, type)   (RCD_##type##_PAGE_ADDR + \
                                (idx) / (STM32_PAGE_SIZE / RCD_##type##_SIZE) * STM32_PAGE_SIZE + \
                                (idx) % (STM32_PAGE_SIZE / RCD_##type##_SIZE) * RCD_##type##_SIZE)

#define _FOR_RECORD_ITEM_ADDR(idx, pos, type) \
                                for(*(idx) = 0, *(pos) = _ITRM_ADDR(*(idx), type); \
                                    *(idx) < (STM32_PAGE_SIZE / RCD_##type##_SIZE) * RCD_##type##_PAGE_NUM; \
                                    *(idx) += 1, *(pos) = _ITRM_ADDR(*(idx), type))

#define FOR_RECORD_ITEM_ADDR_ID(idx, pos)  _FOR_RECORD_ITEM_ADDR((idx), (pos), ID)
#define FOR_RECORD_ITEM_ADDR_VL(idx, pos)  _FOR_RECORD_ITEM_ADDR((idx), (pos), VL)

#define FLASH_ReadHalfWord(Address, pData)  (*(uint16_t *)(pData) = *(uint16_t *)(Address))
#define FLASH_ReadWord(Address, pData)  \
{                                                   \
    uint16_t *pd = (uint16_t *)(pData);             \
    uint16_t *pa = (uint16_t *)(Address);           \
    FLASH_ReadHalfWord(pa++, pd++);                 \
    FLASH_ReadHalfWord(pa, pd);                     \
}

#define _flash_read_halfword(addr, pdata)    FLASH_ReadHalfWord(addr, pdata)
#define _flash_read_word(addr, pdata)        FLASH_ReadWord(addr, pdata)

static void _flash_write_halfword(uint32_t addr, uint16_t data)
{
    FLASH_Status status;
    
    FLASH_Unlock();
    status = FLASH_ProgramHalfWord(addr, data);
    FLASH_Lock();
    if(status != FLASH_COMPLETE) {
        DEBUG("wh:status=%d\n", status);
        error_reboot();
    }
}

static void _flash_write_word(uint32_t addr, uint32_t data)
{
    FLASH_Status status;

    FLASH_Unlock();
    status = FLASH_ProgramWord(addr, data);
    FLASH_Lock();
    if(status != FLASH_COMPLETE) {
        DEBUG("wh:status=%d\n", status);
        error_reboot();
    }
}

static void _record_dev_id_page_erase(void)
{
    int i = 0;
    FLASH_Status status;

    FLASH_Unlock();
    for(i = 0; i < RCD_ID_PAGE_NUM; i++) {
        #if 1
        status = FLASH_ErasePage(RCD_ID_PAGE_ADDR + i * STM32_PAGE_SIZE);
        if(status != FLASH_COMPLETE) {
            error_reboot();
        }
        #else
        for(int j = 0; j < STM32_PAGE_SIZE; j++) {
            *(uint8_t *)(RCD_ID_PAGE_ADDR + j) = 0xff;
        }
        #endif
    }
    FLASH_Lock();
}

static void _record_volume_page_erase(void)
{
    int i = 0;
    FLASH_Status status;
    
    FLASH_Unlock();
    for(i = 0; i < RCD_VL_PAGE_NUM; i++) {
        #if 1
        status = FLASH_ErasePage(RCD_VL_PAGE_ADDR + i * STM32_PAGE_SIZE);
        if(status != FLASH_COMPLETE) {
            error_reboot();
        }
        #else
        for(int j = 0; j < STM32_PAGE_SIZE; j++) {
            *(uint8_t *)(RCD_VL_PAGE_ADDR + j) = 0xff;
        }
        #endif
    }
    FLASH_Lock();
}

static VOCAL_RECORD_S record;

static void record_info_print(VOCAL_RECORD_S *record)
{
    int i = 0;

    for(i = 0; i < MIC_DEV_NUM; i++) {
        DEBUG("MIC[%d]:[%d]id=0x%08x, mute=%d, volume=%d\n",
                i, record->mic_id[i].index, record->mic_id[i].device_id, record->mic_vl[i].mute, record->mic_vl[i].volume);
    }
    for(i = 0; i < SPK_DEV_NUM; i++) {
        DEBUG("SPK[%d]:[%d]id=0x%08x, mute=%d, volume=%d\n",
                i, record->spk_id[i].index, record->spk_id[i].device_id, record->spk_vl.mute, record->spk_vl.volume);
    }
}

static void _record_dev_id_write(VOCAL_RECORD_S *record)
{
    int         i = 0;
    uint32_t    item_base_addr;

    if(record->addr_idx_id == -1) {
        record->addr_idx_id = 0;
    }
    else if(record->addr_idx_id == (STM32_PAGE_SIZE / RCD_ID_SIZE) * RCD_ID_PAGE_NUM - 1) {
        _record_dev_id_page_erase();
        record->addr_idx_id = 0;
    }
    else {
        _flash_write_halfword(_ITRM_ADDR(record->addr_idx_id, ID), 0);
        record->addr_idx_id++;
    }

    item_base_addr = _ITRM_ADDR(record->addr_idx_id, ID);
    _flash_write_halfword(item_base_addr, 0xffa5);
    
    item_base_addr += sizeof(uint16_t);

    for(i = 0; i < MIC_DEV_NUM; i++) {
        DEBUG("RECORD : [0x%08x]wirte : MIC[%d]=0x%08x\n", item_base_addr + i * sizeof(uint32_t),
                i, *(uint32_t *)&record->mic_id[i]);
        _flash_write_word(item_base_addr + i * sizeof(uint32_t), *(uint32_t *)&record->mic_id[i]);
    }
    for(; i < MIC_DEV_NUM + SPK_DEV_NUM; i++) {
        DEBUG("RECORD : [0x%08x]wirte : SPK[%d]=0x%08x\n", item_base_addr + i * sizeof(uint32_t),
                i - MIC_DEV_NUM, *(uint32_t *)&record->spk_id[i - MIC_DEV_NUM]);
        _flash_write_word(item_base_addr + i * sizeof(uint32_t), *(uint32_t *)&record->spk_id[i - MIC_DEV_NUM]);
    }
}

static void _record_volume_write(VOCAL_RECORD_S *record)
{
    uint32_t    item_base_addr;
    uint16_t    data;

    if(record->addr_idx_vl == -1) {
        record->addr_idx_vl = 0;
    }
    else if(record->addr_idx_vl == (STM32_PAGE_SIZE / RCD_VL_SIZE) * RCD_VL_PAGE_NUM - 1) {
        _record_volume_page_erase();
        record->addr_idx_vl = 0;
    }
    else {
        _flash_write_halfword(_ITRM_ADDR(record->addr_idx_vl, VL), 0);
        record->addr_idx_vl++;
    }

    item_base_addr = _ITRM_ADDR(record->addr_idx_vl, VL);
    data = *(uint8_t *)&record->spk_vl;
    
    data <<= 8;
    data |= 0xa5;
    
    _flash_write_halfword(item_base_addr, data);
    _flash_write_halfword(item_base_addr + sizeof(uint16_t), non_align_data16(&record->mic_vl));
    DEBUG("RECORD : write address 0x%08x\n", item_base_addr);
}


static void record_dev_id_init(VOCAL_RECORD_S *record)
{
    int         item_idx;
    int         i;
    uint8_t     flag = 0;
    uint32_t    item_base_addr;

    DEBUG("record_dev_id_init\n");
    FOR_RECORD_ITEM_ADDR_ID(&item_idx, &item_base_addr) {

        _flash_read_halfword(item_base_addr, &flag);
        DEBUG("0x%08x: flag=0x%02x\n", item_base_addr, flag);

        /* used */
        if(flag == 0x0) {
            continue;
        }            
        /* using */
        if(flag == 0xa5) {
            DEBUG("Found dev_id record\n");
            record->addr_idx_id = item_idx;

            item_base_addr += sizeof(uint16_t);
            for(i = 0; i < MIC_DEV_NUM; i++) {
                _flash_read_word(item_base_addr + i * sizeof(uint32_t), (uint32_t *)&record->mic_id[i]);
                DEBUG("ADDR   : [0x%08x]\n", item_base_addr + i * sizeof(uint32_t));
                datadump("", (void *)(item_base_addr + i * sizeof(uint32_t)), sizeof(4));
                record->vocal_sys->mic_dev->nwk_dev[i].device_id = record->mic_id[i].device_id;
                record->vocal_sys->mic_dev->nwk_dev[i].slot      = i;
            }
            for(; i < MIC_DEV_NUM + SPK_DEV_NUM; i++) {
                _flash_read_word(item_base_addr + i * sizeof(uint32_t), (uint32_t *)&record->spk_id[i - MIC_DEV_NUM]);
                DEBUG("ADDR   : [0x%08x]\n", item_base_addr + i * sizeof(uint32_t));
                datadump("", (void *)(item_base_addr + i * sizeof(uint32_t)), sizeof(4));
                record->vocal_sys->spk_dev->nwk_dev[i - MIC_DEV_NUM].device_id  = record->spk_id[i - MIC_DEV_NUM].device_id;
                record->vocal_sys->spk_dev->nwk_dev[i - MIC_DEV_NUM].slot       = i - MIC_DEV_NUM;
            }
            return;
        }
        /* new */
        if(flag == 0xff) {
            DEBUG("Init a dev_id record space\n");
            record->addr_idx_id = -1;
            for(i = 0; i < MIC_DEV_NUM; i++) {
                record->mic_id[i].index     = i;
                record->mic_id[i].device_id = i;
                record->vocal_sys->mic_dev->nwk_dev[i].device_id = record->mic_id[i].device_id;
                record->vocal_sys->mic_dev->nwk_dev[i].slot      = i;
            }
            for(; i < MIC_DEV_NUM + SPK_DEV_NUM; i++) {
                record->spk_id[i - MIC_DEV_NUM].index     = i - MIC_DEV_NUM;
                record->spk_id[i - MIC_DEV_NUM].device_id = i;
                record->vocal_sys->spk_dev->nwk_dev[i - MIC_DEV_NUM].device_id  = record->spk_id[i - MIC_DEV_NUM].device_id;
                record->vocal_sys->spk_dev->nwk_dev[i - MIC_DEV_NUM].slot       = i - MIC_DEV_NUM;
            }
            
            _record_dev_id_write(record);
            return;
        }
    }
}

static void record_volume_init(VOCAL_RECORD_S *record)
{
    int         item_idx;
    int         i = 0;
    uint16_t    data = 0;
    uint8_t     flag = 0;
    uint8_t     volume;
    uint32_t    item_base_addr;

    //DEBUG("record_volume_init\n");
    FOR_RECORD_ITEM_ADDR_VL(&item_idx, &item_base_addr) {

        _flash_read_halfword(item_base_addr, &data);
        flag = data & 0xff;
        DEBUG("0x%08x: 0x%04x, flag=0x%02x\n", item_base_addr, data, flag);

        /* used */
        if(flag == 0) {
            continue;
        }
        /* using */
        if(flag == 0xa5) {
            DEBUG("Found volume record\n");
            record->addr_idx_vl = item_idx;

            volume              = (data >> 8) & 0xff;
            record->spk_vl      = *(RECORD_VOLUME_S *)(&volume);

            _flash_read_halfword(item_base_addr + sizeof(uint16_t), &data);
            DEBUG("ADDR   : [0x%08x]\n", item_base_addr + sizeof(uint16_t));
            volume              = data & 0xff;
            record->mic_vl[0]   = *(RECORD_VOLUME_S *)(&volume);
            volume              = (data >> 8) & 0xff;
            record->mic_vl[1]   = *(RECORD_VOLUME_S *)(&volume);

            for(i = 0; i < MIC_DEV_NUM; i++) {
                record->vocal_sys->mic_dev->nwk_dev[i].mute     = record->mic_vl[i].mute;
                record->vocal_sys->mic_dev->nwk_dev[i].volume   = record->mic_vl[i].volume;
            }
            for(i = 0; i < SPK_DEV_NUM; i++) {
                record->vocal_sys->spk_dev->nwk_dev[i].mute     = record->spk_vl.mute;
                record->vocal_sys->spk_dev->nwk_dev[i].volume   = record->spk_vl.volume;
            }
            return;
        }
        /* new */
        if(flag == 0xff) {
            DEBUG("Init a volume record space\n");
            record->addr_idx_vl     = -1;
            record->spk_vl.mute     = 1;
            record->spk_vl.volume   = 75;
            record->mic_vl[0].volume   = 1;
            record->mic_vl[1].volume   = 2;
            _record_volume_write(record);

            for(i = 0; i < MIC_DEV_NUM; i++) {
                record->vocal_sys->mic_dev->nwk_dev[i].mute     = record->mic_vl[i].mute;
                record->vocal_sys->mic_dev->nwk_dev[i].volume   = record->mic_vl[i].volume;
            }
            for(i = 0; i < SPK_DEV_NUM; i++) {
                record->vocal_sys->spk_dev->nwk_dev[i].mute     = record->spk_vl.mute;
                record->vocal_sys->spk_dev->nwk_dev[i].volume   = record->spk_vl.volume;
            }
            return;
        }
    }
}

static int record_add(VOCAL_RECORD_S *record, const uint32_t device_id, const VOCAL_DEV_TYPE_E type,
                                        RECORD_VOLUME_S *volume, int *idx)
{
    int i;

    switch(type) {
    case DEV_TYPE_SPK:
        for(i = 0; i < SPK_DEV_NUM; i++) {
            if(BIT_ISSET(record->active, i)) {
                continue;
            }
            record->spk_id[i].device_id = device_id;
            memcpy(&record->spk_vl, volume, sizeof(RECORD_VOLUME_S));
            *idx = i;
            BIT_SET(record->active, i);
            return STM_SUCCESS;
        }
        break;
    case DEV_TYPE_MIC:
        for(i = 0; i < MIC_DEV_NUM; i++) {
            if(BIT_ISSET(record->active, SPK_DEV_NUM + i)) {
                continue;
            }
            record->mic_id[i].device_id = device_id;
            memcpy(&record->mic_vl[i], volume, sizeof(RECORD_VOLUME_S));
            *idx = i;
            BIT_SET(record->active, SPK_DEV_NUM + i);
            return STM_SUCCESS;
        }
        break;
    default:

        break;
    }

    return STM_FAILURE;
}

typedef enum rcd_get_or_set_e
{
    RCD_GET = 0,
    RCD_SET,
} RCD_GET_OR_SET_E;

static int _record_rcd_get_or_set(VOCAL_RECORD_S *record, RCD_GET_OR_SET_E ctrl, const uint32_t device_id, const VOCAL_DEV_TYPE_E type,
                                        RECORD_VOLUME_S *volume, int *idx)
{
    RECORD_VOLUME_S *dest = 0;
    RECORD_VOLUME_S *src = 0;
    RECORD_VOLUME_S *vl_head = (DEV_TYPE_SPK == type) ? &record->spk_vl     : &record->mic_vl[0];
    RECORD_DEV_ID_S *id_head = (DEV_TYPE_SPK == type) ? &record->spk_id[0]  : &record->mic_id[0];
    int i, ix;
    int dev_num = (DEV_TYPE_SPK == type) ? SPK_DEV_NUM : MIC_DEV_NUM;
    
    for(i = 0; i < dev_num; i++) {
        if(device_id == id_head[i].device_id) {
            ix      = (DEV_TYPE_SPK == type) ? 0 : i;
            dest    = ctrl ? &vl_head[ix] : volume;
            src     = ctrl ? volume : &vl_head[ix];
            memcpy(dest, src, sizeof(RECORD_VOLUME_S));
            *idx = i;
            BIT_SET(record->active, ((DEV_TYPE_SPK == type) ? 0 : SPK_DEV_NUM) + i);
            return STM_SUCCESS;
        }
    }

    return STM_FAILURE;
}

static int record_get(VOCAL_RECORD_S *record, const uint32_t device_id, const VOCAL_DEV_TYPE_E type,
                                        RECORD_VOLUME_S *volume, int *idx)
{
    return _record_rcd_get_or_set(record, RCD_GET, device_id, type, volume, idx);
}

static int record_set(VOCAL_RECORD_S *record, const uint32_t device_id, const VOCAL_DEV_TYPE_E type,
                                    const RECORD_VOLUME_S *volume, int *idx)
{
    return _record_rcd_get_or_set(record, RCD_SET, device_id, type, (RECORD_VOLUME_S *)volume, idx);
}

static int record_commit(VOCAL_RECORD_S *record)
{
    if(record->evt.rcd_evt_dev_id_chg) {
        _record_dev_id_write(record);
        record->evt.rcd_evt_dev_id_chg = STM_FALSE;
    }

    if(record->evt.rcd_evt_volume_chg) {
        _record_volume_write(record);
        record->evt.rcd_evt_volume_chg = STM_FALSE;
    }

    return STM_TRUE;
}

static void record_erase(VOCAL_RECORD_S *record)
{
    _record_dev_id_page_erase();
    _record_volume_page_erase();
}

void record_init(VOCAL_SYS_S *vocal_sys)
{
    record.vocal_sys    = vocal_sys;

    record.dev_id_init  = record_dev_id_init;
    record.volume_init  = record_volume_init;
    record.add          = record_add;
    record.get          = record_get;
    record.set          = record_set;
    record.commit       = record_commit;
    record.erase        = record_erase;

    vocal_sys->record   = &record;

	//record.erase(&record);

    record.dev_id_init(&record);
    record.volume_init(&record);

    record_info_print(&record);
}

