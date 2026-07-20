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
