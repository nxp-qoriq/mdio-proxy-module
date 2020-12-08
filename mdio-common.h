// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/* Copyright 2020 NXP
 */
#ifndef __MDIO_COMMON
#define __MDIO_COMMON

#define NETLINK_USERSOCK 2
#define MAX_PAYLOAD 1024


enum mdio_operation {
	MDIO_OP_INVALID = 0,
	MDIO_OP_READ,
	MDIO_OP_WRITE,
};

enum mdio_result {
	MDIO_RESULT_INVALID = 0,
	MDIO_RESULT_FAILURE,
	MDIO_RESULT_SUCCESS,
};

struct mdio_message {
	char bus_name[100];
	enum mdio_operation op;
	enum mdio_result result;
	uint16_t addr;
	uint32_t reg;
	uint16_t data;
} __PACKED__;

#endif
