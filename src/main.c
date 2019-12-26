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

#include "rpmsg_lite.h"
#include "rpmsg_queue.h"
#include "rpmsg_ns.h"

#include "imx7_colibri_m4/rsc_table.h"

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
static struct rpmsg_lite_instance instance;
static struct rpmsg_lite_ept_static_context ept_context;

struct rpmsg_lite_endpoint *ctrl_ept;
rpmsg_queue_handle ctrl_q;

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
    rpmsg_handler();
}

#define OCRAM_EPDC 0x20220000
#define TC_LOCAL_EPT_ADDR (30)
#define TC_REMOTE_EPT_ADDR (40)

void* rpmsg_lite_base = OCRAM_EPDC;

struct rpmsg_lite_endpoint *ctrl_ept;
rpmsg_queue_handle ctrl_q;
struct rpmsg_lite_instance *my_rpmsg = NULL;
static void* rx_cb_data;

int my_callback(void *payload, int payload_len, unsigned long src, void *priv)
{
	PRINTF("\n\rmy_callback\n\r");
    return 0;
}


/*!
 * @brief Main function
 */
int main(void)
{
    int recved = 0;
    unsigned long src;
    char buf[256];
    int len;

    hardware_init();

    PRINTF("\n\r================= Hello ==================\n\r");

    /*
     * Prepare for the MU Interrupt
     *  MU (Messaging Unit) must be initialized before rpmsg init is called
     */
    MU_Init(BOARD_MU_BASE_ADDR);
    NVIC_SetPriority(BOARD_MU_IRQ_NUM, APP_MU_IRQ_PRIORITY);
    NVIC_EnableIRQ(BOARD_MU_IRQ_NUM);

    int ret = env_init();
    if(ret == -1)
    {
        PRINTF("env_init failed... Goto fail...\n\r");
        goto fail;
    }

    struct remote_resource_table* table = (struct remote_resource_table*)get_resource_table(0, sizeof(struct remote_resource_table));

	PRINTF("RING_TX=0x%x\n\r", table->rpmsg_vring0.da);
	PRINTF("RING_RX=0x%x\n\r", table->rpmsg_vring1.da);

	rpmsg_lite_base = table->rpmsg_vring0.da;

    my_rpmsg = rpmsg_lite_remote_init(rpmsg_lite_base, RL_PLATFORM_IMX7D_M4_LINK_ID, RL_NO_FLAGS, &instance);

    if(my_rpmsg == NULL)
    {
        PRINTF("rpmsg_lite_remote_init failed... Goto fail...\n\r");
        goto fail;
    }

    //my_rpmsg->tvq
	//my_rpmsg->rvq

    //ctrl_q = rpmsg_queue_create(my_rpmsg);


//    struct rpmsg_lite_endpoint *rpmsg_lite_create_ept(struct rpmsg_lite_instance *rpmsg_lite_dev,
//                                                      unsigned long addr,
//                                                      rl_ept_rx_cb_t rx_cb,
//                                                      void *rx_cb_data,
//                                                      struct rpmsg_lite_ept_static_context *ept_context);

	//ctrl_q = rpmsg_queue_create(my_rpmsg);

	ctrl_ept = rpmsg_lite_create_ept(my_rpmsg, RL_ADDR_ANY, my_callback, rx_cb_data, &ept_context);

	if(ctrl_ept == RL_NULL)
	{
		PRINTF("rpmsg_lite_create_ept failed... Goto fail...\n\r");
		goto fail;
	}

    PRINTF("Waiting for master to get ready...\r\n");

    while(!rpmsg_lite_is_link_up(my_rpmsg))
    {
        PRINTF(".");
        msleep(300);
    }

    PRINTF("\r\n");
    PRINTF("Sending name service announcement to Linux...\r\n");
	if(rpmsg_ns_announce(my_rpmsg, ctrl_ept, "rpmsg-openamp-demo-channel", RL_NS_CREATE) != RL_SUCCESS)
	{
		PRINTF("rpmsg_ns_announce failed... Goto fail...\n\r");
		goto fail;
	}
    PRINTF("Waiting for any messages from Linux...\r\n");
    while(1)
    {
        //rpmsg_queue_recv(my_rpmsg, ctrl_q, &src, (char*)buf, 256, &recved, RL_BLOCK);
        PRINTF("\n\n\rFrom endpoint %d received %d bytes:\n\r", (int)src, recved);
        PRINTF(buf);
        len = sprintf(buf, "Oh you don't say number %d!", (int)src);
        PRINTF("\n\n\rSending %d bytes to endpoint %d:\n\r ", len, src);
        PRINTF(buf);
        rpmsg_lite_send(my_rpmsg, ctrl_ept, src, buf, len, RL_BLOCK);
    }


    rpmsg_lite_destroy_ept(my_rpmsg, ctrl_ept);
    //rpmsg_queue_destroy(my_rpmsg, ctrl_q);
    rpmsg_lite_deinit(my_rpmsg);

    fail:
    while (1);
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
