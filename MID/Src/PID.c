#include "PID.h"
#include "COM.h"
#include <string.h>
#include <stdio.h>

void PID_Init(PID_Struct* pid,
        float kp, float ki, float kd,
        float intgLim) {
    DT_Init(&pid->dtCtx);
    pid->target = 0;
    pid->curVal = 0;
    pid->targetDelta = 0;
    pid->targetDeltaOld = 0;

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->intgLim = intgLim;

    pid->intgAccum = 0;

    pid->ctrlValDelta = 0;
    pid->ctrlVal = 0;
    pid->ctrlValFloor = 0;
    pid->ctrlValCeil = 0;
}

void PID_Reset(PID_Struct* pid, float target, float ctrlVal) {
    pid->target = target;
    pid->curVal = target;
    pid->targetDelta = 0;
    pid->targetDeltaOld = 0;
    pid->dtCtx.timeOld = 0;
    pid->intgAccum = 0;
    pid->ctrlValDelta = 0;
    pid->ctrlVal = ctrlVal;
}

void PID_Clamp(float* src, float floor, float ceil) {
    if (*src > ceil) {
        *src = ceil;
    } else if (*src < floor) {
        *src = floor;
    }
}

void PID_SetTarget(PID_Struct* pid, float target) {
    pid->target = target;
    pid->targetDelta = pid->targetDeltaOld = target - pid->curVal;
}

void PID_SetCurVal(PID_Struct* pid, float curVal) {
    pid->curVal = curVal;
    pid->targetDelta = pid->target - curVal;
}

float PID_Update(PID_Struct* pid) {
    uint32_t dt_us = DT_GetDeltaTime_us(&pid->dtCtx);
    if (dt_us == 0) {
        return 0;
    }

    float resDelta = 0;
    
    // Kp
    resDelta += pid->targetDelta * pid->kp;
    
    // Ki
    float dt = ((float)dt_us / 1e6f);
    pid->intgAccum += pid->targetDelta * dt;
    PID_Clamp(&pid->intgAccum, -pid->intgLim, pid->intgLim);
    
    resDelta += pid->intgAccum * pid->ki;

    // Kd
    float de = pid->targetDelta - pid->targetDeltaOld;
    
    resDelta += de / dt * pid->kd;

    // Sum
    pid->ctrlValDelta = resDelta;
    pid->ctrlVal += resDelta;
    PID_Clamp(&pid->ctrlVal, pid->ctrlValFloor, pid->ctrlValCeil);
    pid->targetDeltaOld = pid->targetDelta;
    return pid->ctrlVal;
}

float PID_GetCtrlVal(PID_Struct* pid) {
    return pid->ctrlVal;
}

float PID_GetCtrlValDelta(PID_Struct* pid) {
    return pid->ctrlValDelta;
}
