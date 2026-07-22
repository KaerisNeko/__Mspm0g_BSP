#include "DT.h"
#include "HRT.h"

void DT_Init(DT_Context* ctx) {
    ctx->timeOld = 0;
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

// void DT_UpdateRateSyncDelay(DT_Context* ctx, uint16_t rate) {
//     uint32_t delayTime_us = 1000000 / rate;
//     while (1) {
//         if (ctx->timeOld == 0) {
//             ctx->timeOld = HRT_GetTime_us();
//             continue;
//         }
//         uint32_t dt = HRT_GetTime_us() - ctx->timeOld;
//         if (dt >= delayTime_us) {
//             ctx->timeOld += dt;
//             break;
//         }
//     }
// }
