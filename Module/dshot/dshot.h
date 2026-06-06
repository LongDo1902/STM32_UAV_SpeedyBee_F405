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
 */

#pragma once

#include "pch.h"

typedef enum dshotTelemetryType_e
{
    DSHOT_TELEMETRY_TYPE_eRPM         = 0,
    DSHOT_TELEMETRY_TYPE_TEMPERATURE  = 1,
    DSHOT_TELEMETRY_TYPE_VOLTAGE      = 2,
    DSHOT_TELEMETRY_TYPE_CURRENT      = 3,
    DSHOT_TELEMETRY_TYPE_DEBUG1       = 4,
    DSHOT_TELEMETRY_TYPE_DEBUG2       = 5,
    DSHOT_TELEMETRY_TYPE_DEBUG3       = 6,
    DSHOT_TELEMETRY_TYPE_STATE_EVENTS = 7,
    DSHOT_TELEMETRY_TYPE_COUNT
} dshotTelemetryType_t;

typedef enum dshotRawValueState_e
{
    DSHOT_RAW_VALUE_STATE_INVALID       = 0,
    DSHOT_RAW_VALUE_STATE_NOT_PROCESSED = 1,
    DSHOT_RAW_VALUE_STATE_PROCESSED     = 2,
    DSHOT_RAW_VALUE_STATE_COUNT
} dshotRawValueState_t;

typedef struct dshotProtocolControl_s
{
    uint16_t value;
    bool     requestTelemetry;
} dshotProtocolControl_t;

// dev
#define MOTOR_COUNT 4
#define DSHOT_FRAME_BITS 16
#define DSHOT_DMA_LEN    18

void DShot_ApplyReorder(uint8_t *order);
void DShot_Update(void);
void DShot_Trigger(void);
// ---

uint16_t    prepareDshotPacket(dshotProtocolControl_t *pcb);
extern bool useDshotTelemetry;
void        validateAndfixMotorOutputReordering(uint8_t *array, const unsigned size);
