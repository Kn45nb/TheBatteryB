#pragma once

#include <stdint.h>
#include <stdbool.h>

/*==============================================================
  Debug / control globals live in TheBatteryB.c
==============================================================*/
extern uint32_t CMD;
extern uint32_t TARGET_SOC;

extern const uint32_t REVISION;
extern const uint32_t POWER_UNIT;
extern const uint32_t DESIGN_CAPACITY;
extern uint32_t LAST_FULL_CHARGE_CAPACITY;
extern const uint32_t BATTERY_TECHNOLOGY;
extern const uint32_t DESIGN_VOLTAGE;
extern uint32_t DESIGN_CAPACITY_OF_WARNING;
extern uint32_t DESIGN_CAPACITY_OF_LOW;
extern const uint32_t CAPACITY_GRANULARITY_1;
extern const uint32_t CAPACITY_GRANULARITY_2;
extern uint32_t CYCLE_COUNT;
extern uint32_t MEASUREMENT_ACCURACY;
extern uint32_t MAX_SAMPLING_TIME;
extern uint32_t MIN_SAMPLING_TIME;
extern uint32_t MAX_AVERAGING_INTERVAL;
extern uint32_t MIN_AVERAGING_INTERVAL;

extern const char MODEL_NUMBER[];
extern const char SERIAL_NUMBER[];
extern const char BATTEY_TYPE[];
extern const char OEM_INFORMATION[];

extern uint32_t BATTERY_STATE;
extern uint32_t BATTERY_PRESENT_RATE;
extern uint32_t BATTEY_REMAINING_CAPACITY;
extern uint32_t BATTERY_VOLTAGE;

/*==============================================================
  State flags
==============================================================*/
#define BAT_STATE_FLAG_CHARGING       (1u << 0)
#define BAT_STATE_FLAG_DISCHARGING    (1u << 1)
#define BAT_STATE_FLAG_AC_PRESENT      (1u << 2)
#define BAT_STATE_FLAG_SHUTDOWN        (1u << 3)

#define BATTERY_UPDATE_INTERVAL_MS     100u

/*==============================================================
  Helpers
==============================================================*/
static inline uint8_t battery_logic_soc(void)
{
    if (LAST_FULL_CHARGE_CAPACITY == 0u) {
        return 0u;
    }

    uint32_t soc = (BATTEY_REMAINING_CAPACITY * 100u) / LAST_FULL_CHARGE_CAPACITY;
    if (soc > 100u) {
        soc = 100u;
    }

    return (uint8_t)soc;
}

static inline void battery_logic_init_defaults(void)
{
    /* Keep the original debug values close to the user's sample */
    LAST_FULL_CHARGE_CAPACITY = DESIGN_CAPACITY;
    DESIGN_CAPACITY_OF_WARNING = (LAST_FULL_CHARGE_CAPACITY / 10u);
    DESIGN_CAPACITY_OF_LOW = (LAST_FULL_CHARGE_CAPACITY / 20u);

    BATTEY_REMAINING_CAPACITY = (LAST_FULL_CHARGE_CAPACITY * 30u) / 100u; /* 30% */
    BATTERY_VOLTAGE = DESIGN_VOLTAGE;
    BATTERY_PRESENT_RATE = 0;
    BATTERY_STATE = BAT_STATE_FLAG_DISCHARGING;
    CYCLE_COUNT = 0;

    CMD = 0;
    TARGET_SOC = 30;
}

static inline void battery_logic_tick(uint32_t dt_ms)
{
    if (LAST_FULL_CHARGE_CAPACITY == 0u) {
        return;
    }

    uint32_t step = (uint32_t)(((uint64_t)LAST_FULL_CHARGE_CAPACITY * dt_ms) / 100000u);
    if (step == 0u) {
        step = 1u;
    }

    const uint32_t full_limit = LAST_FULL_CHARGE_CAPACITY;
    const uint32_t low_limit =
        (DESIGN_CAPACITY_OF_LOW != 0u) ? DESIGN_CAPACITY_OF_LOW
                                       : (LAST_FULL_CHARGE_CAPACITY / 20u);

    bool charging = (BATTERY_STATE & BAT_STATE_FLAG_CHARGING) != 0u;
    bool discharging = !charging;

    switch (CMD) {
    case 1u: /* force charge */
        charging = true;
        discharging = false;
        break;

    case 2u: /* force discharge */
        charging = false;
        discharging = true;
        break;

    case 3u: /* set SOC */
    {
        if (TARGET_SOC > 100u) {
            TARGET_SOC = 100u;
        }
        BATTEY_REMAINING_CAPACITY = (uint32_t)(((uint64_t)LAST_FULL_CHARGE_CAPACITY * TARGET_SOC) / 100u);

        /* Simple debug rule: above mid-point -> charging, below mid-point -> discharging */
        charging = (TARGET_SOC >= 50u);
        discharging = !charging;
    }
        break;

    default: /* auto */
        if (charging) {
            if (BATTEY_REMAINING_CAPACITY + step >= full_limit) {
                BATTEY_REMAINING_CAPACITY = full_limit;
                charging = false;
                discharging = true;
            } else {
                BATTEY_REMAINING_CAPACITY += step;
            }
        } else {
            if (BATTEY_REMAINING_CAPACITY > step) {
                BATTEY_REMAINING_CAPACITY -= step;
            } else {
                BATTEY_REMAINING_CAPACITY = 0u;
            }

            if (BATTEY_REMAINING_CAPACITY <= low_limit) {
                charging = true;
                discharging = false;
                CYCLE_COUNT++;
            }
        }
        break;
    }

    const uint8_t soc = battery_logic_soc();

    BATTERY_STATE = 0;
    if (charging) {
        BATTERY_STATE |= BAT_STATE_FLAG_CHARGING;
        BATTERY_STATE |= BAT_STATE_FLAG_AC_PRESENT;
        BATTERY_PRESENT_RATE = -(int32_t)1200;
    } else {
        BATTERY_STATE |= BAT_STATE_FLAG_DISCHARGING;
        BATTERY_PRESENT_RATE = +(int32_t)1200;
    }

    if (BATTEY_REMAINING_CAPACITY <= low_limit || soc <= 5u) {
        BATTERY_STATE |= BAT_STATE_FLAG_SHUTDOWN;
    }

    /* Keep voltage as the design voltage for clean debug; can be changed later */
    BATTERY_VOLTAGE = DESIGN_VOLTAGE;
}