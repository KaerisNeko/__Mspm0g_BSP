#ifndef __PID_H__
#define __PID_H__

#include "ti_msp_dl_config.h"
#include "DT.h"

typedef struct PID_Struct_t {
    float target;
    float curVal;
    float targetDelta;
    float targetDeltaOld;
    
    DT_Context dtCtx;

    float kp;
    float ki;
    float kd;
    float intgLim;
    
    float intgAccum;
    
    float ctrlVal;

    float ctrlValFloor;
    float ctrlValCeil;
}PID_Struct;

void PID_Init(PID_Struct* pid,
        float kp, float ki, float kd,
        float intgLim);

void PID_Reset(PID_Struct* pid, float target, float ctrlVal);

void PID_SetTarget(PID_Struct* pid, float target);
void PID_SetCurVal(PID_Struct* pid, float curVal);
float PID_Update(PID_Struct* pid);
float PID_GetCtrlVal(PID_Struct* pid);

#endif
