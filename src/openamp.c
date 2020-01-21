/**
  ******************************************************************************
  * @file           : openamp.c
  * @brief          : Code for openamp applications
  ******************************************************************************
    * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "openamp.h"
#include "rsc_table.h"
#include "metal/sys.h"
#include "metal/device.h"
/* Private define ------------------------------------------------------------*/
#include "debug_console_imx.h"
#include "mu_imx.h"

#define SHM_DEVICE_NAME         "IMX7D_SHM"

/* Globals */

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

static int OPENAMP_shmem_init(int RPMsgRole)
{
  int status = 0;
  struct metal_device *device;
  struct metal_init_params metal_params = METAL_INIT_DEFAULTS;
  void* rsc_tab_addr;
  int rsc_size;

  metal_init(&metal_params);

  status = metal_register_generic_device(&shm_device);
  if (status != 0) {
	  PRINTF("metal_register_generic_device failed with %d\n\r", status);
    return status;
  }

  status = metal_device_open("generic", SHM_DEVICE_NAME, &device);
  if (status != 0) {
	  PRINTF("metal_device_open failed with %d\n\r", status);
    return status;
  }

  shm_physmap = SHM_START_ADDRESS;
  PRINTF("shm_physmap = 0x%x\n\r", SHM_START_ADDRESS);

  metal_io_init(&device->regions[0], (void *)SHM_START_ADDRESS, &shm_physmap,
                SHM_SIZE, -1, 0, NULL);

  shm_io = metal_device_io_region(device, 0);

  if (shm_io == NULL) {
    return -1;
  }

  /* Initialize resources table variables */
  resource_table_init(RPMsgRole, &rsc_tab_addr, &rsc_size);
  rsc_table = (struct shared_resource_table *)rsc_tab_addr;
  if (!rsc_table)
  {
    PRINTF("Failed to int resource_table_init");
    return -1;
  }

  metal_io_init(&device->regions[1], rsc_table,
               (metal_phys_addr_t *)rsc_table, rsc_size, -1, 0, NULL);

  // resource_table should be inited from linux remoteproc
  PRINTF("resource_table.vring0.VRING_TX_ADDRESS=0x%x, num=%d\n\r", rsc_table->vring0.da, rsc_table->vring0.num);
  PRINTF("resource_table.vring1.VRING_RX_ADDRESS=0x%x, num=%d\n\r", rsc_table->vring1.da, rsc_table->vring1.num);

  rsc_io = metal_device_io_region(device, 1);
  if (rsc_io == NULL) {
    return -1;
  }

  return 0;
}

#define RL_GET_Q_ID(id) ((id)&0x1U)
#define RPMSG_MU_CHANNEL (1)

// typedef int (*rpvdev_notify_func)(void *priv, uint32_t id);
int MAILBOX_Notify(void *priv, uint32_t id)
{
    (void)priv;

    /* As Linux suggests, use MU->Data Channel 1 as communication channel */
    uint32_t msg = (RL_GET_Q_ID(id)) << 16;
    PRINTF("\n\rMAILBOX_Notify id=%u, msg=0x%x\n\r", id, msg);
    //env_lock_mutex(platform_lock);
    MU_SendMsg(MUB, RPMSG_MU_CHANNEL, msg);
    //env_unlock_mutex(platform_lock);

    return 0;
}


/*! @brief MU status return codes. */
// typedef enum _mu_status
// {
//     kStatus_MU_Success      = 0U, /*!< Success.                              */
//     kStatus_MU_TxNotEmpty   = 1U, /*!< TX register is not empty.             */
//     kStatus_MU_RxNotFull    = 2U, /*!< RX register is not full.              */
//     kStatus_MU_FlagPending  = 3U, /*!< Previous flags update pending.        */
//     kStatus_MU_EventPending = 4U, /*!< MU event is pending.                  */
//     kStatus_MU_Initialized  = 5U, /*!< MU driver has initialized previously. */
//     kStatus_MU_IntPending   = 6U, /*!< Previous general interrupt still pending. */
//     kStatus_MU_Failed       = 7U  /*!< Execution failed.                     */
// } mu_status_t;

