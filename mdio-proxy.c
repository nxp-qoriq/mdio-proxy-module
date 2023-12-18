// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/* Copyright 2020 NXP
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>

#include <linux/of_device.h>
#include <linux/of_mdio.h>
#include <linux/of_gpio.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/reset.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>
#include <linux/io.h>
#include <linux/uaccess.h>


#include "mdio-common.h"

struct sock *socket;
struct mdio_message mdio_msg;

static void netlink_recvmsg(struct sk_buff *skb)
{
	struct mii_bus *mdio_bus;
	struct nlmsghdr *nlh = (struct nlmsghdr *) skb->data;
	pid_t pid = nlh->nlmsg_pid;
	struct mdio_message mdio_msg;
	size_t message_size = sizeof(struct mdio_message);
	struct sk_buff *skb_out;
	bool is_c45;
	int tmp;

	memcpy(&mdio_msg,  nlmsg_data(nlh), sizeof(mdio_msg));
	is_c45 = !!(mdio_msg.reg & (1 << 30));

	/*	
	printk(KERN_INFO "Received: bus=[%s], addr=%#x, op=%s, reg=%#x, data=%#x\n",
		mdio_msg.bus_name,
		mdio_msg.addr,
		mdio_msg.op == MDIO_OP_READ ? "READ":"WRITE",
		mdio_msg.reg,
		mdio_msg.data);
	*/

	mdio_bus = mdio_find_bus((const char *)mdio_msg.bus_name);
	if (!mdio_bus) {
		mdio_msg.result = MDIO_RESULT_FAILURE;
		goto bailout;
	}

	if (mdio_msg.op == MDIO_OP_READ) {
		if (is_c45) {
			tmp = mdiobus_c45_read(mdio_bus, mdio_msg.addr,
					       mdio_msg.reg >> 16, mdio_msg.reg & 0xFFFF);
			if (tmp < 0) {
				mdio_msg.result = MDIO_RESULT_FAILURE;
				goto bailout;
			}
		} else {
			tmp = mdiobus_read(mdio_bus, mdio_msg.addr, mdio_msg.reg);
			if (tmp < 0) {
				mdio_msg.result = MDIO_RESULT_FAILURE;
				goto bailout;
			}
		}

		mdio_msg.result = MDIO_RESULT_SUCCESS;
		mdio_msg.data = (uint16_t) tmp;
	} else if (mdio_msg.op == MDIO_OP_WRITE) {
		if (is_c45) {
			tmp = mdiobus_c45_write(mdio_bus, mdio_msg.addr, mdio_msg.reg >> 16,
						mdio_msg.reg & 0xFFFF, mdio_msg.data);
		} else {
			tmp = mdiobus_write(mdio_bus, mdio_msg.addr, mdio_msg.reg, mdio_msg.data);
		}
		/* speed optimization - do not send ack after write
		if (tmp < 0) {
			mdio_msg.result = MDIO_RESULT_FAILURE;
			goto bailout;
		}
		*/
		
		return;
	}

bailout:
	skb_out = nlmsg_new(message_size, GFP_KERNEL);
	if (!skb_out) {
		printk(KERN_ERR "Failed to allocate a new skb\n");
		return;
 	}

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, message_size, 0);
	NETLINK_CB(skb_out).dst_group = 0;
	memcpy(nlmsg_data(nlh), &mdio_msg, message_size);

	//printk(KERN_INFO "data=%#x\n", mdio_msg.data);
	nlmsg_unicast(socket, skb_out, pid);
}

static int __init mdio_proxy_init(void)
{
	struct netlink_kernel_cfg nl_cfg = {
		.input = netlink_recvmsg,
	};

	socket = netlink_kernel_create(&init_net, NETLINK_USERSOCK, &nl_cfg);
	if (socket == NULL) {
		return -1;
 	}

	printk(KERN_INFO "NXP MDIO proxy module loaded.\n");
	return 0;
}

static void __exit mdio_proxy_exit(void) {
	if (socket) {
		netlink_kernel_release(socket);
	}
}

module_init(mdio_proxy_init);
module_exit(mdio_proxy_exit);

MODULE_LICENSE("Dual BSD/GPL");
