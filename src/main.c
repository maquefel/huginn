/*
 ============================================================================
 Name        : main.c
 Author      : Nikita Shubin
 Version     :
 Copyright   : Topcon
 Description :
============================================================================
 */

#include <string.h>
#include <assert.h>
#include "board.h"
#include "mu_imx.h"
#include "gpio_imx.h"
#include "gpio_pins.h"

/** debug */
#include "debug_console_imx.h"
#include "print_scan.h"

/** openamp */
// #include "openamp.h"
#include <openamp/open_amp.h>
#include "openamp_conf.h"
#include "rsc_table.h"

/** libmetal */
#include <metal/sys.h>
#include <metal/device.h>

#define MAX_BUFFER_SIZE RPMSG_BUFFER_SIZE
#include "virt_uart.h"

typedef enum
{
    RESET = 0,
    SET = !RESET
} FlagStatus, ITStatus;

VIRT_UART_HandleTypeDef huart0;
VIRT_UART_HandleTypeDef huart1;

__IO FlagStatus VirtUart0RxMsg = RESET;
uint8_t VirtUart0ChannelBuffRx[MAX_BUFFER_SIZE];
uint16_t VirtUart0ChannelRxSize = 0;

__IO FlagStatus VirtUart1RxMsg = RESET;
uint8_t VirtUart1ChannelBuffRx[MAX_BUFFER_SIZE];
uint16_t VirtUart1ChannelRxSize = 0;

#define debug_log(format, ...) metal_log(METAL_LOG_DEBUG, "%s:%s:%d: " format "\n", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)

/*
 * APP decided interrupt priority
 */
#define APP_MU_IRQ_PRIORITY     3
#define SHM_DEVICE_NAME         "IMX7DM4_SHM"
#define RPMSG_MU_CHANNEL        1

static struct metal_io_region *shm_io;
static struct metal_io_region *rsc_io;
static struct shared_resource_table *rsc_table;
static struct rpmsg_virtio_shm_pool shpool;
static struct rpmsg_virtio_device rvdev;

static metal_phys_addr_t shm_physmap;

struct metal_device shm_device = {
    .name = SHM_DEVICE_NAME,
    .num_regions = 2,
    .regions = {
        {.virt = NULL}, /* shared memory */
        {.virt = NULL}, /* rsc_table memory */
    },
    .node = { NULL },
    .irq_num = 0,
    .irq_info = NULL
};

static int openamp_shmem_init(int RPMsgRole)
{
    int status = 0;
    struct metal_device *device;
    void* rsc_tab_addr;
    int rsc_size;

    status = metal_register_generic_device(&shm_device);
    if (status != 0) {
        return status;
    }

    status = metal_device_open("generic", SHM_DEVICE_NAME, &device);
    if (status != 0) {
        return status;
    }

    shm_physmap = SHM_START_ADDRESS;
    metal_io_init(&device->regions[0], (void *)SHM_START_ADDRESS, &shm_physmap,
                  SHM_SIZE, -1, 0, NULL);

    debug_log("metal_io_init SHM_START_ADDRESS=0x%x, shm_physmap=0x%x", SHM_START_ADDRESS, shm_physmap);

    shm_io = metal_device_io_region(device, 0);
    if (shm_io == NULL) {
        return -1;
    }

    /* Initialize resources table variables */
    resource_table_init(RPMsgRole, &rsc_tab_addr, &rsc_size);
    rsc_table = (struct shared_resource_table *)rsc_tab_addr;
    if (!rsc_table)
    {
        return -1;
    }

    /** Checking resources table sanity*/
    debug_log("rsc_table->vdev->status=%u", rsc_table->vdev.status);
    debug_log("rsc_table->vring0: da=0x%x, align=%u, num=%u", rsc_table->vring0.da, rsc_table->vring0.align, rsc_table->vring0.num);
    debug_log("rsc_table->vring1: da=0x%x, align=%u, num=%u", rsc_table->vring1.da, rsc_table->vring1.align, rsc_table->vring1.num);

    metal_io_init(&device->regions[1], rsc_table,
                  (metal_phys_addr_t *)rsc_table, rsc_size, -1, 0, NULL);

    rsc_io = metal_device_io_region(device, 1);

    if (rsc_io == NULL) {
        return -1;
    }

    return 0;
}

#define RL_GET_Q_ID(id) ((id)&0x1U)

int MAILBOX_Notify(void *priv, uint32_t id)
{
    (void)priv;

    /* As Linux suggests, use MU->Data Channel 1 as communication channel */
    uint32_t msg = (RL_GET_Q_ID(id)) << 16;
    //env_lock_mutex(platform_lock);
    //RPMSG_MU_CHANNEL
    MU_SendMsg(MUB, RPMSG_MU_CHANNEL, msg);
    //env_unlock_mutex(platform_lock);
    PRINTF("\n\rMAILBOX_Notify id=%u, msg=0x%x\n\r", id, msg);

    return 0;
}

