/*
 * money.h
 *
 *  Created on: Nov 29, 2025
 *      Author: dobao
 */

#ifndef INC_MONEY_H_
#define INC_MONEY_H_

#include "headers.h"

#define SIZE	10

const uint32_t moneyList[SIZE] = {
		3700000,
		750000
};

uint32_t sumMoney(const uint32_t* inputMoneyList){
	if(inputMoneyList == NULL) return 0u;

	uint32_t sum = 0;
	for(uint32_t i = 0; i < SIZE; i++){
		sum+= inputMoneyList[i];
	}

	return sum;
}


#endif /* INC_MONEY_H_ */
