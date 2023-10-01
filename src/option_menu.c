#include "global.h"
#include "option_menu.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "scanline_effect.h"
#include "sprite.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "gba/m4a_internal.h"
#include "constants/rgb.h"

#define tMenuSelection data[0]
#define tMenuOffset data[1]

#define tTextSpeed data[2]
#define tBattleSceneOff data[3]
#define tBattleStyle data[4]
#define tSound data[5]
#define tButtonMode data[6]
#define tWindowFrameType data[7]
#define tRandomEncounters data[8]
#define tRandomStarters data[9]
#define tRandomTrainers data[10]


enum
{
    MENUITEM_TEXTSPEED,
    MENUITEM_BATTLESCENE,
    MENUITEM_BATTLESTYLE,
    MENUITEM_SOUND,
    MENUITEM_BUTTONMODE,
    MENUITEM_FRAMETYPE,
    MENUITEM_RANDOM_ENCOUNTERS,
    MENUITEM_RANDOM_STARTERS,
    MENUITEM_RANDOM_TRAINERS,
    MENUITEM_CANCEL,
    MENUITEM_COUNT,
};
#define MENUITEM_DISPLAY_COUNT 7

enum
{
    WIN_HEADER,
    WIN_OPTIONS
};

#define YPOS_TEXTSPEED    (MENUITEM_TEXTSPEED * 16)
#define YPOS_BATTLESCENE  (MENUITEM_BATTLESCENE * 16)
#define YPOS_BATTLESTYLE  (MENUITEM_BATTLESTYLE * 16)
#define YPOS_SOUND        (MENUITEM_SOUND * 16)
#define YPOS_BUTTONMODE   (MENUITEM_BUTTONMODE * 16)
#define YPOS_FRAMETYPE    (MENUITEM_FRAMETYPE * 16)
#define YPOS_RAND_ENCOUNTER    (MENUITEM_RANDOM_ENCOUNTERS * 16)
#define YPOS_RAND_STARTERS    (MENUITEM_RANDOM_STARTERS * 16)
#define YPOS_RAND_TRAINERS    (MENUITEM_RANDOM_TRAINERS * 16)

static void Task_OptionMenuFadeIn(u8 taskId);
static void Task_OptionMenuProcessInput(u8 taskId);
static void Task_OptionMenuSave(u8 taskId);
static void Task_OptionMenuFadeOut(u8 taskId);
static void HighlightOptionMenuItem(u8 selection);
static u8 TextSpeed_ProcessInput(u8 selection);
static void TextSpeed_DrawChoices(u8 selection, u8 taskId);
static u8 BattleScene_ProcessInput(u8 selection);
static void BattleScene_DrawChoices(u8 selection, u8 taskId);
static u8 BattleStyle_ProcessInput(u8 selection);
static void BattleStyle_DrawChoices(u8 selection, u8 taskId);
static u8 Sound_ProcessInput(u8 selection);
static void Sound_DrawChoices(u8 selection, u8 taskId);
static u8 RandomEncounters_ProcessInput(u8 selection);
static void RandomEncounters_DrawChoices(u8 selection, u8 taskId);
static u8 RandomStarters_ProcessInput(u8 selection);
static void RandomStarters_DrawChoices(u8 selection, u8 taskId);
static u8 RandomTrainers_ProcessInput(u8 selection);
static void RandomTrainers_DrawChoices(u8 selection, u8 taskId);
static u8 FrameType_ProcessInput(u8 selection);
static void FrameType_DrawChoices(u8 selection, u8 taskId);
static u8 ButtonMode_ProcessInput(u8 selection);
static void ButtonMode_DrawChoices(u8 selection, u8 taskId);
static void DrawHeaderText(void);
static void DrawOptionMenuTexts(u8 taskId);
static void DrawOptionMenuChoices(u8 taskId);
static void DrawBgWindowFrames(void);

EWRAM_DATA static bool8 sArrowPressed = FALSE;

