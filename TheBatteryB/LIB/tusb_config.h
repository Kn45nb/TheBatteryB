#pragma once

#include "tusb_option.h"

#define CFG_TUSB_MCU               OPT_MCU_RP2040
#define CFG_TUSB_OS                OPT_OS_NONE

#define CFG_TUSB_RHPORT0_MODE      OPT_MODE_DEVICE
#define BOARD_TUD_RHPORT           0

#define CFG_TUD_ENABLED            1
#define CFG_TUH_ENABLED            0

#define CFG_TUD_CDC                0
#define CFG_TUD_MSC                0
#define CFG_TUD_HID                1
#define CFG_TUD_VENDOR             0

#define CFG_TUD_ENDPOINT0_SIZE     64
#define CFG_TUD_HID_EP_BUFSIZE     64

#define CFG_TUSB_DEBUG             0