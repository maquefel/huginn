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

#include "rpmsg/rpmsg_ext.h"
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

static void rpmsg_read_cb(struct rpmsg_channel *rp_chnl, void *data, int len,
		void * priv, unsigned long src)
{
	memcpy((void*) rpmsg_recv_buffer, data, len);
	rpmsg_recv_buffer[len] = 0;
	PRINTF("rpmsg_recv_buffer: %s\r\n", rpmsg_recv_buffer);
	char * pch;
	pch = strstr(rpmsg_recv_buffer, "BTN1=");
	if(pch)
	{
		pch = pch + 5;
		if(*pch == '1')
		{
			GPIO_WritePinOutput(gpioLed3.base,gpioLed3.pin, (gpio_pin_action_t) 1);
		}
		else if(*pch == '0')
		{
			GPIO_WritePinOutput(gpioLed3.base,gpioLed3.pin, (gpio_pin_action_t) 0);
		}
	}
}

/* rpmsg_rx_callback will call into this for a channel creation event*/
static void rpmsg_channel_created(struct rpmsg_channel *rp_chnl)
{
	/* We should give the created rp_chnl handler to app layer */
	app_chnl = rp_chnl;
	PRINTF("Name service handshake is done, M4 has setup a rpmsg channel [%d ---> %d]\r\n", app_chnl->src, app_chnl->dst);
}

static void rpmsg_channel_deleted(struct rpmsg_channel *rp_chnl)
{
	rpmsg_destroy_ept(rp_chnl->rp_ept);
}

/*
 * MU Interrrupt ISR
 */
void BOARD_MU_HANDLER(void)
{
	/*
	 * calls into rpmsg_handler provided by middleware
	 */
	rpmsg_handler();
}

/*!
 * @brief Main function
 */
int main(void)
{
	struct remote_device *rdev;
	hardware_init();

    GPIO_Init(gpioLed3.base, &Led3);
    GPIO_Init(gpioBtn3.base, &Btn3);

	PRINTF("\n\r================= GPIO Functionality==================\n\r");

	//Zera Led3
	GPIO_WritePinOutput(gpioLed3.base,gpioLed3.pin, (gpio_pin_action_t) btn_value3);

	btn_value3  = GPIO_ReadPinInput(gpioBtn3.base, gpioBtn3.pin);
	btn_last_value3 = btn_value3;
	PRINTF("eBotao3 Value %d\n\n\r", btn_value3);

	/*
	 * Prepare for the MU Interrupt
	 *  MU (Messaging Unit) must be initialized before rpmsg init is called
	 */
	MU_Init(BOARD_MU_BASE_ADDR);
	NVIC_SetPriority(BOARD_MU_IRQ_NUM, APP_MU_IRQ_PRIORITY);
	NVIC_EnableIRQ(BOARD_MU_IRQ_NUM);

	/* Print the initial banner */
	PRINTF("\r\nRPMSG String Echo Bare Metal Demo...\r\n");

	/* RPMSG Init as REMOTE */
	PRINTF("RPMSG Init as Remote\r\n");
	rpmsg_init(0, &rdev, rpmsg_channel_created, rpmsg_channel_deleted, rpmsg_read_cb, RPMSG_MASTER);

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
		btn_value3  = GPIO_ReadPinInput(gpioBtn3.base, gpioBtn3.pin);
		if(btn_value3 != btn_last_value3)
		{
			btn_last_value3 = btn_value3;
			sprintf(tx_buffer, "BTN3=%d\n", btn_value3);
			tx_buffer_len = strlen(tx_buffer);
			PRINTF("Sending : \"%s\" [len : %d]\r\n", tx_buffer, tx_buffer_len);
			rpmsg_send(app_chnl, tx_buffer, tx_buffer_len);
		}
		msleep(100);
	}
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