static const u16 sOptionMenuText_Pal[] = INCBIN_U16("graphics/interface/option_menu_text.gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCBIN_U8("graphics/interface/option_menu_equals_sign.4bpp");

static const u8 *const sOptionMenuItemsNames[MENUITEM_COUNT] =
{
    [MENUITEM_TEXTSPEED]   = gText_TextSpeed,
    [MENUITEM_BATTLESCENE] = gText_BattleScene,
    [MENUITEM_BATTLESTYLE] = gText_BattleStyle,
    [MENUITEM_SOUND]       = gText_Sound,
    [MENUITEM_BUTTONMODE]  = gText_ButtonMode,
    [MENUITEM_FRAMETYPE]   = gText_Frame,
    [MENUITEM_RANDOM_ENCOUNTERS]   = gText_RandomEncounters,
    [MENUITEM_RANDOM_STARTERS]   = gText_RandomStarters,
    [MENUITEM_RANDOM_TRAINERS]   = gText_RandomTrainers,
    [MENUITEM_CANCEL]      = gText_OptionMenuCancel,
};

static const struct WindowTemplate sOptionMenuWinTemplates[] =
{
    [WIN_HEADER] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 2
    },
    [WIN_OPTIONS] = {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 5,
        .width = 26,
        .height = 14,
        .paletteNum = 1,
        .baseBlock = 0x36
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sOptionMenuBgTemplates[] =
{
    {
        .bg = 1,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 0,
        .charBaseIndex = 1,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    }
};

static const u16 sOptionMenuBg_Pal[] = {RGB(17, 18, 31)};

static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

void CB2_InitOptionMenu(void)
{
    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void *)(VRAM), VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
        ChangeBgX(0, 0, BG_COORD_SET);
        ChangeBgY(0, 0, BG_COORD_SET);
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        ChangeBgX(3, 0, BG_COORD_SET);
        ChangeBgY(3, 0, BG_COORD_SET);
        InitWindows(sOptionMenuWinTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_BG0);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_CLR);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_DARKEN);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        gMain.state++;
        break;
    case 4:
        LoadPalette(sOptionMenuBg_Pal, BG_PLTT_ID(0), sizeof(sOptionMenuBg_Pal));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        gMain.state++;
        break;
    case 5:
        LoadPalette(sOptionMenuText_Pal, BG_PLTT_ID(1), sizeof(sOptionMenuText_Pal));
        gMain.state++;
        break;
    case 6:
        PutWindowTilemap(WIN_HEADER);
        DrawHeaderText();
        gMain.state++;
        break;
    case 7:
        gMain.state++;
        break;
    case 8:
        PutWindowTilemap(WIN_OPTIONS);
        DrawOptionMenuTexts(255);
        gMain.state++;
    case 9:
        DrawBgWindowFrames();
        gMain.state++;
        break;
    case 10:
    {
        u8 taskId = CreateTask(Task_OptionMenuFadeIn, 0);

        gTasks[taskId].tMenuSelection = 0;
        gTasks[taskId].tTextSpeed = gSaveBlock2Ptr->optionsTextSpeed;
        gTasks[taskId].tBattleSceneOff = gSaveBlock2Ptr->optionsBattleSceneOff;
        gTasks[taskId].tBattleStyle = gSaveBlock2Ptr->optionsBattleStyle;
        gTasks[taskId].tSound = gSaveBlock2Ptr->optionsSound;
        gTasks[taskId].tButtonMode = gSaveBlock2Ptr->optionsButtonMode;
        gTasks[taskId].tWindowFrameType = gSaveBlock2Ptr->optionsWindowFrameType;
        gTasks[taskId].tRandomEncounters = gSaveBlock2Ptr->optionsRandomEncounters;
        gTasks[taskId].tRandomStarters = gSaveBlock2Ptr->optionsRandomStarters;
        gTasks[taskId].tRandomTrainers = gSaveBlock2Ptr->optionsRandomTrainers;

        DrawOptionMenuChoices(taskId);

        CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
        gMain.state++;
        break;
    }
    case 11:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        return;
    }
}

static void Task_OptionMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_OptionMenuProcessInput;
}

