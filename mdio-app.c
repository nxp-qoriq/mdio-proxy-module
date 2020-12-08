// SPDX-License-Identifier: (GPL-2.0+ OR BSD-3-Clause)
/* Copyright 2020 NXP
 */
#include <linux/types.h>
#include <linux/netlink.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "mdio-common.h"

/* A very early proof-of-concept for accessing mdio busses from userspace */
int main(int argc, char *argv[])
{
	int fd;
	struct sockaddr_nl local;
	struct sockaddr_nl kernel;
	struct nlmsghdr *nlh;
	struct iovec iov;
	struct msghdr msg;
	struct mdio_message mdio_msg;
	int is_c45 = 0;
	int retries = 100;

	if ((argc < 5) || (argc > 7)) {
		fprintf(stderr, "Wrong number of parameters provided.\n");
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "\t%s <bus> <addr> c22 <reg> [data]\n", argv[0]);
		fprintf(stderr, "\t%s <bus> <addr> c45 <devad> <reg> [data]\n", argv[0]);
		return -1;
	}

	memset(&mdio_msg, 0, sizeof(mdio_msg));
	/* default operation is read */
	mdio_msg.op = MDIO_OP_READ;

	/* get bus name & desired address */
	strncpy(mdio_msg.bus_name, argv[1], sizeof(mdio_msg.bus_name) - 1);
	mdio_msg.addr = strtol(argv[2], NULL, 16);
	mdio_msg.op = MDIO_OP_READ;

	/* clause-45 access ? */
	if (!strncmp("c45", argv[3], 3)) {
		is_c45 = 1;
		mdio_msg.reg = (1<<30);
		mdio_msg.reg |= strtol(argv[4], NULL, 16) << 16;
	}

	/* from this point everything gets offseted by is_c45 */
	mdio_msg.reg |= strtol(argv[4 + is_c45], NULL, 16);

	if (argc == (6 + is_c45)) {
		mdio_msg.data = strtol(argv[5 + is_c45], NULL, 16);
		mdio_msg.op = MDIO_OP_WRITE;
	}

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_USERSOCK);

	/* setup our receive side first */	
	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_pid = getpid();
	local.nl_groups = 0;

	if (bind(fd, (struct sockaddr *) &local, sizeof(local)) < 0) {
		perror("bind");
		return -1;
	}

	/* send out message */
	memset(&kernel, 0, sizeof(kernel));
	kernel.nl_family = AF_NETLINK;
	kernel.nl_pid = 0; /* kernel expects 0 */
	kernel.nl_groups = 0;

	nlh = malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;
	memcpy(NLMSG_DATA(nlh), &mdio_msg, sizeof(mdio_msg));

	memset(&iov, 0, sizeof(iov));
	iov.iov_base = (void *) nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *) &kernel;
	msg.msg_namelen = sizeof(kernel);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	sendmsg(fd, &msg, 0);

	/* receive reply using the same iov and msg
	 * clear the nlh before recv
	 */
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	while (--retries && !recvmsg(fd, &msg, 0)) {
		usleep(10);
	}

	if (!retries) {
		printf("Operation timeout\n");
		goto finish;
	}

	memcpy(&mdio_msg, NLMSG_DATA(nlh), sizeof(mdio_msg));
	printf("Got back data: 0x%04x\n", mdio_msg.data);

finish:
	free(nlh);
	return 0;
}