void MAILBOX_Init()
{
    /*
     * Prepare for the MU Interrupt
     *  MU (Messaging Unit) must be initialized before rpmsg init is called
     */
    MU_Init(BOARD_MU_BASE_ADDR);
    NVIC_SetPriority(BOARD_MU_IRQ_NUM, APP_MU_IRQ_PRIORITY);
    NVIC_EnableIRQ(BOARD_MU_IRQ_NUM);
}

int MAILBOX_Poll(struct virtio_device *vdev)
{
    (void)vdev;

    /* If we got an interrupt, ask for the corresponding virtqueue processing */
    uint32_t msg;
    mu_status_t status = MU_TryReceiveMsg(BOARD_MU_BASE_ADDR, RPMSG_MU_CHANNEL, &msg);

    if(status == kStatus_MU_Success) {
        debug_log("kStatus_MU_Success");
        rproc_virtio_notified(vdev, VRING1_ID);
        rproc_virtio_notified(NULL, VRING1_ID);
    }

//     if (msg_received_ch1 == RX_BUF_FREE) {
//         OPENAMP_log_dbg("Running virt0 (ch_1 buf free)\r\n");
//         rproc_virtio_notified(vdev, VRING0_ID);
//         msg_received_ch1 = RX_NO_MSG;
//         return 0;
//     }
//
//     if (msg_received_ch2 == RX_NEW_MSG) {
//         OPENAMP_log_dbg("Running virt1 (ch_2 new msg)\r\n");
//         rproc_virtio_notified(vdev, VRING1_ID);
//         msg_received_ch2 = RX_NO_MSG;
//
//         /* The OpenAMP framework does not notify for free buf: do it here */
//         rproc_virtio_notified(NULL, VRING1_ID);
//         return 0;
//     }

    return -1;
}

int openamp_init(int RPMsgRole, rpmsg_ns_bind_cb ns_bind_cb)
{
    struct fw_rsc_vdev_vring *vring_rsc;
    struct virtio_device *vdev;
    int status = 0;

    MAILBOX_Init();

    vdev = rproc_virtio_create_vdev(RPMsgRole, VDEV_ID, &rsc_table->vdev,
                                    rsc_io, NULL, MAILBOX_Notify, NULL);
    if (vdev == NULL)
    {
        return -1;
    }

    rproc_virtio_wait_remote_ready(vdev);
    vring_rsc = &rsc_table->vring0;
    status = rproc_virtio_init_vring(vdev, 0, vring_rsc->notifyid,
                                     (void *)vring_rsc->da, shm_io,
                                     vring_rsc->num, vring_rsc->align);
    if (status != 0)
    {
        return status;
    }
    vring_rsc = &rsc_table->vring1;
    status = rproc_virtio_init_vring(vdev, 1, vring_rsc->notifyid,
                                     (void *)vring_rsc->da, shm_io,
                                     vring_rsc->num, vring_rsc->align);
    if (status != 0)
    {
        return status;
    }

    rpmsg_virtio_init_shm_pool(&shpool, (void *)VRING_BUFF_ADDRESS,
                               (size_t)SHM_SIZE);
    status = rpmsg_init_vdev(&rvdev, vdev, ns_bind_cb, shm_io, &shpool);
    if(status != 0)
    {
        return status;
    }

    return 0;
}

void OPENAMP_DeInit()
{
    rpmsg_deinit_vdev(&rvdev);

    metal_finish();
}

void OPENAMP_init_ept(struct rpmsg_endpoint *ept)
{
    rpmsg_init_ept(ept, "", RPMSG_ADDR_ANY, RPMSG_ADDR_ANY, NULL, NULL);
}

int OPENAMP_create_endpoint(struct rpmsg_endpoint *ept, const char *name,
                            uint32_t dest, rpmsg_ept_cb cb,
                            rpmsg_ns_unbind_cb unbind_cb)
{
    return rpmsg_create_ept(ept, &rvdev.rdev, name, RPMSG_ADDR_ANY, dest, cb,
                            unbind_cb);
}

void VIRT_UART0_RxCpltCallback(VIRT_UART_HandleTypeDef *huart);
void VIRT_UART1_RxCpltCallback(VIRT_UART_HandleTypeDef *huart);

void OPENAMP_check_for_message(void)
{
    MAILBOX_Poll(rvdev.vdev);
}

void OPENAMP_Wait_EndPointready(struct rpmsg_endpoint *rp_ept)
{
    while(!is_rpmsg_ept_ready(rp_ept))
        MAILBOX_Poll(rvdev.vdev);
}

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