static void Task_OptionMenuProcessInput(u8 taskId)
{
    bool8 offsetChanged;

    if (JOY_NEW(A_BUTTON))
    {
        if (gTasks[taskId].tMenuSelection == MENUITEM_CANCEL)
            gTasks[taskId].func = Task_OptionMenuSave;
    }
    else if (JOY_NEW(B_BUTTON))
    {
        gTasks[taskId].func = Task_OptionMenuSave;
    }
    else if (JOY_NEW(DPAD_UP))
    {
        if (gTasks[taskId].tMenuSelection > 0) {
            gTasks[taskId].tMenuSelection--;
            if (gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuOffset < 0) {
                gTasks[taskId].tMenuOffset--;
                offsetChanged = TRUE;
            }
        } else {
            gTasks[taskId].tMenuSelection = MENUITEM_CANCEL;
            gTasks[taskId].tMenuOffset = MENUITEM_CANCEL - MENUITEM_DISPLAY_COUNT;
            offsetChanged = TRUE;
        }

        if (offsetChanged) {
            DrawOptionMenuTexts(taskId);
            DrawOptionMenuChoices(taskId);
        } else {
            HighlightOptionMenuItem(gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuOffset);
        }
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (gTasks[taskId].tMenuSelection < MENUITEM_CANCEL) {
            gTasks[taskId].tMenuSelection++;
            if (gTasks[taskId].tMenuSelection >= MENUITEM_DISPLAY_COUNT) {
                gTasks[taskId].tMenuOffset++;
                offsetChanged = TRUE;
            }
        } else {
            gTasks[taskId].tMenuSelection = 0;
            gTasks[taskId].tMenuOffset = 0;
            offsetChanged = TRUE;
        }

        if (offsetChanged) {
            DrawOptionMenuTexts(taskId);
            DrawOptionMenuChoices(taskId);
        } else {
            HighlightOptionMenuItem(gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuOffset);
        }
    }
    else
    {
        u8 previousOption;

        switch (gTasks[taskId].tMenuSelection)
        {
        case MENUITEM_TEXTSPEED:
            previousOption = gTasks[taskId].tTextSpeed;
            gTasks[taskId].tTextSpeed = TextSpeed_ProcessInput(gTasks[taskId].tTextSpeed);

            if (previousOption != gTasks[taskId].tTextSpeed)
                TextSpeed_DrawChoices(gTasks[taskId].tTextSpeed, taskId);
            break;
        case MENUITEM_BATTLESCENE:
            previousOption = gTasks[taskId].tBattleSceneOff;
            gTasks[taskId].tBattleSceneOff = BattleScene_ProcessInput(gTasks[taskId].tBattleSceneOff);

            if (previousOption != gTasks[taskId].tBattleSceneOff)
                BattleScene_DrawChoices(gTasks[taskId].tBattleSceneOff, taskId);
            break;
        case MENUITEM_BATTLESTYLE:
            previousOption = gTasks[taskId].tBattleStyle;
            gTasks[taskId].tBattleStyle = BattleStyle_ProcessInput(gTasks[taskId].tBattleStyle);

            if (previousOption != gTasks[taskId].tBattleStyle)
                BattleStyle_DrawChoices(gTasks[taskId].tBattleStyle, taskId);
            break;
        case MENUITEM_SOUND:
            previousOption = gTasks[taskId].tSound;
            gTasks[taskId].tSound = Sound_ProcessInput(gTasks[taskId].tSound);

            if (previousOption != gTasks[taskId].tSound)
                Sound_DrawChoices(gTasks[taskId].tSound, taskId);
            break;
        case MENUITEM_BUTTONMODE:
            previousOption = gTasks[taskId].tButtonMode;
            gTasks[taskId].tButtonMode = ButtonMode_ProcessInput(gTasks[taskId].tButtonMode);

            if (previousOption != gTasks[taskId].tButtonMode)
                ButtonMode_DrawChoices(gTasks[taskId].tButtonMode, taskId);
            break;
        case MENUITEM_FRAMETYPE:
            previousOption = gTasks[taskId].tWindowFrameType;
            gTasks[taskId].tWindowFrameType = FrameType_ProcessInput(gTasks[taskId].tWindowFrameType);

            if (previousOption != gTasks[taskId].tWindowFrameType)
                FrameType_DrawChoices(gTasks[taskId].tWindowFrameType, taskId);
            break;
        case MENUITEM_RANDOM_ENCOUNTERS:
            previousOption = gTasks[taskId].tRandomEncounters;
            gTasks[taskId].tRandomEncounters = RandomEncounters_ProcessInput(gTasks[taskId].tRandomEncounters);

            if (previousOption != gTasks[taskId].tRandomEncounters)
                RandomEncounters_DrawChoices(gTasks[taskId].tRandomEncounters, taskId);
            break;
        case MENUITEM_RANDOM_STARTERS:
            previousOption = gTasks[taskId].tRandomStarters;
            gTasks[taskId].tRandomStarters = RandomStarters_ProcessInput(gTasks[taskId].tRandomStarters);

            if (previousOption != gTasks[taskId].tRandomStarters)
                RandomStarters_DrawChoices(gTasks[taskId].tRandomStarters, taskId);
            break;
        case MENUITEM_RANDOM_TRAINERS:
            previousOption = gTasks[taskId].tRandomTrainers;
            gTasks[taskId].tRandomTrainers = RandomTrainers_ProcessInput(gTasks[taskId].tRandomTrainers);

            if (previousOption != gTasks[taskId].tRandomTrainers)
                RandomTrainers_DrawChoices(gTasks[taskId].tRandomTrainers, taskId);
            break;
        default:
            return;
        }

        if (sArrowPressed) {
            sArrowPressed = FALSE;
            offsetChanged = TRUE;
        }
    }

    if (offsetChanged) {
        CopyWindowToVram(WIN_OPTIONS, COPYWIN_GFX);
    }
}

