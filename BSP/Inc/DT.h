#ifndef __DT_H__
#define __DT_H__

#include "ti_msp_dl_config.h"

typedef struct DT_Context_t {
    uint32_t timeOld;
}DT_Context;

void DT_Init(DT_Context* ctx);
uint32_t DT_GetDeltaTime_us(DT_Context* ctx);
// void DT_UpdateRateSyncDelay(DT_Context* ctx, uint16_t rate);

#endif
