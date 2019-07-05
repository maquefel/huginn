/*
 ============================================================================
 Name        : Exercicio_05_M7.c
 Author      : Raul Munoz
 Version     :
 Copyright   : Toradex
 Description : Read BTN3 and send status through RPMsg to A7.
 	 	 	   Receive BTN1 status through RPmsg and change LED3 status according to this.
============================================================================
 */

#include <string.h>
#include "assert.h"
#include "board.h"
#include "mu_imx.h"
#include "gpio_imx.h"
#include "gpio_pins.h"
#include "debug_console_imx.h"

/*
 * APP decided interrupt priority
 */
#define APP_MU_IRQ_PRIORITY 3
#define MAX_STRING_SIZE 496         /* Maximum size to hold the data A7 gives */
//Novas Varieaveis

/* Globals */
static struct rpmsg_channel *app_chnl = NULL;
static bool btn_value3 = false;
static bool btn_last_value3 = false;
static char rpmsg_recv_buffer[MAX_STRING_SIZE + 1];

/* Globals */

/**
 * msleep
 *
 * @param num_msec - delay time in ms.
 *
 * This is not an accurate delay, it ensures at least num_msec passed when return.
 */
void msleep(int num_msec)
{
    uint32_t loop;

    /* Recalculate the CPU frequency */
    SystemCoreClockUpdate();

    /* Calculate the CPU loops to delay, each loop has 3 cycles */
    loop = SystemCoreClock / 3 / 1672 * num_msec;

    /* There's some difference among toolchains, 3 or 4 cycles each loop */
    while (loop)
    {
        __NOP();
        loop--;
    }
}

/*
 * MU Interrrupt ISR
 */
void BOARD_MU_HANDLER(void)
{

}

/*!
 * @brief Main function
 */
int main(void)
{
	hardware_init();

	PRINTF("\n\r================= Hello ==================\n\r");

	/*
	 * Prepare for the MU Interrupt
	 *  MU (Messaging Unit) must be initialized before rpmsg init is called
	 */
	MU_Init(BOARD_MU_BASE_ADDR);
	NVIC_SetPriority(BOARD_MU_IRQ_NUM, APP_MU_IRQ_PRIORITY);
	NVIC_EnableIRQ(BOARD_MU_IRQ_NUM);

	/*
	 * str_echo demo loop
	 */
	char tx_buffer[MAX_STRING_SIZE];
	int tx_buffer_len = 0;
	/*
	 * str_echo demo loop
	 */
	while (1)
	{
		msleep(100);
	}
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