static void Task_OptionMenuSave(u8 taskId)
{
    gSaveBlock2Ptr->optionsTextSpeed = gTasks[taskId].tTextSpeed;
    gSaveBlock2Ptr->optionsBattleSceneOff = gTasks[taskId].tBattleSceneOff;
    gSaveBlock2Ptr->optionsBattleStyle = gTasks[taskId].tBattleStyle;
    gSaveBlock2Ptr->optionsSound = gTasks[taskId].tSound;
    gSaveBlock2Ptr->optionsButtonMode = gTasks[taskId].tButtonMode;
    gSaveBlock2Ptr->optionsWindowFrameType = gTasks[taskId].tWindowFrameType;
    gSaveBlock2Ptr->optionsRandomEncounters = gTasks[taskId].tRandomEncounters;
    gSaveBlock2Ptr->optionsRandomStarters = gTasks[taskId].tRandomStarters;
    gSaveBlock2Ptr->optionsRandomTrainers = gTasks[taskId].tRandomTrainers;

    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_OptionMenuFadeOut;
}

static void Task_OptionMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

static void HighlightOptionMenuItem(u8 index)
{
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, DISPLAY_WIDTH - 16));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(index * 16 + 40, index * 16 + 56));
}

static void DrawOptionMenuChoice(const u8 *text, u8 x, u8 y, u8 style)
{
    u8 dst[16];
    u16 i;

    for (i = 0; *text != EOS && i < ARRAY_COUNT(dst) - 1; i++)
        dst[i] = *(text++);

    if (style != 0)
    {
        dst[2] = TEXT_COLOR_RED;
        dst[5] = TEXT_COLOR_LIGHT_RED;
    }

    dst[i] = EOS;
    AddTextPrinterParameterized(WIN_OPTIONS, FONT_NORMAL, dst, x, y + 1, TEXT_SKIP_DRAW, NULL);
}

static u8 TextSpeed_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection <= 1)
            selection++;
        else
            selection = 0;

        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = 2;

        sArrowPressed = TRUE;
    }
    return selection;
}

