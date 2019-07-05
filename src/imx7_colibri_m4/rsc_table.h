/*
* Copyright (c) 2014, Mentor Graphics Corporation
* All rights reserved.
* Copyright (c) 2015 Xilinx, Inc. All rights reserved.
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/* This file populates resource table for BM remote
* for use by the Linux Master */

#ifndef RSC_TABLE_H_
#define RSC_TABLE_H_

#include <stddef.h>
#include <stdint.h>

#define FW_RSC_U64_ADDR_ANY 0xFFFFFFFFFFFFFFFFUL
#define FW_RSC_U32_ADDR_ANY 0xFFFFFFFFUL

enum fw_resource_type {
    RSC_CARVEOUT = 0,
    RSC_DEVMEM = 1,
    RSC_TRACE = 2,
    RSC_VDEV = 3,
    RSC_LAST = 4,
    RSC_VENDOR_START = 128,
    RSC_VENDOR_END = 512,
};

struct fw_rsc_vdev_vring {
    uint32_t da;
    uint32_t align;
    uint32_t num;
    uint32_t notifyid;
    uint32_t reserved;
}__attribute__((packed));

struct fw_rsc_vdev {
    uint32_t type;
    uint32_t id;
    uint32_t notifyid;
    uint32_t dfeatures;
    uint32_t gfeatures;
    uint32_t config_len;
    uint8_t status;
    uint8_t num_of_vrings;
    uint8_t reserved[2];
    struct fw_rsc_vdev_vring vring[0];
}__attribute__((packed));

#define NO_RESOURCE_ENTRIES         8

/* Resource table for the given remote */
struct remote_resource_table {
    uint32_t version;
    uint32_t num;
    uint32_t reserved[2];
    uint32_t offset[NO_RESOURCE_ENTRIES];
    /* rpmsg vdev entry */
    struct fw_rsc_vdev rpmsg_vdev;
}__attribute__((packed));

void *get_resource_table (int rsc_id, int *len);

#endif /* RSC_TABLE_H_ */
