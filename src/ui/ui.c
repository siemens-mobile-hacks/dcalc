#include <swilib.h>
#include <string.h>

#include "icons.h"
#include "result.h"

int ICON[] = {ICON_DATE_CALCULATOR};

HEADER_DESC HEADER_D = {{0, 0, 0, 0}, ICON, (int)"Date calculator", LGP_NULL};

static SOFTKEY_DESC SOFTKEYS_D[] = {
    {0x0018, 0x0000, (int)"Calc"},
    {0x0001, 0x0000, (int)"Exit"},
};

static const SOFTKEYSTAB SOFTKEYS_TAB = {
    SOFTKEYS_D, 2,
};

static int OnKey(GUI *gui, GUI_MSG *msg) {
    if (msg->keys == 0x18) {
        TDate d1, d2;
        EDIT_GetDate(gui, 2, &d1);
        EDIT_GetDate(gui, 4, &d2);

        TDate *date_from, *date_to, epoch;
        if (CmpDates(&d1, &d2) == -1) {
            date_from = &d1;
            date_to = &d2;
        } else {
            date_from = &d2;
            date_to = &d1;
        }
        if (date_to->year - date_from->year <= 130) {
            InitDate(&epoch, date_from->year - 1, 1, 1);
            CreateResultUI(date_from, date_to, &epoch);
        } else {
            MsgBoxError(1, (int)"Date period is too long!");
        }
    }
    return 0;
}

static void GHook(GUI *gui, int cmd) {
    if (cmd == TI_CMD_REDRAW) {
        SetSoftKey(gui, &SOFTKEYS_D[0], SET_LEFT_SOFTKEY);
        SetSoftKey(gui, &SOFTKEYS_D[1], SET_RIGHT_SOFTKEY);
    }
}

static INPUTDIA_DESC INPUTDIA_D = {
    8,
    OnKey,
    GHook,
    NULL,
    0,
    &SOFTKEYS_TAB,
    {0, 0, 0, 0},
    FONT_MEDIUM,
    100,
    101,
    0,
     0,
    INPUTDIA_FLAGS_SWAP_SOFTKEYS,
};

int CreateUI() {
    memcpy(&(HEADER_D.rc), GetHeaderRECT(), sizeof(RECT));
    memcpy(&(INPUTDIA_D.rc), GetMainAreaRECT(), sizeof(RECT));

    void *ma = malloc_adr();
    void *eq = AllocEQueue(ma, mfree_adr());

    WSHDR ws;
    uint16_t wsbody[128];
    CreateLocalWS(&ws, wsbody, 127);

    EDITCONTROL ec;
    PrepareEditControl(&ec);

    TDate date;
    GetDateTime(&date, NULL);

    CutWSTR(&ws, 0);
    wsprintf(&ws, "%s", "From:");
    ConstructEditControl(&ec, ECT_HEADER, ECF_APPEND_EOL, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    ConstructEditControl(&ec, ECT_CALENDAR, ECF_APPEND_EOL, NULL, 0);
    ConstructEditDate(&ec, &date);
    AddEditControlToEditQend(eq, &ec, ma);

    CutWSTR(&ws, 0);
    wsprintf(&ws, "%s", "Until:");
    ConstructEditControl(&ec, ECT_HEADER, ECF_APPEND_EOL, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    ConstructEditControl(&ec, ECT_CALENDAR, ECF_APPEND_EOL, NULL, 0);
    ConstructEditDate(&ec, &date);
    AddEditControlToEditQend(eq, &ec, ma);

    return CreateInputTextDialog(&INPUTDIA_D, &HEADER_D, eq, 1, NULL);
}