static void TextSpeed_DrawChoices(u8 selection, u8 taskId)
{
    u8 styles[3];
    s32 widthInst, widthMid, widthFast, xMid;

    if (gTasks[taskId].tMenuOffset > MENUITEM_TEXTSPEED) {
        return;
    }

    styles[0] = 0;
    styles[1] = 0;
    styles[2] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_TextSpeedInstant, 104, YPOS_TEXTSPEED, styles[0]);

    widthInst = GetStringWidth(FONT_NORMAL, gText_TextSpeedInstant, 0);
    widthMid = GetStringWidth(FONT_NORMAL, gText_TextSpeedMid, 0);
    widthFast = GetStringWidth(FONT_NORMAL, gText_TextSpeedFast, 0);

    widthMid -= 94;
    xMid = (widthInst - widthMid - widthFast) / 2 + 104;
    DrawOptionMenuChoice(gText_TextSpeedMid, xMid, YPOS_TEXTSPEED, styles[1]);

    DrawOptionMenuChoice(gText_TextSpeedFast, GetStringRightAlignXOffset(FONT_NORMAL, gText_TextSpeedFast, 198), YPOS_TEXTSPEED, styles[2]);
}

static u8 BattleScene_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void BattleScene_DrawChoices(u8 selection, u8 taskId)
{
    u8 styles[2];
    u8 yPos;

    if (gTasks[taskId].tMenuOffset > MENUITEM_BATTLESCENE) {
        return;
    }

    yPos = YPOS_BATTLESCENE - (gTasks[taskId].tMenuOffset * 16);

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_BattleSceneOn, 104, yPos, styles[0]);
    DrawOptionMenuChoice(gText_BattleSceneOff, GetStringRightAlignXOffset(FONT_NORMAL, gText_BattleSceneOff, 198), yPos, styles[1]);
}

static u8 BattleStyle_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void BattleStyle_DrawChoices(u8 selection, u8 taskId)
{
    u8 styles[2];
    u8 yPos;

    if (gTasks[taskId].tMenuOffset > MENUITEM_BATTLESTYLE) {
        return;
    }

    yPos = YPOS_BATTLESTYLE - (gTasks[taskId].tMenuOffset * 16);

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_BattleStyleShift, 104, yPos, styles[0]);
    DrawOptionMenuChoice(gText_BattleStyleSet, GetStringRightAlignXOffset(FONT_NORMAL, gText_BattleStyleSet, 198), yPos, styles[1]);
}

static u8 Sound_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        SetPokemonCryStereo(selection);
        sArrowPressed = TRUE;
    }

    return selection;
}

static void Sound_DrawChoices(u8 selection, u8 taskId)
{
    u8 styles[2];
    u8 yPos;

    if (gTasks[taskId].tMenuOffset > MENUITEM_SOUND) {
        return;
    }

    yPos = YPOS_SOUND - (gTasks[taskId].tMenuOffset * 16);

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_SoundMono, 104, yPos, styles[0]);
    DrawOptionMenuChoice(gText_SoundStereo, GetStringRightAlignXOffset(FONT_NORMAL, gText_SoundStereo, 198), yPos, styles[1]);
}

static u8 RandomEncounters_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection <= 1)
            selection++;
        else
            selection = 0;

        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = 2;

        sArrowPressed = TRUE;
    }
    return selection;
}

static void RandomEncounters_DrawChoices(u8 selection, u8 taskId)
{
    s32 widthOn, widthSeeded, widthOff, xSeeded;
    u8 styles[3];
    u8 yPos;

    if (gTasks[taskId].tMenuOffset > MENUITEM_RANDOM_ENCOUNTERS) {
        return;
    }

    yPos = YPOS_RAND_ENCOUNTER - (gTasks[taskId].tMenuOffset * 16);

    styles[0] = 0;
    styles[1] = 0;
    styles[2] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_RandomEncountersOff, 104, yPos, styles[0]);

    widthOff = GetStringWidth(FONT_NORMAL, gText_RandomEncountersOff, 0);
    widthSeeded = GetStringWidth(FONT_NORMAL, gText_RandomEncountersSeeded, 0);
    widthOn = GetStringWidth(FONT_NORMAL, gText_RandomEncountersOn, 0);

    widthSeeded -= 94;
    xSeeded = (widthOn - widthSeeded - widthOff) / 2 + 114;
    DrawOptionMenuChoice(gText_RandomEncountersSeeded, xSeeded, yPos, styles[1]);

    DrawOptionMenuChoice(gText_RandomEncountersOn, GetStringRightAlignXOffset(FONT_NORMAL, gText_RandomEncountersOn, 198), yPos, styles[2]);
}

