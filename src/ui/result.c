#include <swilib.h>
#include <stdlib.h>
#include <string.h>

#define UNITS_N 7

typedef struct {
    WSHDR *ws;
    TDate epoch;
    int seconds_from;
    int seconds_to;
} RESULT_DATA;

extern HEADER_DESC HEADER_D;

enum Unit {
    UNIT_YEARS,
    UNIT_MONTHS,
    UNIT_WEEKS,
    UNIT_DAYS,
    UNIT_HOURS,
    UNIT_MINUTES,
    UNIT_SECONDS,
};

typedef enum Unit Unit;

const char *UNITS[UNITS_N] = {
    "Years",
    "Months",
    "Weeks",
    "Days",
    "Hours",
    "Minutes",
    "Seconds",
};

static SOFTKEY_DESC SOFTKEYS_D[] = {
    {0x0018, 0x0000, (int)LGP_NULL},
    {0x0001, 0x0000, (int)"Back"},
};

static const SOFTKEYSTAB SOFTKEYS_TAB = {
    SOFTKEYS_D, 2,
};

void AppendValueIntoWS(WSHDR *ws, uint32_t value, const char *text) {
    wstrcatprintf(ws, "%u %s", value, text);
    if (value > 1) {
        wsAppendChar(ws, 's');
    }
    wsAppendChar(ws, '\n');
}

void AppendWeekDayIntoWS(WSHDR *ws, int days) {
    if (days > 7) {
        AppendValueIntoWS(ws, days / 7, "week");
        if (days % 7) {
            AppendValueIntoWS(ws, days % 7, "day");
        }
    } else {
        AppendValueIntoWS(ws, days, "day");
    }
}

void PutResultIntoWS(GUI *gui, Unit unit) {
    RESULT_DATA *data = EDIT_GetUserPointer(gui);

    TTime time;
    TDate date, date_from, date_to;
    GetDateTimeFromSeconds(&(data->seconds_from), &date_from, &time, &(data->epoch));
    GetDateTimeFromSeconds(&(data->seconds_to), &date_to, &time, &(data->epoch));
    memcpy(&date, &date_to, sizeof(TDate));
    int64_t days_from = GetDays(&date_from);
    int64_t days_to = GetDays(&date_to);
    int days = (int)(days_to - days_from);
    uint32_t seconds = data->seconds_to - data->seconds_from;

    if (date_from.day != date.day) {
        DateAddDays(&date, -((int)date_from.day));
    } else {
        date.day = 0;
    }
    if (date_from.month != date.month) {
        DateAddMonths(&date, -((int)date_from.month));
    } else {
        date.month = 0;
    }
    if (date_from.year != date.year) {
        DateAddYears(&date, -((int)date_from.year));
    } else {
        date.year = 0;
    }

    wsprintf(data->ws, "%c%c%c", UTF16_TEXT_COLOR, PC_FOREGROUND, UTF16_FONT_MEDIUM)
    switch (unit) {
        case UNIT_YEARS: default:
            AppendValueIntoWS(data->ws, date.year, "year");
            if (date.month) {
                AppendValueIntoWS(data->ws, date.month, "month");
            } if (date.day) {
                AppendWeekDayIntoWS(data->ws, date.day);
            }
        break;
        case UNIT_MONTHS:
            AppendValueIntoWS(data->ws, date.year * 12 + date.month, "month");
            if (date.day) {
                AppendWeekDayIntoWS(data->ws, date.day);
            }
        break;
        case UNIT_WEEKS:
            AppendValueIntoWS(data->ws, days / 7, "week");
            if (days % 7) {
                AppendValueIntoWS(data->ws, days % 7, "day");
            }
        break;
        case UNIT_DAYS:
            AppendValueIntoWS(data->ws, days, "day");
        break;
        case UNIT_HOURS:
            AppendValueIntoWS(data->ws, seconds / 60 / 60, "hour");
        break;
        case UNIT_MINUTES:
            AppendValueIntoWS(data->ws, seconds / 60, "minute");
        break;
        case UNIT_SECONDS:
            AppendValueIntoWS(data->ws, seconds, "second");
        break;
    }
    wsAppendChar(data->ws, UTF16_FONT_RESET);
}

static void GHook(GUI *gui, int cmd) {
    RESULT_DATA *data = EDIT_GetUserPointer(gui);

    int i = EDIT_GetItemNumInFocusedComboBox(gui);
    if (cmd == TI_CMD_REDRAW) {
        PutResultIntoWS(gui, i - 1);
        EDIT_SetTextToEditControl(gui, 3, data->ws);
        SetSoftKey(gui, &SOFTKEYS_D[0], SET_LEFT_SOFTKEY);
        SetSoftKey(gui, &SOFTKEYS_D[1], SET_RIGHT_SOFTKEY);
    }
    else if (cmd == TI_CMD_CREATE) {
        data->ws = AllocWS(128);
        PutResultIntoWS(gui, UNIT_YEARS);
        EDIT_SetTextToEditControl(gui, 3, data->ws);
    } else if (cmd == TI_CMD_COMBOBOX_FOCUS) {
        if (i) {
            wsprintf(data->ws, UNITS[i - 1]);
        } else {
            wsprintf(data->ws, "%s", "Unit")
        }
        EDIT_SetTextToFocused(gui, data->ws);
    } else if (cmd == TI_CMD_DESTROY) {
        FreeWS(data->ws);
        mfree(data);
    }
}

static INPUTDIA_DESC INPUTDIA_D = {
    8,
    NULL,
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

int CreateResultUI(TDate *date_from, TDate *date_to, const TDate *epoch) {
    memcpy(&(HEADER_D.rc), GetHeaderRECT(), sizeof(RECT));
    memcpy(&(INPUTDIA_D.rc), GetMainAreaRECT(), sizeof(RECT));

    TTime time;
    RESULT_DATA *data = malloc(sizeof(RESULT_DATA));
    memcpy(&(data->epoch), epoch, sizeof(TDate));
    InitTime(&time, 1, 0, 0, 0);
    GetSecondsFromDateTime(&(data->seconds_from), date_from, &time, epoch);
    GetSecondsFromDateTime(&(data->seconds_to), date_to, &time, epoch);

    void *ma = malloc_adr();
    void *eq = AllocEQueue(ma, mfree_adr());

    WSHDR ws;
    uint16_t wsbody[128];
    CreateLocalWS(&ws, wsbody, 127);

    EDITCONTROL ec;
    PrepareEditControl(&ec);

    wsprintf(&ws, "%s", "Unit:");
    ConstructEditControl(&ec, ECT_HEADER, ECF_APPEND_EOL, &ws, wstrlen(&ws));
    AddEditControlToEditQend(eq, &ec, ma);

    wsprintf(&ws, "%s", UNITS[UNIT_YEARS]);
    ConstructComboBox(&ec,ECT_COMBO_BOX,ECF_APPEND_EOL, &ws, 127, 0,
            UNITS_N, UNIT_YEARS + 1);
    AddEditControlToEditQend(eq, &ec, ma);

    CutWSTR(&ws, 0);
    ConstructEditControl(&ec, ECT_HEADER, ECF_NORMAL_STR, &ws, 127);
    AddEditControlToEditQend(eq, &ec, ma);

    return CreateInputTextDialog(&INPUTDIA_D, &HEADER_D, eq, 1, (void*)data);
}
