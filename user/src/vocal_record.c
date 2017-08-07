#include <vocal_record.h>
#include <stm32f0xx_flash.h>
#include <debug.h>
#include <cc85xx_dev.h>

#define STM32_TAIL_ADDR         0x08008000
#define STM32_PAGE_SIZE         0x400

#define RCD_ID_PAGE_NUM         1
#define RCD_VL_PAGE_NUM         1

#define RCD_VL_PAGE_ADDR        (STM32_TAIL_ADDR - RCD_VL_PAGE_NUM * STM32_PAGE_SIZE)
#define RCD_ID_PAGE_ADDR        (RCD_VL_PAGE_ADDR - RCD_VL_PAGE_NUM * STM32_PAGE_SIZE)

#define RCD_ID_SIZE             (sizeof(uint32_t) * (4 + 2) + 2)
#define RCD_VL_SIZE             (sizeof(uint8_t) * 3 + 1)

#define _ITRM_ADDR(idx, type)   (RCD_##type##_PAGE_ADDR + \
                                (idx) / (STM32_PAGE_SIZE / RCD_##type##_SIZE) * STM32_PAGE_SIZE + \
                                (idx) % (STM32_PAGE_SIZE / RCD_##type##_SIZE) * RCD_##type##_SIZE)

#define _FOR_RECORD_ITEM_ADDR(idx, pos, type) \
                                for(*(idx) = 0, *(pos) = _ITRM_ADDR(*(idx), type); \
                                    *(idx) < (STM32_PAGE_SIZE / RCD_##type##_SIZE) * RCD_ID_PAGE_NUM; \
                                    *(idx) += 1, *(pos) = _ITRM_ADDR(*(idx), type))

#define FOR_RECORD_ITEM_ADDR_ID(idx, pos)  _FOR_RECORD_ITEM_ADDR((idx), (pos), ID)
#define FOR_RECORD_ITEM_ADDR_VL(idx, pos)  _FOR_RECORD_ITEM_ADDR((idx), (pos), VL)

#define FLASH_ReadHalfWord(Address, pData)  (*(pData) = *(uint16_t *)(Address))
#define FLASH_ReadWord(Address, pData)      (*(pData) = *(uint32_t *)(Address))

#define _flash_read_halfword(addr, pdata)    FLASH_ReadHalfWord(addr, pdata)
#define _flash_read_word(addr, pdata)        FLASH_ReadWord(addr, pdata)

static void _flash_write_halfword(uint32_t addr, uint16_t data)
{  
    FLASH_Unlock();
    FLASH_ProgramHalfWord(addr, data);
    FLASH_Lock();
}

static void _flash_write_word(uint32_t addr, uint16_t data)
{
    FLASH_Unlock();
    FLASH_ProgramWord(addr, data);
    FLASH_Lock();
}

static void _record_dev_id_page_erase(void)
{
    int i = 0;

    FLASH_Unlock();
    for(i = 0; i < RCD_ID_PAGE_NUM; i++) {
        #if 0
        FLASH_ErasePage(RCD_ID_PAGE_ADDR + i * STM32_PAGE_SIZE);
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

    FLASH_Unlock();
    for(i = 0; i < RCD_VL_PAGE_NUM; i++) {
        #if 0
        FLASH_ErasePage(RCD_VL_PAGE_ADDR + i * STM32_PAGE_SIZE);
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

static void record_dev_id_item_print(int index, uint32_t base_addr)
{
    int         i = 0;
    uint32_t    data = 0;

    _flash_read_halfword(base_addr, &data);
    DEBUG("0x%08x: 0x%04x", base_addr, data);
    
    base_addr += sizeof(uint16_t);
    for(i = 0; i < MIC_DEV_NUM; i++) {
        _flash_read_word(base_addr + i * sizeof(uint32_t), &data);
        DEBUG(", M[%d]=0x%08x", i, data);

    }
    for(; i < MIC_DEV_NUM + SPK_DEV_NUM; i++) {
        _flash_read_word(base_addr + i * sizeof(uint32_t), &data);
        DEBUG(", S[%d]=0x%08x", i, data);
    }
    DEBUG("\n");
}

static void record_volume_item_print(int index, uint32_t base_addr)
{
    uint16_t    data = 0;

    _flash_read_halfword(base_addr, &data);
    DEBUG("0x%08x: 0x%04x", base_addr, data);

    DEBUG(", S=%02x", (data >> 8) & 0xff);
    
    _flash_read_word(base_addr + sizeof(uint16_t), &data);
    DEBUG(", M[0]=0x%02x", data & 0xff);
    DEBUG(", M[1]=0x%02x\n", (data >> 8) & 0xff);
}


static void record_dev_id_flash_print(void)
{
    int         item_idx;
    uint32_t    item_base_addr;

    DEBUG("dev_id_flash_print\n");
    FOR_RECORD_ITEM_ADDR_ID(&item_idx, &item_base_addr) {
        record_dev_id_item_print(item_idx, item_base_addr);
    }
}

static void record_volume_flash_print(void)
{
    int         item_idx;
    uint32_t    item_base_addr;

    DEBUG("volume_flash_print\n");
    FOR_RECORD_ITEM_ADDR_VL(&item_idx, &item_base_addr) {
        record_volume_item_print(item_idx, item_base_addr);
    }
}

static void record_dev_id_write(VOCAL_RECORD_S *record)
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
        _flash_write_word(item_base_addr + i * sizeof(uint32_t), *(uint32_t *)&record->mic_id[i]);
    }
    for(; i < MIC_DEV_NUM + SPK_DEV_NUM; i++) {
        _flash_write_word(item_base_addr + i * sizeof(uint32_t), *(uint32_t *)&record->spk_id[i - MIC_DEV_NUM]);
    }
}

static void record_volume_write(VOCAL_RECORD_S *record)
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
    _flash_write_halfword(item_base_addr + sizeof(uint16_t), *(uint16_t *)&record->mic_vl);
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
        if(flag == 0) {
            continue;
        }            
        /* using */
        if(flag == 0xa5) {
            DEBUG("Found dev_id record\n");
            record->addr_idx_id = item_idx;

            item_base_addr += sizeof(uint16_t);
            for(i = 0; i < MIC_DEV_NUM; i++) {
                _flash_read_word(item_base_addr + i * sizeof(uint32_t), (uint32_t *)&record->mic_id[i]);
                record->vocal_sys->mic_dev->nwk_dev[i].device_id = record->mic_id[i].device_id;
                record->vocal_sys->mic_dev->nwk_dev[i].slot      = i;
            }
            for(; i < MIC_DEV_NUM + SPK_DEV_NUM; i++) {
                _flash_read_word(item_base_addr + i * sizeof(uint32_t), (uint32_t *)&record->spk_id[i - MIC_DEV_NUM]);
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
            
            record_dev_id_write(record);
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

    DEBUG("record_volume_init\n");
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
            record_volume_write(record);

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

void record_init(VOCAL_SYS_S *vocal_sys)
{
    record.vocal_sys    = vocal_sys;

#if 1
    _record_dev_id_page_erase();
    _record_volume_page_erase();
#endif

    record.dev_id_init  = record_dev_id_init;
    record.volume_init  = record_volume_init;
    record.dev_id_write = record_dev_id_write;
    record.volume_write = record_volume_write;

    vocal_sys->record   = &record;

    record.dev_id_init(&record);
    record.volume_init(&record);

    //record_dev_id_flash_print();
    //record_volume_flash_print();

    //for(int i = 0; i < 43; i++) {
    //    vocal_sys->record->dev_id_write(vocal_sys->record);
    //}

    //for(int i = 0; i < 260; i++) {
    //    record_volume_write(&record);
    //}

    //record_dev_id_flash_print();
    //record_volume_flash_print();

    record.dev_id_init(&record);
    record.volume_init(&record);

    record_info_print(&record);
}

void write_record(uint32_t device_id, uint16_t volume, uint8_t mute, uint8_t record_id)
{

}