static u8 RandomStarters_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void RandomStarters_DrawChoices(u8 selection, u8 taskId)
{
    u8 styles[2];
    u8 yPos;

    if (gTasks[taskId].tMenuOffset > MENUITEM_RANDOM_STARTERS) {
        return;
    }

    yPos = YPOS_RAND_STARTERS - (gTasks[taskId].tMenuOffset * 16);

    styles[0] = 0;
    styles[1] = 0;
    styles[selection ? 0 : 1] = 1;

    DrawOptionMenuChoice(gText_RandomStartersOn, 104, yPos, styles[0]);
    DrawOptionMenuChoice(
        gText_RandomStartersOff,
        GetStringRightAlignXOffset(FONT_NORMAL, gText_RandomStartersOff, 198),
        yPos,
        styles[1]
    );
}

static u8 RandomTrainers_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
        sArrowPressed = TRUE;
    }

    return selection;
}

static void RandomTrainers_DrawChoices(u8 selection, u8 taskId)
{
    u8 styles[2];
    u8 yPos;

    if (gTasks[taskId].tMenuOffset > MENUITEM_RANDOM_TRAINERS) {
        return;
    }

    yPos = YPOS_RAND_TRAINERS - (gTasks[taskId].tMenuOffset * 16);

    styles[0] = 0;
    styles[1] = 0;
    styles[selection ? 0 : 1] = 1;

    DrawOptionMenuChoice(gText_RandomTrainersOn, 104, yPos, styles[0]);
    DrawOptionMenuChoice(
        gText_RandomTrainersOff,
        GetStringRightAlignXOffset(FONT_NORMAL, gText_RandomTrainersOff, 198),
        yPos,
        styles[1]
    );
}

static u8 FrameType_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection < WINDOW_FRAMES_COUNT - 1)
            selection++;
        else
            selection = 0;

        LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
        LoadPalette(GetWindowFrameTilesPal(selection)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = WINDOW_FRAMES_COUNT - 1;

        LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
        LoadPalette(GetWindowFrameTilesPal(selection)->pal, BG_PLTT_ID(7), PLTT_SIZE_4BPP);
        sArrowPressed = TRUE;
    }
    return selection;
}

static void FrameType_DrawChoices(u8 selection, u8 taskId)
{
    u8 text[16];
    u8 n = selection + 1;
    u16 i;
    u8 yPos;

    if (gTasks[taskId].tMenuOffset > MENUITEM_FRAMETYPE) {
        return;
    }

    yPos = YPOS_FRAMETYPE - (gTasks[taskId].tMenuOffset * 16);

    for (i = 0; gText_FrameTypeNumber[i] != EOS && i <= 5; i++)
        text[i] = gText_FrameTypeNumber[i];

    // Convert a number to decimal string
    if (n / 10 != 0)
    {
        text[i] = n / 10 + CHAR_0;
        i++;
        text[i] = n % 10 + CHAR_0;
        i++;
    }
    else
    {
        text[i] = n % 10 + CHAR_0;
        i++;
        text[i] = CHAR_SPACER;
        i++;
    }

    text[i] = EOS;

    DrawOptionMenuChoice(gText_FrameType, 104, yPos, 0);
    DrawOptionMenuChoice(text, 128, yPos, 1);
}

static u8 ButtonMode_ProcessInput(u8 selection)
{
    if (JOY_NEW(DPAD_RIGHT))
    {
        if (selection <= 1)
            selection++;
        else
            selection = 0;

        sArrowPressed = TRUE;
    }
    if (JOY_NEW(DPAD_LEFT))
    {
        if (selection != 0)
            selection--;
        else
            selection = 2;

        sArrowPressed = TRUE;
    }
    return selection;
}

