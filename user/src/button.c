#include <button.h>

/**********************************************************************************/
/* Button Interface Functions                                                     */
/**********************************************************************************/

int button_check_active(BUTTON_S *button)
{
    int interval = 0;
    
    /* shack process */
    if(button->state.shack) {
        if((button->state.shack == SHACK_OLD_PRESS  && button->state.press) ||
           (button->state.shack == SHACK_OLD_NORMAL && !button->state.press)) {
            button->state.duration++;
        }
        else {
            button->state.shack = button->state.press ? SHACK_OLD_PRESS : SHACK_OLD_NORMAL;
            button->state.duration = 0;
        }

        if(button->state.duration >= button->interval.shack) {
            button->state.shack     = 0;
            button->state.duration  = 0;
            if(!button->state.effective && button->state.press) {
                button->state.effective = ECT_PRESSED;
                button->state.avtice = 1;
            }
            else if(button->state.effective && !button->state.press) {
                button->state.effective = ECT_LOOSE;
            }
        }
        return button->state.avtice;
    }

    switch(button->state.effective) {
    case ECT_LOOSE:
        if(button->state.press) {
            button->state.shack = SHACK_OLD_PRESS;
        }
        break;
    case ECT_PRESSED:
    case ECT_FOCUSED:
        if(!button->state.press) {
            button->state.shack = SHACK_OLD_NORMAL;
            button->state.duration  = 0;
        }
        else {
            button->state.duration++;
            interval = button->state.effective == ECT_PRESSED ? button->interval.focused : button->interval.pressed;
            if(button->state.duration >= interval) {
                button->state.duration  = 0;
                button->state.effective = ECT_FOCUSED;
                button->state.avtice    = 1;
            }
        }
        break;
    }
    return button->state.avtice;
}




