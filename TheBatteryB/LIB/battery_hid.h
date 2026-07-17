#pragma once

#include <stdint.h>
#include "battery_acpi.h"
#include "usb_descriptors.h"

typedef struct __attribute__((packed))
{
    uint32_t CMD;
    uint32_t BATTERY_STATE;
    uint32_t BATTERY_PRESENT_RATE;
    uint32_t BATTEY_REMAINING_CAPACITY;
    uint32_t BATTERY_VOLTAGE;
    uint32_t LAST_FULL_CHARGE_CAPACITY;
    uint32_t DESIGN_CAPACITY;
    uint32_t CYCLE_COUNT;
} battery_hid_input_report_t;

typedef struct __attribute__((packed))
{
    uint32_t CMD;
    uint32_t TARGET_SOC;
} battery_hid_control_report_t;

static inline void battery_hid_build_input_report(battery_hid_input_report_t *out)
{
    if (!out) {
        return;
    }

    out->CMD = CMD;
    out->BATTERY_STATE = BATTERY_STATE;
    out->BATTERY_PRESENT_RATE = BATTERY_PRESENT_RATE;
    out->BATTEY_REMAINING_CAPACITY = BATTEY_REMAINING_CAPACITY;
    out->BATTERY_VOLTAGE = BATTERY_VOLTAGE;
    out->LAST_FULL_CHARGE_CAPACITY = LAST_FULL_CHARGE_CAPACITY;
    out->DESIGN_CAPACITY = DESIGN_CAPACITY;
    out->CYCLE_COUNT = CYCLE_COUNT;
}

static inline void battery_hid_apply_control_report(const battery_hid_control_report_t *in)
{
    if (!in) {
        return;
    }

    CMD = in->CMD;
    TARGET_SOC = in->TARGET_SOC;

    if (TARGET_SOC > 100u) {
        TARGET_SOC = 100u;
    }
}