static void ButtonMode_DrawChoices(u8 selection, u8 taskId)
{
    s32 widthNormal, widthLR, widthLA, xLR;
    u8 styles[3];
    u8 yPos;

    if (gTasks[taskId].tMenuOffset > MENUITEM_BUTTONMODE) {
        return;
    }

    yPos = YPOS_BUTTONMODE - (gTasks[taskId].tMenuOffset * 16);

    styles[0] = 0;
    styles[1] = 0;
    styles[2] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_ButtonTypeNormal, 104, yPos, styles[0]);

    widthNormal = GetStringWidth(FONT_NORMAL, gText_ButtonTypeNormal, 0);
    widthLR = GetStringWidth(FONT_NORMAL, gText_ButtonTypeLR, 0);
    widthLA = GetStringWidth(FONT_NORMAL, gText_ButtonTypeLEqualsA, 0);

    widthLR -= 94;
    xLR = (widthNormal - widthLR - widthLA) / 2 + 104;
    DrawOptionMenuChoice(gText_ButtonTypeLR, xLR, yPos, styles[1]);

    DrawOptionMenuChoice(gText_ButtonTypeLEqualsA, GetStringRightAlignXOffset(FONT_NORMAL, gText_ButtonTypeLEqualsA, 198), yPos, styles[2]);
}

static void DrawHeaderText(void)
{
    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(1));
    AddTextPrinterParameterized(WIN_HEADER, FONT_NORMAL, gText_Option, 8, 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(WIN_HEADER, COPYWIN_FULL);
}

static void DrawOptionMenuTexts(u8 taskId)
{
    u8 i, idx;
    u8 scrollOffset = 0;
    if (taskId != 255) {
        scrollOffset = gTasks[taskId].tMenuOffset;
    }

    FillWindowPixelBuffer(WIN_OPTIONS, PIXEL_FILL(1));
    for (i = 0; i < MENUITEM_DISPLAY_COUNT; i++) {
        AddTextPrinterParameterized(
            WIN_OPTIONS, 
            FONT_NORMAL, 
            sOptionMenuItemsNames[i + scrollOffset], 
            8, 
            (i * 16) + 1, 
            TEXT_SKIP_DRAW, 
            NULL
        );
    }
    CopyWindowToVram(WIN_OPTIONS, COPYWIN_FULL);
}

static void DrawOptionMenuChoices(u8 taskId)
{
    TextSpeed_DrawChoices(gTasks[taskId].tTextSpeed, taskId);
    BattleScene_DrawChoices(gTasks[taskId].tBattleSceneOff, taskId);
    BattleStyle_DrawChoices(gTasks[taskId].tBattleStyle, taskId);
    Sound_DrawChoices(gTasks[taskId].tSound, taskId);
    ButtonMode_DrawChoices(gTasks[taskId].tButtonMode, taskId);
    RandomEncounters_DrawChoices(gTasks[taskId].tRandomEncounters, taskId);
    RandomStarters_DrawChoices(gTasks[taskId].tRandomStarters, taskId);
    RandomTrainers_DrawChoices(gTasks[taskId].tRandomTrainers, taskId);
    FrameType_DrawChoices(gTasks[taskId].tWindowFrameType, taskId);
    HighlightOptionMenuItem(gTasks[taskId].tMenuSelection - gTasks[taskId].tMenuOffset);
};

#define TILE_TOP_CORNER_L 0x1A2
#define TILE_TOP_EDGE     0x1A3
#define TILE_TOP_CORNER_R 0x1A4
#define TILE_LEFT_EDGE    0x1A5
#define TILE_RIGHT_EDGE   0x1A7
#define TILE_BOT_CORNER_L 0x1A8
#define TILE_BOT_EDGE     0x1A9
#define TILE_BOT_CORNER_R 0x1AA

static void DrawBgWindowFrames(void)
{
    //                     bg, tile,              x, y, width, height, palNum
    // Draw title window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  0, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  0,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  1,  1,  2,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1,  3,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2,  3, 27,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28,  3,  1,  1,  7);

    // Draw options list window frame
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_L,  1,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_EDGE,      2,  4, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_TOP_CORNER_R, 28,  4,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_LEFT_EDGE,     1,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_RIGHT_EDGE,   28,  5,  1, 18,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_L,  1, 19,  1,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_EDGE,      2, 19, 26,  1,  7);
    FillBgTilemapBufferRect(1, TILE_BOT_CORNER_R, 28, 19,  1,  1,  7);

    CopyBgTilemapBufferToVram(1);
}
