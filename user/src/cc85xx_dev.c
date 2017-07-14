#include <cc85xx_dev.h>
#include <vocal_sys.h>

static CC85XX_DEV_S mic_dev;
static CC85XX_DEV_S spk_dev;

void mic_dev_init(VOCAL_SYS_S *vocal_sys)
{
    mic_dev.vocal_sys       = vocal_sys;
    mic_dev.ehif.init       = ehif_init;
    
    mic_dev.ehif.init(&mic_dev.ehif, DEV_TYPE_MIC);

    vocal_sys->mic_dev      = &mic_dev;
}

void spk_dev_init(VOCAL_SYS_S *vocal_sys)
{
    spk_dev.vocal_sys       = vocal_sys;
    spk_dev.ehif.init       = ehif_init;
    
    spk_dev.ehif.init(&spk_dev.ehif, DEV_TYPE_SPK);

    vocal_sys->spk_dev      = &spk_dev;
}

void mic_detect(VOCAL_SYS_S *sys_status)
{
    if(!mic_dev.ehif.status.cmdreq_rdy || mic_dev.ehif.status.pwr_state > 5) {
        mic_dev.ehif.get_status(&mic_dev.ehif);
    }

    if(!mic_dev.nwk_enable) {
        mic_dev.ehif.nwm_control_enable(&mic_dev.ehif, 0);
        //sleep(10);
        mic_dev.ehif.nwm_control_enable(&mic_dev.ehif, 1);
        mic_dev.nwk_enable = 1;
    }

    if(!mic_dev.nwk_stable || mic_dev.ehif.status.evt_nwk_chg) {
        //_nwk_updata(sys_status);
    }
}


