/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stdint.h>
#include <zephyr/kernel.h>
	
int main(void)
{
volatile uint64_t k=0;

	while (1) {

		k++;
	
	//   k_sleep_ticks(K_MSEC(100));


	     k_sleep(K_MSEC(100));

	}

	return 0;
}
