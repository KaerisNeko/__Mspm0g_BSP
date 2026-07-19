#include "HRT.h"
#include <core_cm0plus.h>

volatile uint32_t HRT_cnt_ms = 0;
volatile uint32_t HRT_cnt_1024us = 0;

void HRT_Init(void) {
    HRT_cnt_ms = 0;
    HRT_cnt_1024us = 0;
    NVIC_EnableIRQ(HRT_TIM_IT_IRQN);
}

uint32_t HRT_GetTime_us(void) {
    return (HRT_cnt_1024us << 10) | DL_TimerG_getTimerCount(HRT_TIM_INST);
}



void SysTick_Handler(void) {
    HRT_cnt_ms++;
}

void HRT_TIM_IT_HANDLER(void) {
    uint32_t iidx = DL_TimerG_getPendingInterrupt(HRT_TIM_INST);
    if (iidx == HRT_TIM_IT_IIDX) {
        HRT_cnt_1024us++;
    }
}
