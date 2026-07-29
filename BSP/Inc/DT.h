#ifndef __DT_H__
#define __DT_H__

#include "ti_msp_dl_config.h"

typedef struct DT_Context_t {
    uint32_t timeOld;
    uint16_t presc;
    uint16_t cnt;
}DT_Context;

void DT_Init(DT_Context* ctx);
uint32_t DT_GetDeltaTime_us(DT_Context* ctx);
void DT_UpdateRateSyncDelay(DT_Context* ctx, uint16_t rate);
uint8_t DT_FreqPrescale(DT_Context* ctx);

#endif