int MAILBOX_Poll(struct virtio_device *vdev)
{
    uint32_t msg;
    mu_status_t status = MU_TryReceiveMsg(MUB, RPMSG_MU_CHANNEL, &msg);

    if(status == kStatus_MU_Success) {
        // MAILBOX_Notify((void*)0, 0);
        // clear

        PRINTF("MU_TryReceiveMsg=kStatus_MU_Success=0x%x\n\r", msg);
        int ret = rproc_virtio_notified(vdev, VRING1_ID);
        if(ret != 0) {
            PRINTF("MAILBOX_Poll: rproc_virtio_notified failed with %d\n\r", ret);
            return -1;
        }

        rproc_virtio_notified(NULL, VRING1_ID);

        return 0;
    }

    if(!MU_IsRxFull(MUB, RPMSG_MU_CHANNEL)) {
        //PRINTF("MAILBOX_Poll: MU_IsTxEmpty\n\r");
        int ret = rproc_virtio_notified(vdev, VRING0_ID);
        return 0;
    }

    return -1;

// 	if (msg_received_ch1 == RX_BUF_FREE) {
// 		OPENAMP_log_dbg("Running virt0 (ch_1 buf free)\r\n");
// 		rproc_virtio_notified(vdev, VRING0_ID);
// 		msg_received_ch1 = RX_NO_MSG;
// 		return 0;
// 	}

// 	if (msg_received_ch2 == RX_NEW_MSG) {
// 		OPENAMP_log_dbg("Running virt1 (ch_2 new msg)\r\n");
// 		rproc_virtio_notified(vdev, VRING1_ID);
// 		msg_received_ch2 = RX_NO_MSG;
//
// 		/* The OpenAMP framework does not notify for free buf: do it here */
// 		rproc_virtio_notified(NULL, VRING1_ID);
// 		return 0;
// 	}
}

int MX_OPENAMP_Init(int RPMsgRole, rpmsg_ns_bind_cb ns_bind_cb)
{
  struct fw_rsc_vdev_vring *vring_rsc;
  struct virtio_device *vdev;
  int status = 0;

  /* Libmetal Initilalization */
  status = OPENAMP_shmem_init(RPMsgRole);
  if(status)
  {
    PRINTF("OPENAMP_shmem_init failed with %d\n\r", status);
    return status;
  }

  vdev = rproc_virtio_create_vdev(RPMsgRole,
                                  VDEV_ID,
                                  &rsc_table->vdev,
                                  rsc_io,
                                  NULL,
                                  MAILBOX_Notify,
                                  NULL);
  if (vdev == NULL)
  {
    PRINTF("rproc_virtio_create_vdev failed with %d\n\r", status);
    return -1;
  }

  rproc_virtio_wait_remote_ready(vdev);
  vring_rsc = &rsc_table->vring0;
  status = rproc_virtio_init_vring(vdev, 0, vring_rsc->notifyid,
                                   (void *)vring_rsc->da, shm_io,
                                   vring_rsc->num, vring_rsc->align);
  if (status != 0)
  {
    PRINTF("rproc_virtio_init_vring0 failed with %d\n\r", status);
    return status;
  }

  vring_rsc = &rsc_table->vring1;
  status = rproc_virtio_init_vring(vdev, 1, vring_rsc->notifyid,
                                   (void *)vring_rsc->da, shm_io,
                                   vring_rsc->num, vring_rsc->align);
  if (status != 0)
  {
    PRINTF("rproc_virtio_init_vring1 failed with %d\n\r", status);
    return status;
  }

  PRINTF("VRING_BUFF_ADDRESS=0x%x\n\r", VRING_BUFF_ADDRESS);
  PRINTF("shm_io->virt=0x%x, shpool.base=0x%x\n\r", shm_io->virt, shpool.base);

  rpmsg_virtio_init_shm_pool(&shpool,
                             (void *)VRING_BUFF_ADDRESS,
                             (size_t)SHM_SIZE);

  return rpmsg_init_vdev(&rvdev, vdev, ns_bind_cb, shm_io, &shpool);
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
    PRINTF("OPENAMP_create_endpoint: rvdev.ns_ept.addr=0x%x\n\r", rvdev.rdev.ns_ept.addr);
    PRINTF("OPENAMP_create_endpoint: rvdev.ns_ept.dest_addr=0x%x\n\r", rvdev.rdev.ns_ept.dest_addr);
    return rpmsg_create_ept(ept, &rvdev.rdev, name, RPMSG_ADDR_ANY, dest, cb,
                            unbind_cb);
}

void OPENAMP_check_for_message(void)
{
    //rproc_virtio_notified(rvdev.vdev, VRING1_ID);
    MAILBOX_Poll(rvdev.vdev);
}

void OPENAMP_Wait_EndPointready(struct rpmsg_endpoint *rp_ept)
{
  while(!is_rpmsg_ept_ready(rp_ept))
    MAILBOX_Poll(rvdev.vdev);

  PRINTF("OPENAMP_Wait_EndPointready: returned\n\r");
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
