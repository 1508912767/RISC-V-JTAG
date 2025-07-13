#ifndef __INTERFACE_FUNCTION_H
#define __INTERFACE_FUNCTION_H

#include "config.h"

static inline void delay_us(uint32_t nus)
{
    for (int i = 0; i < 40*nus; i++)
    {
        __NOP();
    }
}

static inline void delay_ms(uint32_t nms)
{
    HAL_Delay(nms);
}

//////////////////////////////////////////////////////////////////
// gpio interface
typedef int (*gpio_init_fn_t)(void*, uint32_t, uint32_t, uint32_t);
typedef int (*gpio_on_fn_t)(void*);
typedef int (*gpio_off_fn_t)(void*);
typedef int (*gpio_get_state_fn_t)(void*, uint8_t*);
typedef int (*gpio_toggle_fn_t)(void*);
typedef struct 
{
    gpio_init_fn_t init;
    gpio_on_fn_t on;
    gpio_off_fn_t off;
    gpio_get_state_fn_t get_state;
    gpio_toggle_fn_t toggle;
}gpio_i;

static inline int gpio_init(void *self, uint32_t mode, uint32_t pull, uint32_t speed)
{
    return (*(gpio_i **)self)->init(self, mode, pull, speed);
}
static inline int gpio_on(void *self)
{
    return (*(gpio_i **)self)->on(self);
}
static inline int gpio_off(void *self)
{
    return (*(gpio_i **)self)->off(self);
}
static inline int gpio_get_state(void *self, uint8_t *state)
{
    return (*(gpio_i **)self)->get_state(self, state);
}
static inline int gpio_toggle(void *self)
{
    return (*(gpio_i **)self)->toggle(self);
}

// mcu gpio interface
typedef struct 
{
    gpio_i *interface;
	GPIO_TypeDef* port;
    uint16_t pin;
}mcu_gpio_t;

static inline int mcu_gpio_init(mcu_gpio_t *self, uint32_t mode, uint32_t pull, uint32_t speed)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = self->pin;
	GPIO_InitStruct.Mode = mode;
	GPIO_InitStruct.Pull = pull;
	GPIO_InitStruct.Speed = speed;
	HAL_GPIO_Init(self->port, &GPIO_InitStruct); 
    return 0;
}
static inline int mcu_gpio_on(mcu_gpio_t *self)
{
    HAL_GPIO_WritePin(self->port, self->pin, GPIO_PIN_SET);
    return 0;
}
static inline int mcu_gpio_off(mcu_gpio_t *self)
{
    HAL_GPIO_WritePin(self->port, self->pin, GPIO_PIN_RESET);
    return 0;
}
static inline int mcu_gpio_get_state(mcu_gpio_t *self, uint8_t *state)
{
    *state = HAL_GPIO_ReadPin(self->port, self->pin);
    return 0;
}
static inline int mcu_gpio_toggle(mcu_gpio_t *self)
{
    HAL_GPIO_TogglePin(self->port, self->pin);
    return 0;
}



#endif