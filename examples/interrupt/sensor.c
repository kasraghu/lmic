/*
 * Copyright (c) 2014-2016 IBM Corporation.
 * All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the <organization> nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "lmic.h"
#include "hw.h"

// use PB12 (DIP switch 1)
#define INP_PORT 1
#define INP_PIN 12

static osjob_t irqjob;

// use DIP1 as sensor value
void initsensor (osjobcb_t callback) {
    // configure input
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN; // clock enable port B
    hw_cfg_pin(GPIOx(INP_PORT), INP_PIN, GPIOCFG_MODE_INP | GPIOCFG_OSPEED_40MHz | GPIOCFG_OTYPE_OPEN);
    hw_cfg_extirq(INP_PORT, INP_PIN, GPIO_IRQ_CHANGE);
    // save application callback
    irqjob.func = callback;
}

// read PB12
uint16_t readsensor () {
    return ((GPIOB->IDR & (1 << INP_PIN)) != 0);
}

// called by EXTI_IRQHandler
// (set preprocessor option CFG_EXTI_IRQ_HANDLER=sensorirq)
#ifndef CFG_EXTI_IRQ_HANDLER
#error "CFG_EXTI_IRQ_HANDLER must be defined in makefile!"
#endif
void sensorirq () {
    if((EXTI->PR & (1<<INP_PIN)) != 0) { // pending
	EXTI->PR = (1<<INP_PIN); // clear irq
        // run application callback function in 50ms (debounce)
        os_setTimedCallback(&irqjob, os_getTime()+ms2osticks(50), irqjob.func);
    }
}
