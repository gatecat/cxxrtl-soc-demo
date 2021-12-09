// This file is Copyright (c) 2013-2014 Sebastien Bourdeauducq <sb@m-labs.hk>
// This file is Copyright (c) 2014-2019 Florent Kermarrec <florent@enjoy-digital.fr>
// This file is Copyright (c) 2018 Ewen McNeill <ewen@naos.co.nz>
// This file is Copyright (c) 2018 Felix Held <felix-github@felixheld.de>
// This file is Copyright (c) 2019 Gabriel L. Somlo <gsomlo@gmail.com>
// This file is Copyright (c) 2017 Tim 'mithro' Ansell <mithro@mithis.com>
// This file is Copyright (c) 2018 William D. Jones <thor0505@comcast.net>
// License: BSD

#include "sfl.h"
#include <boot/common.h>
#include <boot/serial.h>
#include <config.h>
#include <crc.h>
#include <stdio.h>
#include <timer.h>
#include <uart.h>

enum {
	ACK_TIMEOUT,
	ACK_CANCELLED,
	ACK_OK
};

static int check_ack(void)
{
	int recognized;
	static const char str[SFL_MAGIC_LEN] = SFL_MAGIC_ACK;
        uint32_t period = CONFIG_CLOCK_FREQ / 4;

	timer_enable(0);
	timer_reload(0);
	timer_set_count(period);
	timer_enable(1);
	recognized = 0;
	while (timer_get_count() != 0) {
		if (uart_read_nonblock()) {
			char c;
			c = uart_read();
			if ((c == 'Q') || (c == '\e'))
				return ACK_CANCELLED;
			if (c == str[recognized]) {
				recognized++;
				if (recognized == SFL_MAGIC_LEN)
					return ACK_OK;
			} else {
				if (c == str[0])
					recognized = 1;
				else
					recognized = 0;
			}
		}
	}
	return ACK_TIMEOUT;
}

static uint32_t get_uint32(unsigned char* data)
{
	return ((uint32_t) data[0] << 24) |
		   ((uint32_t) data[1] << 16) |
		   ((uint32_t) data[2] << 8) |
		    (uint32_t) data[3];
}

#define MAX_FAILED 5

/* Returns 1 if other boot methods should be tried */
int serialboot(void)
{
	struct sfl_frame frame;
	int failed;
	static const char str[SFL_MAGIC_LEN+1] = SFL_MAGIC_REQ;
	const char *c;
	int ack_status;

	printf("Booting from serial...\n");
	printf("Press Q or ESC to abort boot completely.\n");

	c = str;
	while(*c) {
		uart_write(*c);
		c++;
	}
	ack_status = check_ack();
	if(ack_status == ACK_TIMEOUT) {
		printf("Timeout\n");
		return 1;
	}
	if(ack_status == ACK_CANCELLED) {
		printf("Cancelled\n");
		return 0;
	}
	/* assume ACK_OK */

	failed = 0;
	while(1) {
		int i;
		int actualcrc;
		int goodcrc;

		/* Get one Frame */
		frame.length = uart_read();
		frame.crc[0] = uart_read();
		frame.crc[1] = uart_read();
		frame.cmd = uart_read();
		for(i=0;i<frame.length;i++)
			frame.payload[i] = uart_read();

		/* Check Frame CRC (if CMD has a CRC) */
		if (frame.cmd != SFL_CMD_LOAD_NO_CRC) {
			actualcrc = ((int)frame.crc[0] << 8)|(int)frame.crc[1];
			goodcrc = crc16(&frame.cmd, frame.length+1);
			if(actualcrc != goodcrc) {
				failed++;
				if(failed == MAX_FAILED) {
					printf("Too many consecutive errors, aborting");
					return 1;
				}
				uart_write(SFL_ACK_CRCERROR);
				continue;
			}
		}

		/* Execute Frame CMD */
		switch(frame.cmd) {
			case SFL_CMD_ABORT:
				failed = 0;
				uart_write(SFL_ACK_SUCCESS);
				return 1;
			case SFL_CMD_LOAD:
			case SFL_CMD_LOAD_NO_CRC: {
				char *writepointer;

				failed = 0;
				writepointer = (char *) get_uint32(&frame.payload[0]);
				for(i=4;i<frame.length;i++)
					*(writepointer++) = frame.payload[i];
				if (frame.cmd == SFL_CMD_LOAD)
					uart_write(SFL_ACK_SUCCESS);
				break;
			}
			case SFL_CMD_JUMP: {
				uint32_t addr;

				failed = 0;
				addr = get_uint32(&frame.payload[0]);
				uart_write(SFL_ACK_SUCCESS);
				boot(0, 0, 0, addr);
				break;
			}
			case SFL_CMD_FLASH: {
				break;
			}
			case SFL_CMD_REBOOT:
				break;
			default:
				failed++;
				if(failed == MAX_FAILED) {
					printf("Too many consecutive errors, aborting");
					return 1;
				}
				uart_write(SFL_ACK_UNKNOWN);
				break;
		}
	}
	return 1;
}
