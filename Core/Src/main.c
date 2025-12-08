#include <main.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * =====================================================
 * 					 PRIVATE INCLUDES
 * =====================================================
 */
#include "stm32f4xx_hal.h"
#include "BLong_cortex.h"
#include "BLong.h"


int main(void){
	BLong_init(); //This is a clone version of HAL_Init()

	while(1);
}
