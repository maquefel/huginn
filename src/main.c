/*
* ============================================================================
* Name        : Exercicio_05_M7.c
* Author      : Raul Munoz
* Version     :
* Copyright   : Toradex
* Description : Read BTN3 and send status through RPMsg to A7.
*            Receive BTN1 status through RPmsg and change LED3 status according to this.
* ============================================================================
*/

#include <string.h>
#include "assert.h"
#include "board.h"
#include "mu_imx.h"
#include "gpio_imx.h"
#include "gpio_pins.h"
#include "debug_console_imx.h"

#include "openamp.h"
#include <openamp/rpmsg.h>
#include <openamp/rpmsg_virtio.h>

/*
* APP decided interrupt priority
*/
#define APP_MU_IRQ_PRIORITY 3
#define MAX_STRING_SIZE 496         /* Maximum size to hold the data A7 gives */
#define RPMSG_SERV_NAME "rpmsg-client-sample"
#define MSG_LIMIT 100
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

/*
* MU Interrrupt RPMsg handler
*/
// void rpmsg_handler(void)
// {
//     uint32_t msg, channel;
//
//     if (MU_TryReceiveMsg(MUB, RPMSG_MU_CHANNEL, &msg) == kStatus_MU_Success)
//     {
//         channel = msg >> 16;
//         env_isr(channel);
//     }
//
//     return;
// }

// struct isr_info *info;
// RL_ASSERT(vector < ISR_COUNT);
// if (vector < ISR_COUNT)
// {
//     info = &isr_table[vector];
//     virtqueue_notification((struct virtqueue *)info->data);
// }

void MU_M4_Handler(void)
{

    //MU_DisableRxFullInt(MUB, 1);
    PRINTF("MU_M4_Handler\n\r");
    MU_ClearGeneralIntPending(MUB, 0);
    //MU_EnableRxFullInt(MUB, 1);
    return;
}

static int rpmsg_endpoint_cb(struct rpmsg_endpoint *ept, void *data, size_t len,
                            uint32_t src, void *priv)
{
    (void)priv;
    (void)src;
    static uint32_t count = 0;
    char payload[RPMSG_BUFFER_SIZE];

    /* Send data back MSG_LIMIT time to master */
    memset(payload, 0, RPMSG_BUFFER_SIZE);
    memcpy(payload, data, len);
    if (++count <= MSG_LIMIT) {
        PRINTF("echo message number %u: %s\r\n",
            (unsigned int)count, payload);
        if (rpmsg_send(ept, (char *)data, len) < 0) {
            PRINTF("rpmsg_send failed\r\n");
            goto destroy_ept;
        }

        if (count == MSG_LIMIT) {
            goto destroy_ept;
        }
    }
    return RPMSG_SUCCESS;

    destroy_ept:
    //shutdown_req = 1;
    return RPMSG_SUCCESS;
}

static void rpmsg_service_unbind(struct rpmsg_endpoint *ept)
{
    (void)ept;
    PRINTF("unexpected Remote endpoint destroy\r\n");
    //shutdown_req = 1;
}

// typedef void (*rpmsg_ns_bind_cb)(struct rpmsg_device *rdev, const char *name, uint32_t dest);

void ns_bind_example(struct rpmsg_device *rdev, const char *name, uint32_t dest)
{
    PRINTF("rpmsg_ns_bind_cb: ns_bind_example: name=%s, dest=0x%x\r\n", name, dest);
    return;
}

int MX_OPENAMP_Init(int RPMsgRole, rpmsg_ns_bind_cb ns_bind_cb);

/*!
* @brief Main function
*/
int main(void)
{
    static struct rpmsg_endpoint ept;

    hardware_init();

    /*
    * Prepare for the MU Interrupt
    *  MU (Messaging Unit) must be initialized before rpmsg init is called
    */
    MU_Init(BOARD_MU_BASE_ADDR);
    NVIC_SetPriority(BOARD_MU_IRQ_NUM, APP_MU_IRQ_PRIORITY);
    NVIC_EnableIRQ(BOARD_MU_IRQ_NUM);

    //MU_EnableGeneralInt(MUB, 0);
    //MU_EnableRxFullInt(MUB, 0);

    __asm volatile("cpsie i"); //platform_global_isr_enable();

    PRINTF("\n\r================= Hello ==================\n\r");
    PRINTF("NVIC_GetEnableIRQ(%d)=%d\n\r", BOARD_MU_IRQ_NUM, NVIC_GetEnableIRQ(BOARD_MU_IRQ_NUM));

    int status = MX_OPENAMP_Init(RPMSG_REMOTE, ns_bind_example);

    if(status != 0) {
        PRINTF("MX_OPENAMP_Init failed with %d\n\r", status);
    }

    /** kick ass */
    //MAILBOX_Notify((void*)0, 0);


    OPENAMP_init_ept(&ept);

    PRINTF("OPENAMP_create_endpoint...\n\r");
    status = OPENAMP_create_endpoint(&ept,
                                    "huginn_hang_with_odin",
                                    0,
                                    rpmsg_endpoint_cb,
                                    rpmsg_service_unbind);

    if(status != 0) {
        PRINTF("OPENAMP_create_endpoint failed with %d\n\r", status);
    }

    OPENAMP_Wait_EndPointready(&ept);

    PRINTF("OPENAMP_create_endpoint success!\n\r");
    PRINTF("ep: addr=0x%x\n\r", ept.addr);
    PRINTF("ep: dest_addr=0x%x\n\r", ept.dest_addr);

    status = rpmsg_send(&ept, "TEST", 4);

    if (status)
        PRINTF("rpmsg_send failed: %d\n", status);

    while (1)
    {
        OPENAMP_check_for_message();
        msleep(100);
    }

    fail:
    /* We should never get here as control is now taken by the scheduler */
    while (1)
    {
        msleep(100);
    }
}
/*******************************************************************************
* EOF
******************************************************************************/
