/**
 * @file    eg_pwm.c
 * @brief   Three-phase PWM driver implementation (TIM1, vendor HAL based).
 * @details See eg_pwm.h for the interface, execution context and deviations.
 */

#include "eg_pwm.h"                       /* our own interface first, so a broken header fails early */
#include "tim.h"                          /* CubeMX header: declares htim1 and pulls in the HAL */

/**
 * @brief Maps phase index to HAL channel. const: lives in flash, cannot be altered at run time.
 *        File scope, internal linkage (static): used by three functions, hidden from other files.
 */
static const uint32_t eg_pwm_channel_table[EG_PWM_PHASE_COUNT] =
{
    TIM_CHANNEL_1,                        /* phase 0 -> TIM1 CH1 */
    TIM_CHANNEL_2,                        /* phase 1 -> TIM1 CH2 */
    TIM_CHANNEL_3                         /* phase 2 -> TIM1 CH3 */
};

eg_pwm_status_t eg_pwm_init(void)
{
    eg_pwm_status_t status = EG_PWM_OK;   /* assume success until a HAL call fails */
    uint8_t idx;                          /* loop counter over the phases */

    for (idx = 0U; idx < EG_PWM_PHASE_COUNT; idx++)   /* visit every phase exactly once */
    {
        /* Load 0 % first so the output never glitches high when the channel starts. */
        __HAL_TIM_SET_COMPARE(&htim1, eg_pwm_channel_table[idx], 0U);

        /* Start the channel. On TIM1 this also sets MOE (main output enable). */
        if (HAL_TIM_PWM_Start(&htim1, eg_pwm_channel_table[idx]) != HAL_OK)
        {
            status = EG_PWM_ERR_HAL;      /* remember the failure but keep starting the others */
        }
    }

    return status;                        /* single exit point (Rule 15.5) */
}

eg_pwm_status_t eg_pwm_set_duty_permille(uint8_t phase, uint16_t duty_permille)
{
    eg_pwm_status_t status = EG_PWM_OK;   /* default result */
    uint32_t period_ticks;                /* number of timer ticks in one counting direction (ARR + 1) */
    uint32_t compare_value;               /* value to load into the capture/compare register */

    /* Validate both arguments before touching hardware or indexing the table. */
    if ((phase >= EG_PWM_PHASE_COUNT) ||
        ((uint32_t)duty_permille > EG_PWM_DUTY_MAX_PERMILLE))
    {
        status = EG_PWM_ERR_PARAM;        /* reject and change nothing */
    }
    else
    {
        /* Read ARR so the maths follows whatever period the .ioc file sets. */
        period_ticks = __HAL_TIM_GET_AUTORELOAD(&htim1) + 1U;

        /* Scale: ticks * permille / 1000. TIM1 is 16 bit, so the largest product is
         * 65536 * 1000, which fits in 32 bits and cannot overflow. */
        compare_value = (period_ticks * (uint32_t)duty_permille) / EG_PWM_DUTY_MAX_PERMILLE;

        /* Write the compare value. Preload is enabled, so it takes effect at the next update event. */
        __HAL_TIM_SET_COMPARE(&htim1, eg_pwm_channel_table[phase], compare_value);
    }

    return status;                        /* single exit point */
}

void eg_pwm_stop(void)
{
    uint8_t idx;                          /* loop counter over the phases */

    for (idx = 0U; idx < EG_PWM_PHASE_COUNT; idx++)   /* every phase */
    {
        /* 0 % duty: high side off, low side on. This brakes the motor; it does not coast. */
        __HAL_TIM_SET_COMPARE(&htim1, eg_pwm_channel_table[idx], 0U);
    }
}
