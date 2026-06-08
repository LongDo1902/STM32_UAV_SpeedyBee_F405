/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: jflyper
 *
 * Follows the extended dshot telemetry documentation found at
 * https://github.com/bird-sanctuary/extended-dshot-telemetry
 */

#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

// platform
#define USE_DSHOT
#define FAST_CODE

// fix order
#include "nvic.h"
// fix order
#include "common_atomic.h"
// fix order

#include "dshot.h"

#ifdef USE_DSHOT
#define ERPM_PER_LSB 100.0f

#define DSHOT_FRAME_BITS 16
#define DSHOT_DMA_LEN    18
#define MOTOR_COUNT      4

// (ARR = 139 case, DShot600 @ 84MHz timer)
#define DSHOT_T0         52
#define DSHOT_T1         105

extern TIM_HandleTypeDef htim8;

static uint16_t motorValue[MOTOR_COUNT];
static uint16_t dshot_dma[MOTOR_COUNT][DSHOT_DMA_LEN];
static uint8_t  motor_output_map[MOTOR_COUNT] = {0, 1, 2, 3};

FAST_CODE uint16_t
prepareDshotPacket(dshotProtocolControl_t *pcb)
{
    uint16_t packet;

    ATOMIC_BLOCK(NVIC_PRIO_DSHOT_DMA)
    {
        packet                = (pcb->value << 1) | (pcb->requestTelemetry ? 1 : 0);
        pcb->requestTelemetry = false; // reset telemetry request to make sure it's
                                       // triggered only once in a row
    }

    // compute checksum
    unsigned csum      = 0;
    unsigned csum_data = packet;
    for (int i = 0; i < 3; i++) {
        csum ^= csum_data; // xor data by nibbles
        csum_data >>= 4;
    }
    // append checksum
    csum &= 0xf;
    packet = (packet << 4) | csum;

    return packet;
}

#endif // USE_DSHOT

// temporarily here, needs to be moved during refactoring
void
validateAndfixMotorOutputReordering(uint8_t *array, const unsigned size)
{
    bool invalid = false;

    for (unsigned i = 0; i < size; i++) {
        if (array[i] >= size) {
            invalid = true;
            break;
        }
    }

    int valuesAsIndexes[size];

    for (unsigned i = 0; i < size; i++) {
        valuesAsIndexes[i] = -1;
    }

    if (!invalid) {
        for (unsigned i = 0; i < size; i++) {
            if (-1 != valuesAsIndexes[array[i]]) {
                invalid = true;
                break;
            }

            valuesAsIndexes[array[i]] = array[i];
        }
    }

    if (invalid) {
        for (unsigned i = 0; i < size; i++) {
            array[i] = i;
        }
    }
}

static void
dshot_encode_motor(uint8_t motor, uint16_t value)
{
    dshotProtocolControl_t pcb;
    pcb.value            = value;
    pcb.requestTelemetry = 0;

    uint16_t packet = prepareDshotPacket(&pcb);

    for (int i = 0; i < 16; i++) {
        if (packet & (1 << (15 - i)))
            dshot_dma[motor][i] = DSHOT_T1;
        else
            dshot_dma[motor][i] = DSHOT_T0;
    }

    dshot_dma[motor][16] = 0;
    dshot_dma[motor][17] = 0;
}

void
DShot_ApplyReorder(uint8_t *order)
{
    validateAndfixMotorOutputReordering(order, MOTOR_COUNT);
    // internal save
    memcpy(motor_output_map, order, MOTOR_COUNT);
}

void
DShot_Update(void)
{
    for (int i = 0; i < MOTOR_COUNT; i++) {
        dshot_encode_motor(i, motorValue[i]);
    }
}

void
DShot_Trigger(void)
{
    HAL_TIM_PWM_Start_DMA(&htim8, TIM_CHANNEL_1, (uint32_t *)dshot_dma[0], DSHOT_DMA_LEN);
    HAL_TIM_PWM_Start_DMA(&htim8, TIM_CHANNEL_2, (uint32_t *)dshot_dma[1], DSHOT_DMA_LEN);
    HAL_TIM_PWM_Start_DMA(&htim8, TIM_CHANNEL_3, (uint32_t *)dshot_dma[2], DSHOT_DMA_LEN);
    HAL_TIM_PWM_Start_DMA(&htim8, TIM_CHANNEL_4, (uint32_t *)dshot_dma[3], DSHOT_DMA_LEN);
}