void default_log_handler(enum metal_log_level level, const char *format, ...)
{
    char msg[1024] = {0};
    va_list args;

    static const char *level_strs[] = {
        "metal: emerg:  ",
        "metal: alert:  ",
        "metal: crit:   ",
        "metal: error:  ",
        "metal: warn:   ",
        "metal: notice: ",
        "metal: info:   ",
        "metal: debug:  ",
    };

    char *p = msg;
    char **buffer = &p;

    va_start(args, format);
    _doprint((void*)buffer, _sputc, sizeof(msg), (char*)format, args);
    va_end(args);

    debug_printf("%s%s\r\n", level_strs[level], msg);
}

void dump_vring(struct vring *vr)
{
    metal_log(METAL_LOG_DEBUG, "vring num=%u desc_addr=0x%x, avail_addr=0x%x, used_addr=0x%x", vr->num, vr->desc, vr->avail, vr->used);
}



/*!
 * @brief Main function
 */
int main(void)
{
    struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
    struct rpmsg_endpoint ept;

    hardware_init();

    int ret = metal_init(&metal_params);

    if(ret != 0) {
        PRINTF("metal_init failed with %d.", errno);
        goto fail;
    }

    metal_set_log_handler(default_log_handler);
    metal_set_log_level(METAL_LOG_DEBUG);

    debug_printf("\n\r================= Hello ==================\n\r");

    ret = openamp_shmem_init(RPMSG_REMOTE);

    if(ret != 0) {
        debug_log("openamp_shmem_init failed with %d", ret);
        goto fail;
    }

    ret = openamp_init(RPMSG_REMOTE, NULL);

    if(ret != 0) {
        debug_log("openamp_init failed with %d", ret);
        goto fail;
    }

    debug_log("openamp_init succeeded", ret);

    OPENAMP_init_ept(&ept);

    debug_log("Virtual UART0 OpenAMP-rpmsg channel creation");
    if (VIRT_UART_Init(&huart0) != VIRT_UART_OK) {
        debug_log("VIRT_UART_Init UART0 failed.");
        goto fail;
    }

    debug_log("Virtual UART1 OpenAMP-rpmsg channel creation");
    if (VIRT_UART_Init(&huart1) != VIRT_UART_OK) {
        debug_log("VIRT_UART_Init UART1 failed.");
        goto fail;
    }

    /*Need to register callback for message reception by channels*/
    if(VIRT_UART_RegisterCallback(&huart0, VIRT_UART_RXCPLT_CB_ID, VIRT_UART0_RxCpltCallback) != VIRT_UART_OK)
    {
        debug_log("VIRT_UART_RegisterCallback UART0 failed.");
        goto fail;
    }

    if(VIRT_UART_RegisterCallback(&huart1, VIRT_UART_RXCPLT_CB_ID, VIRT_UART1_RxCpltCallback) != VIRT_UART_OK)
    {
        debug_log("VIRT_UART_RegisterCallback UART1 failed.");
        goto fail;
    }

    debug_log("receive virtqueue:");
    virtqueue_dump(rvdev.rvq);
    dump_vring(&rvdev.rvq->vq_ring);

    debug_log("send virtqueue:");
    virtqueue_dump(rvdev.svq);
    dump_vring(&rvdev.svq->vq_ring);

    debug_printf("================ Goodbye =================\n\r");

	while (1)
	{
        OPENAMP_check_for_message();

        if (VirtUart0RxMsg) {
            VirtUart0RxMsg = RESET;
            VIRT_UART_Transmit(&huart0, VirtUart0ChannelBuffRx, VirtUart0ChannelRxSize);
        }

        if (VirtUart1RxMsg) {
            VirtUart1RxMsg = RESET;
            VIRT_UART_Transmit(&huart1, VirtUart1ChannelBuffRx, VirtUart1ChannelRxSize);
        }
	}

	fail:
	while (1)
    {
        msleep(100);
    }
}

void VIRT_UART0_RxCpltCallback(VIRT_UART_HandleTypeDef *huart)
{

    debug_log("Msg received on VIRTUAL UART0 channel:  %s", (char *) huart->pRxBuffPtr);

    /* copy received msg in a variable to sent it back to master processor in main infinite loop*/
    VirtUart0ChannelRxSize = huart->RxXferSize < MAX_BUFFER_SIZE? huart->RxXferSize : MAX_BUFFER_SIZE-1;
    memcpy(VirtUart0ChannelBuffRx, huart->pRxBuffPtr, VirtUart0ChannelRxSize);
    VirtUart0RxMsg = SET;
}

void VIRT_UART1_RxCpltCallback(VIRT_UART_HandleTypeDef *huart)
{

    debug_log("Msg received on VIRTUAL UART1 channel:  %s", (char *) huart->pRxBuffPtr);

    /* copy received msg in a variable to sent it back to master processor in main infinite loop*/
    VirtUart1ChannelRxSize = huart->RxXferSize < MAX_BUFFER_SIZE? huart->RxXferSize : MAX_BUFFER_SIZE-1;
    memcpy(VirtUart1ChannelBuffRx, huart->pRxBuffPtr, VirtUart1ChannelRxSize);
    VirtUart1RxMsg = SET;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
