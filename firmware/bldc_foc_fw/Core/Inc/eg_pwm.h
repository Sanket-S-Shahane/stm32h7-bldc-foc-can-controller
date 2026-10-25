/**
 * @file    eg_pwm.h
 * @brief   Three-phase PWM driver on TIM1 CH1..CH3 (center-aligned) - public interface.
 * @details Drives the three half-bridge inputs (IN1..IN3) of the motor driver board.
 *          Duty is given in permille (0..1000 = 0..100 %) so the caller needs no
 *          knowledge of the timer period (ARR) chosen in the .ioc file.
 *
 * @par Execution context
 *      eg_pwm_init() and eg_pwm_stop(): MAIN CONTEXT ONLY.
 *      eg_pwm_set_duty_permille(): main context, or from a periodic control ISR after
 *      init - it is one register write and shares no RAM state, so no race exists.
 *
 * @par MISRA C:2023 deviations
 *      DEV-EG-001: the vendor HAL macros (__HAL_TIM_*) are function-like macros
 *      (Dir 4.9, advisory). Their use is confined to eg_pwm.c.
 */

#ifndef EG_PWM_H                          /* include guard: prevents double inclusion (Dir 4.10) */
#define EG_PWM_H                          /* define the guard symbol */

#include <stdint.h>                       /* fixed-width integer types: uint8_t, uint16_t */

/** @brief Number of PWM phases (three half-bridges). U suffix: unsigned constant (Rule 7.2). */
#define EG_PWM_PHASE_COUNT          (3U)

/** @brief Duty scale: 1000 permille equals 100 % duty. */
#define EG_PWM_DUTY_MAX_PERMILLE    (1000U)

/** @brief Result codes returned by this module. */
typedef enum                              /* enumeration of every possible outcome */
{
    EG_PWM_OK = 0,                        /* call succeeded */
    EG_PWM_ERR_PARAM = 1,                 /* caller passed an out-of-range argument; nothing was changed */
    EG_PWM_ERR_HAL = 2                    /* the vendor HAL reported a failure */
} eg_pwm_status_t;                        /* type name: snake_case with eg_ prefix and _t suffix */

/**
 * @brief  Start all three PWM channels at 0 % duty.
 * @note   Main context only. Call once after MX_TIM1_Init().
 * @retval EG_PWM_OK on success, EG_PWM_ERR_HAL if any channel failed to start.
 */
eg_pwm_status_t eg_pwm_init(void);

/**
 * @brief  Set the duty of one phase.
 * @param  phase           Phase index 0..2 (maps to TIM1 CH1..CH3).
 * @param  duty_permille   Duty 0..1000 (1000 = 100 %).
 * @retval EG_PWM_OK on success, EG_PWM_ERR_PARAM if an argument is out of range.
 */
eg_pwm_status_t eg_pwm_set_duty_permille(uint8_t phase, uint16_t duty_permille);

/**
 * @brief  Set all phases to 0 % duty.
 * @note   Main context only. This does NOT disconnect the driver; a true coast
 *         needs the driver enable (EN) pin, which a later module will own.
 */
void eg_pwm_stop(void);

#endif                                    /* EG_PWM_H */
