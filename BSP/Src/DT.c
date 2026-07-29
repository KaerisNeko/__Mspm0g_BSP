#include "DT.h"
#include "HRT.h"

void DT_Init(DT_Context* ctx) {
    ctx->timeOld = 0;
    ctx->presc = 1;
    ctx->cnt = 0;
}

uint32_t DT_GetDeltaTime_us(DT_Context* ctx) {
    if (ctx->timeOld == 0) {
        ctx->timeOld = HRT_GetTime_us();
        return 0;
    }
    uint32_t dt = HRT_GetTime_us() - ctx->timeOld;
    ctx->timeOld += dt;
    return dt;
}

void DT_UpdateRateSyncDelay(DT_Context* ctx, uint16_t rate) {
    uint32_t delayTime_us = 1000000 / rate;
    if (ctx->timeOld == 0) {
        ctx->timeOld = HRT_GetTime_us();
    }
    uint32_t endTime = ctx->timeOld + delayTime_us;
    while (HRT_GetTime_us() < endTime);
    ctx->timeOld = endTime;
}

uint8_t DT_FreqPrescale(DT_Context* ctx) {
    if (++(ctx->cnt) >= ctx->presc) {
        ctx->cnt = 0;
        return 1;
    }
    return 0;
}
