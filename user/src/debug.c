#include <debug.h>

#if defined(DUMP_ENABLE)
void datadump(char *title, void *data, uint32_t len) {
    int     i = 0;
    uint8_t *pdata = (uint8_t *)data;
    
    DEBUG("%s : {", title);
    for(i = 0; i < len; i++) {
        DEBUG("%02x", pdata[i]);
        if(i != len - 1) {
            DEBUG(",");
        }
    }
    DEBUG("}\n");
}
#endif
