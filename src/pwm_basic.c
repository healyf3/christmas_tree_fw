/**
 * \file
 *
 * \brief PWM Basic driver implementation.
 *
 (c) 2020 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms,you may use this software and
    any derivatives exclusively with Microchip products.It is your responsibility
    to comply with third party license terms applicable to your use of third party
    software (including open source software) that may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

/**
 * \defgroup doc_driver_pwm_basic PWM Basic Driver
 * \ingroup doc_driver_pwm
 *
 * \section doc_driver_pwm_normal_rev Revision History
 * - v0.0.0.1 Initial Commit
 *
 *@{
 */
#include <pwm_basic.h>

/** Function pointer to callback function called by IRQ.
    NULL=default value: No callback function is to be used.
*/
pwm_irq_cb_t PWM_0_cb = NULL;

/**
 * \brief Initialize PWM
 * If module is configured to disabled state, the clock to the PWM is disabled
 * if this is supported by the device's clock system.
 *
 * \return Initialization status.
 */
// int8_t PWM_0_init()
// {
// 
// 	/* Enable TC2 */
// 	PRR &= ~(1 << PRTIM2);
// 
// 	TCCR2A = (0 << COM2A1) | (0 << COM2A0)   /* Normal port operation, OCA disconnected */
// 	         | (0 << COM2B1) | (0 << COM2B0) /* Normal port operation, OCB disconnected */
// 	         | (0 << WGM21) | (1 << WGM20);  /* TC8 Mode 1 Phase Correct */
// 
// 	// TCCR2B = 0 /* TC8 Mode 1 Phase Correct */
// 	//		 | (0 << CS22) | (0 << CS21) | (0 << CS20); /* No clock source (Timer/Counter stopped) */
// 
// 	// TIMSK2 = 0 << OCIE2B /* Output Compare B Match Interrupt Enable: disabled */
// 	//		 | 0 << OCIE2A /* Output Compare A Match Interrupt Enable: disabled */
// 	//		 | 0 << TOIE2; /* Overflow Interrupt Enable: disabled */
// 
// 	return 0;
// }

/**
 * \brief Enable PWM_0
 * 1. If supported by the clock system, enables the clock to the PWM
 * 2. Enables the PWM module by setting the enable-bit in the PWM control register
 *
 * \return Nothing
 */
void PWM_0_enable()
{
	/* Enable TC2 */
	PRR &= ~(1 << PRTIM2);
}

/**
 * \brief Disable PWM_0
 * 1. Disables the PWM module by clearing the enable-bit in the PWM control register
 * 2. If supported by the clock system, disables the clock to the PWM
 *
 * \return Nothing
 */
void PWM_0_disable()
{
	/* Disable TC2 */
	PRR |= (1 << PRTIM2);
}

/**
 * \brief Enable PWM output on channel 0
 *
 * \return Nothing
 */
void PWM_0_enable_output_ch0()
{

	TCCR2A |= ((0 << COM2A1) | (0 << COM2A0));
}

/**
 * \brief Disable PWM output on channel 0
 *
 * \return Nothing
 */
void PWM_0_disable_output_ch0()
{

	TCCR2A &= ~((0 << COM2A1) | (0 << COM2A0));
}

/**
 * \brief Enable PWM output on channel 1
 *
 * \return Nothing
 */
void PWM_0_enable_output_ch1()
{

	TCCR2A |= ((0 << COM2B1) | (0 << COM2B0));
}

/**
 * \brief Disable PWM output on channel 1
 *
 * \return Nothing
 */
void PWM_0_disable_output_ch1()
{

	TCCR2A &= ~((0 << COM2B1) | (0 << COM2B0));
}

/**
 * \brief Load COUNTER register in PWM_0
 *
 * \param[in] counter_value The value to load into COUNTER
 *
 * \return Nothing
 */
void PWM_0_load_counter(PWM_0_register_t counter_value)
{
	TCNT2 = counter_value;
}

/**
 * \brief Load duty cycle register in for channel 0.
 * The physical register may have different names, depending on the hardware.
 * This is not the duty cycle as percentage of the whole period, but the actual
 * counter compare value.
 *
 * \param[in] counter_value The value to load into the duty cycle register.
 *
 * \return Nothing
 */
void PWM_0_load_duty_cycle_ch0(PWM_0_register_t duty_value)
{
	OCR2A = duty_value;
}

/**
 * \brief Load duty cycle register in for channel 1.
 * The physical register may have different names, depending on the hardware.
 * This is not the duty cycle as percentage of the whole period, but the actual
 * counter compare value.
 *
 * \param[in] counter_value The value to load into the duty cycle register.
 *
 * \return Nothing
 */
void PWM_0_load_duty_cycle_ch1(PWM_0_register_t duty_value)
{
	OCR2B = duty_value;
}

/**
 * \brief Register a callback function to be called at the end of the overflow ISR.
 *
 * \param[in] f Pointer to function to be called
 *
 * \return Nothing.
 */
void PWM_0_register_callback(pwm_irq_cb_t f)
{
	PWM_0_cb = f;
}

ISR(TIMER2_OVF_vect)
{
	static volatile uint8_t callback_count = 0;

	// Clear the interrupt flag
	TIFR2 |= (1 << TOV2);

	// callback function - called every 0 passes
	if ((++callback_count >= PWM_0_INTERRUPT_CB_RATE) && (PWM_0_INTERRUPT_CB_RATE != 0)) {
		if (PWM_0_cb != NULL) {
			PWM_0_cb();
		}
	}
}
