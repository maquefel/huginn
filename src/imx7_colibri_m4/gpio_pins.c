/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of Freescale Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>
#include "gpio_pins.h"

gpio_config_t gpioLed = {
    "EXT_IO0 LED",                      /* name */
    &IOMUXC_LPSR_SW_MUX_CTL_PAD_GPIO1_IO02,  /* muxReg */
    0,                                  /* muxConfig */
    &IOMUXC_LPSR_SW_PAD_CTL_PAD_GPIO1_IO02,  /* padReg */
    0,                                  /* padConfig */
    GPIO1,                              /* base */
    2                                   /* pin */
};

gpio_config_t gpioKeyFunc1 = {
    "EXT_IO1",                                      /* name */
    &IOMUXC_SW_MUX_CTL_PAD_EPDC_GDRL,               /* muxReg */
    5,                                              /* muxConfig */
    &IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL,               /* padReg */
    IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL_PS(2) |        /* padConfig */
        IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL_PE_MASK |
	IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL_HYS_MASK,
    GPIO2,                                          /* base */
    26                                              /* pin */
};

gpio_config_t gpioKeyFunc2 = {
    "EXT_IO2",                                       /* name */
    &IOMUXC_SW_MUX_CTL_PAD_EPDC_SDCE2,               /* muxReg */
    5,                                              /* muxConfig */
    &IOMUXC_SW_PAD_CTL_PAD_EPDC_SDCE2,                  /* padReg */
    IOMUXC_SW_PAD_CTL_PAD_EPDC_SDCE2_PS(2) |        /* padConfig */
        IOMUXC_SW_PAD_CTL_PAD_EPDC_SDCE2_PE_MASK |
	IOMUXC_SW_PAD_CTL_PAD_EPDC_SDCE2_HYS_MASK,
    GPIO2,                                          /* base */
    22                                              /* pin */
};


gpio_config_t gpioLed1 = {
    "Led 1",                                         /* name */
    &IOMUXC_SW_MUX_CTL_PAD_ECSPI1_SCLK,              /* muxReg */
    5,                                               /* muxConfig */
    &IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK,              /* padReg */
    IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK_PS(3)   |      /* padConfig */
    IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK_PE_MASK |
    IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SCLK_HYS_MASK,
    GPIO4,                                           /* base */
    16                                                /* pin */
};

gpio_init_config_t Led1 = {
    .pin           = 16, //pin number
    .direction     = gpioDigitalOutput,
    .interruptMode = gpioIntRisingEdge
};

gpio_config_t gpioLed2 = {
    "Led 1",                                         /* name */
    &IOMUXC_SW_MUX_CTL_PAD_ECSPI1_MOSI,              /* muxReg */
    5,                                               /* muxConfig */
    &IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI,              /* padReg */
    IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI_PS(3)   |      /* padConfig */
    IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI_PE_MASK |
    IOMUXC_SW_PAD_CTL_PAD_ECSPI1_MOSI_HYS_MASK,
    GPIO4,                                           /* base */
    17                                                /* pin */
};
gpio_init_config_t Led2 = {
    .pin           = 17, //pin number
    .direction     = gpioDigitalOutput,
    .interruptMode = gpioIntRisingEdge
};

gpio_config_t gpioLed3 = {
    "Led 1",                                         /* name */
    &IOMUXC_SW_MUX_CTL_PAD_EPDC_GDRL,              /* muxReg */
    5,                                               /* muxConfig */
    &IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL,              /* padReg */
    IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL_PS(3)   |      /* padConfig */
    IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL_PE_MASK |
    IOMUXC_SW_PAD_CTL_PAD_EPDC_GDRL_HYS_MASK,
    GPIO2,                                           /* base */
    26                                                /* pin */
};

gpio_init_config_t Led3 = {
    .pin           = 26, //pin number
    .direction     = gpioDigitalOutput,
    .interruptMode = gpioIntRisingEdge
};


gpio_config_t gpioBtn1 = {
    "Led 1",                                         /* name */
    &IOMUXC_SW_MUX_CTL_PAD_ECSPI2_MISO,              /* muxReg */
    5,                                               /* muxConfig */
    &IOMUXC_SW_PAD_CTL_PAD_ECSPI2_MISO,              /* padReg */
    IOMUXC_SW_PAD_CTL_PAD_ECSPI2_MISO_PS(3)   |      /* padConfig */
    IOMUXC_SW_PAD_CTL_PAD_ECSPI2_MISO_PE_MASK |
    IOMUXC_SW_PAD_CTL_PAD_ECSPI2_MISO_HYS_MASK,
    GPIO4,                                           /* base */
    22                                                /* pin */
};
gpio_init_config_t Btn1 = {
    .pin           = 22, //pin number
    .direction     = gpioDigitalInput,
    .interruptMode = gpioNoIntmode
};

gpio_config_t gpioBtn2 = {
    "Led 1",                                         /* name */
    &IOMUXC_SW_MUX_CTL_PAD_SD2_RESET_B,              /* muxReg */
    5,                                               /* muxConfig */
    &IOMUXC_SW_PAD_CTL_PAD_SD2_RESET_B,              /* padReg */
    IOMUXC_SW_PAD_CTL_PAD_SD2_RESET_B_PS(3)   |      /* padConfig */
    IOMUXC_SW_PAD_CTL_PAD_SD2_RESET_B_PE_MASK |
    IOMUXC_SW_PAD_CTL_PAD_SD2_RESET_B_HYS_MASK,
    GPIO5,                                           /* base */
    11                                                /* pin */
};
gpio_init_config_t Btn2 = {
    .pin           = 11, //pin number
    .direction     = gpioDigitalInput,
    .interruptMode = gpioNoIntmode
};

gpio_config_t gpioBtn3 = {
    "Led 1",                                         /* name */
    &IOMUXC_SW_MUX_CTL_PAD_ECSPI1_SS0,              /* muxReg */
    5,                                               /* muxConfig */
    &IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SS0,              /* padReg */
    IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SS0_PS(3)   |      /* padConfig */
    IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SS0_PE_MASK |
    IOMUXC_SW_PAD_CTL_PAD_ECSPI1_SS0_HYS_MASK,
    GPIO4,                                           /* base */
    19                                                /* pin */
};
gpio_init_config_t Btn3 = {
    .pin           = 19, //pin number
    .direction     = gpioDigitalInput,
    .interruptMode = gpioNoIntmode
};

void configure_gpio_pin(gpio_config_t *config)
{
    assert(config);

    *(config->muxReg) = config->muxConfig;
    *(config->padReg) = config->padConfig;
}

/*******************************************************************************
 * EOF
 ******************************************************************************/
