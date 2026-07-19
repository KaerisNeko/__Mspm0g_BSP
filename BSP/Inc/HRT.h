#ifndef __HRT_H__
#define __HRT_H__

#include <ti_msp_dl_config.h>

#define HRT_TIM_INST            TIMER_HRT_INST
#define HRT_TIM_IT_HANDLER      TIMER_HRT_INST_IRQHandler
#define HRT_TIM_IT_IRQN         TIMER_HRT_INST_INT_IRQN
#define HRT_TIM_IT_IIDX         DL_TIMER_IIDX_LOAD

void HRT_Init(void);

uint32_t HRT_GetTime_us(void);

#endif
