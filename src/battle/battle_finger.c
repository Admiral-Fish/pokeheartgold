#include "battle/battle_finger.h"

#include "global.h"

#include "math_util.h"
#include "unk_0200CF18.h"

static void BattleFinger_Update(SysTask *task, void *data);
static void ov12_0226BEB8(BattleFinger *finger);

void BattleFinger_LoadResources(SpriteRenderer *renderer, SpriteGfxHandler *gfxHandler, HeapID heapId, PaletteData *plttData, u32 character, u32 pal, u32 cell, u32 animation) {
    NARC *narc = NARC_New(NARC_a_1_6_4, heapId);
    sub_0200D68C(plttData, PLTTBUF_SUB_OBJ, renderer, gfxHandler, narc, 1, 0, 1, NNS_G2D_VRAM_TYPE_2DSUB, pal);
    SpriteRenderer_LoadCharResObjFromOpenNarc(renderer, gfxHandler, narc, 0, 0, NNS_G2D_VRAM_TYPE_2DSUB, character);
    SpriteRenderer_LoadCellResObjFromOpenNarc(renderer, gfxHandler, narc, 2, 0, cell);
    SpriteRenderer_LoadAnimResObjFromOpenNarc(renderer, gfxHandler, narc, 3, 0, animation);
    NARC_Delete(narc);
}

void BattleFinger_FreeResources(SpriteGfxHandler *gfxHandler, u32 character, u32 pal, u32 cell, u32 animation) {
    SpriteGfxHandler_UnloadCharObjById(gfxHandler, character);
    SpriteGfxHandler_UnloadPlttObjById(gfxHandler, pal);
    SpriteGfxHandler_UnloadCellObjById(gfxHandler, cell);
    SpriteGfxHandler_UnloadAnimObjById(gfxHandler, animation);
}

static const UnkTemplate_0200D748 ov12_0226EBD4 = {
    .x = 0,
    .y = 0,
    .z = 0,
    .animation = 0,
    .spritePriority = 0,
    .pal = 0,
    .vram = NNS_G2D_VRAM_TYPE_2DSUB,
    .resIdList = { 0, 0, 0, 0, -1, -1 },
    .bgPriority = 0,
    .vramTransfer = 0
};

BattleFinger *BattleFinger_New(SpriteRenderer *renderer, SpriteGfxHandler *gfxHandler, HeapID heapId, u32 character, u32 pal, u32 cell, u32 animation, u32 spritePriority, u32 bgPriority) {
    BattleFinger *finger;
    UnkTemplate_0200D748 unkStruct = ov12_0226EBD4;

    unkStruct.resIdList[0] = character;
    unkStruct.resIdList[1] = pal;
    unkStruct.resIdList[2] = cell;
    unkStruct.resIdList[3] = animation;
    unkStruct.spritePriority = spritePriority;
    unkStruct.bgPriority = bgPriority;

    finger = AllocFromHeap(heapId, sizeof(BattleFinger));
    MI_CpuFill8(finger, 0, sizeof(BattleFinger));

    finger->unk0 = SpriteRenderer_LoadResourcesAndCreateSprite(renderer, gfxHandler, &unkStruct);
    UnkImageStruct_SetSpriteVisibleFlag(finger->unk0, 0);

    finger->unk14 = 0xC0000;
    finger->task = SysTask_CreateOnMainQueue(BattleFinger_Update, finger, 0x3e7);

    return finger;
}

// BattleFinger_Delete
void BattleFinger_Delete(BattleFinger *finger) {
    UnkImageStruct_Delete(finger->unk0);
    SysTask_Destroy(finger->task);
    FreeToHeap(finger);
}

void ov12_0226BCFC(BattleFinger *finger, int x, int y, fx32 a3) {
    ov12_0226BEB8(finger);
    finger->x = x;
    finger->y = y;
    finger->unk14 = a3;
    UnkImageStruct_SetSpritePositionXY_CustomScreenYOffset(finger->unk0, x, y, a3);
    UnkImageStruct_SetSpriteVisibleFlag(finger->unk0, TRUE);
}

void ov12_0226BD2C(BattleFinger *finger, int x, int y) {
    ov12_0226BCFC(finger, x, y, (fx32)0xC0000);
}

void BattleFinger_Disable(BattleFinger *finger) {
    UnkImageStruct_SetSpriteVisibleFlag(finger->unk0, FALSE);
    ov12_0226BEB8(finger);
}

void ov12_0226BD4C(BattleFinger *finger, int delay) {
    finger->delay = delay;
}

BOOL ov12_0226BD50(BattleFinger *finger) {
    return finger->touchOccurrence;
}

static void BattleFinger_Update(SysTask *task, void *data) {
    BattleFinger *finger = (BattleFinger *)data;

    if (finger->touchOccurrence == TRUE) {
        finger->touchOccurrence = FALSE;
    }

    if (finger->delay > 0) {
        finger->delay--;
        if (finger->delay == 0) {
            finger->touchRequest = TRUE;
        }
    }

    if (!UnkImageStruct_GetSpriteVisibleFlag(finger->unk0)) {
        return;
    }

    if (!finger->touchAnimationFlag) {
        int yOffset;
        finger->angle += 1000;
        if (finger->angle >= 18000) {
            finger->angle -= 18000;
            if (finger->touchRequest == TRUE) {
                finger->touchAnimationFlag = TRUE;
                finger->touchRequest = FALSE;
            }
        }

        if (!finger->touchAnimationFlag) {
            yOffset = FX_Mul(GF_SinDegNoWrap(finger->angle / 100), 14 << FX32_SHIFT) / FX32_ONE;
            UnkImageStruct_SetSpritePositionXY_CustomScreenYOffset(finger->unk0, finger->x, finger->y - yOffset, finger->unk14);
        }
    }

    if (finger->touchAnimationFlag == TRUE) {
        switch (finger->touchAnimationState) {
        case 0:
            finger->touchAnimationDelay++;
            if (finger->touchAnimationDelay > 3) {
                finger->touchAnimationDelay = 0;
                finger->touchAnimationState++;
            }
            break;
        case 1:
            UnkImageStruct_SetSpritePositionXY_CustomScreenYOffset(finger->unk0, finger->x, finger->y + 8, finger->unk14);
            finger->touchOccurrence = TRUE;
            finger->touchAnimationState++;
            break;
        case 2:
            finger->touchAnimationDelay++;
            if (finger->touchAnimationDelay > 2) {
                UnkImageStruct_SetSpritePositionXY_CustomScreenYOffset(finger->unk0, finger->x, finger->y + 2, finger->unk14);
                finger->touchAnimationDelay = 0;
                finger->touchAnimationState++;
            }
            break;
        case 3:
            finger->touchAnimationDelay++;
            if (finger->touchAnimationDelay > 2) {
                BattleFinger_Disable(finger);
                finger->touchAnimationDelay = 0;
                finger->touchAnimationState++;
            }
            break;
        default:
            break;
        }
    }

    UnkImageStruct_TickSpriteAnimation1Frame(finger->unk0);
}

static void ov12_0226BEB8(BattleFinger *finger) {
    finger->angle = 0;
    finger->touchAnimationFlag = FALSE;
    finger->touchAnimationState = 0;
    finger->touchAnimationDelay = 0;
}
