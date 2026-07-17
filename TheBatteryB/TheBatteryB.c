/*============================================================================================================================================================================
Information
==============================================================================================================================================================================
SDK: 		v2.3.0
Toolchain:	15.2Rel1
Ninja:		v1.13.2
CMake:		v4.4.0
============================================================================================================================================================================*/



/*============================================================================================================================================================================
Include	Lib
============================================================================================================================================================================*/
// Base
#include    <stdio.h>
#include    <stdint.h>
#include    <stdbool.h>
#include    <string.h>

// SDK Pico
#include    "pico/stdlib.h"
#include    "bsp/board_api.h"
#include    "tusb.h"
#include    "pico/cyw43_arch.h"

// Project libs
#include    "battery_logic.h"
#include    "battery_acpi.h"
#include    "battery_hid.h"
#include    "usb_descriptors.h"

// SPI
#include	"hardware/spi.h"

// I2C
#include	"hardware/i2c.h"

// UART
#include	"hardware/uart.h"

// Other Lib
// #include	"SSD1306.h"



/*============================================================================================================================================================================
Defines     Var                             Val             Mô tả
============================================================================================================================================================================*/
#define UPDATE_INTERVAL_MS                  BATTERY_UPDATE_INTERVAL_MS
#define HID_REPORT_INTERVAL_MS              250u


// SPI defines
#define     SPI_PORT                        spi0
#define     PIN_MISO                        16
#define     PIN_CS                          17
#define     PIN_SCK                         18
#define     PIN_MOSI                        19


// I2C_0 defines
#define     I2C_PORT_0                      i2c0
#define     I2C_SDA_0                       8
#define     I2C_SCL_0                       9


// I2C_1 defines
#define     I2C_PORT_1                      i2c1
#define     I2C_SDA_1                       12
#define     I2C_SCL_1                       13



// // UART defines (default UART 'uart0')
// #define     UART_ID                         uart1
// #define     BAUD_RATE                       115200
// // PIN
// #define     UART_TX_PIN                     4
// #define     UART_RX_PIN                     5




/*============================================================================================================================================================================
Const   Type        Var                           Val                     Mô tả                                               Đơn vị          Note
============================================================================================================================================================================*/
// Commant of Master
        uint32_t    CMD                         = 0x0;                  // Command of Master                                 // N/a          //
        uint32_t    TARGET_SOC                  = 30;                   // Target SOC of Master                              // %            // !set: 0-100, @Kn45nb Cần một bộ logic i++, khởi tạo là 30 trừ dần mỗi lần time full.  

// Package: _BIX
const   uint32_t    REVISION                    = 0x0;                  // Version of the data structure _BIX               // N/a          // Basic: 0x0 (?const)
const   uint32_t    POWER_UNIT                  = 0x0;                  // Biến giá trị đơn vị                              // N/a          // 0: mWh, 1: mAh
const   uint32_t    DESIGN_CAPACITY             = 0x186A0;              // Dung lượng thiết kế của pin                      // POWER_UNIT   // !set: 0x0 || 0xFFFFFFFF
        uint32_t    LAST_FULL_CHARGE_CAPACITY   = 0x186A0;              // Dung lượng sạc đầy cuối cùng của pin             // POWER_UNIT   // !set: 0x0 || 0xFFFFFFFF, max: DESIGN_CAPACITY, @Kn45nb Cần một bộ logic i++, khởi tạo là DESIGN_CAPACITY trừ dần mỗi lần time full.
const   uint32_t    BATTERY_TECHNOLOGY          = 0x1;                  // Công nghệ tái tạo pin (khả năng sạc)             // N/a          // 0: No, 1: Yes
const   uint32_t    DESIGN_VOLTAGE              = 0x4A38;               // Điện áp thiết kế của pin                         // mV           // !set: 0x0 || 0xFFFFFFFF, Basic voltage: 19V/12V
        uint32_t    DESIGN_CAPACITY_OF_WARNING  = 0x2710;               // Dung lượng cảnh báo thiết kế của pin             // POWER_UNIT   // Windows ignores this value (Nhưng Bios thì rất hay đọc)
        uint32_t    DESIGN_CAPACITY_OF_LOW      = 0x1388;               // Dung lượng thấp thiết kế của pin (hibernation)   // POWER_UNIT   // 0-5% of LAST_FULL_CHARGE_CAPACITY, @Kn45nb Cần fun hoặc logic cập nhật sau khi update LAST_FULL_CHARGE_CAPACITY
const   uint32_t    CAPACITY_GRANULARITY_1      = 0x1;                  // Độ tinh mịch (phân giải) dung lượng trên ngưỡng  // POWER_UNIT   // <=1% of DESIGN_CAPACITY
const   uint32_t    CAPACITY_GRANULARITY_2      = 0x1;                  // Độ tinh mịch (phân giải) dung lượng dưới ngưỡng  // POWER_UNIT   // <=75mW, (exp: 0.25% of 25000mWh)
        uint32_t    CYCLE_COUNT                 = 0x0;                  // Số lần sạc của pin                               // Times        // !set: 0xFFFFFFFF
/*cont*/uint32_t    MEASUREMENT_ACCURACY        = 0x1;                  // Độ chính xác đo lường                            // PPM          // >95000 (95%), 1%=1000
/*cont*/uint32_t    MAX_SAMPLING_TIME           = 0x1;                  // Thời gian lấy mẫu tối đa của _BST                // ms           //
/*cont*/uint32_t    MIN_SAMPLING_TIME           = 0x1;                  // Thời gian lấy mẫu tối thiểu của _BST             // ms           //
/*cont*/uint32_t    MAX_AVERAGING_INTERVAL      = 0x1;                  // Thời gian trung bình max của cảm biến PIN        // ms           //
/*cont*/uint32_t    MIN_AVERAGING_INTERVAL      = 0x1;                  // Thời gian trung bình min của cảm biến PIN        // ms           //
const   char        MODEL_NUMBER[]              = "Notebook";           // Mã số model của pin                              // N/a          // !NULL
const   char        SERIAL_NUMBER[]             = "0001";               // Số serial duy nhất của pin                       // N/a          // !NULL
const   char        BATTEY_TYPE[]               = "LION";               // Loại pin                                         // N/a          //
const   char        OEM_INFORMATION[]           = "MAKE BY @Kn45nb";    // Thông tin OEM                                    // N/a          //


// Package: _BST
        uint32_t    BATTERY_STATE               = 0x1;                  // Trạng thái pin                                   // N/a          // 0x0000: Charging, 0x0001: Discharging, ...
        uint32_t    BATTERY_PRESENT_RATE        = 0x1;                  // Tốc độ Sạc/xả (Điện áp giữa 2 đầu nguồn điện)    // POWER_UNIT   // !set: 0x0 || 0xFFFFFFFF, (Âm sạc, dương xả) @Kn45nb $Do that
        uint32_t    BATTEY_REMAINING_CAPACITY   = 0x1;                  // Dung lượng còn lại của pin                       // POWER_UNIT   // !set: 0x0 || 0xFFFFFFFF
        uint32_t    BATTERY_VOLTAGE             = 0x4A38;               // Điện áp hiện tại của pin                         // mV           // 

        

/*============================================================================================================================================================================
Local data
============================================================================================================================================================================*/
static  battery_bix_t                   g_bix;
static  battery_bst_t                   g_bst;
static  battery_hid_input_report_t      g_input_report;
static  battery_hid_control_report_t    g_control_report;

static  uint32_t                        g_last_logic_ms     = 0;
static  uint32_t                        g_last_hid_ms       = 0;
static  uint32_t                        g_last_led_ms       = 0;
static  bool                            g_led_state         = false;
static  uint32_t                        g_blink_interval_ms = 250;



/*============================================================================================================================================================================
Methor
============================================================================================================================================================================*/
static void battery_refresh_debug_snapshots(void)
{
    battery_build_bix(&g_bix);
    battery_build_bst(&g_bst);
    battery_hid_build_input_report(&g_input_report);
}


void blink(uint16_t TIME_BLINK)
{
    tight_loop_contents();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(TIME_BLINK);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    sleep_ms(TIME_BLINK);
}


static void blink_error_forever(uint8_t code)
{
    while (true) {
        for (uint8_t i = 0; i < code; i++) {
            blink(120);
        }
        sleep_ms(800);
    }
}


static void battery_send_report(void)
{
    if (!tud_mounted() || !tud_hid_ready()) {
        return;
    }

    (void)tud_hid_report(THEBATTERYB_REPORT_ID_BATTERY,
                         &g_input_report,
                         sizeof(g_input_report));
}



static void led_task(void)
{
    uint32_t now = board_millis();
    if ((now - g_last_led_ms) < g_blink_interval_ms) {
        return;
    }

    g_last_led_ms = now;
    g_led_state = !g_led_state;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, g_led_state ? 1 : 0);
}




/*============================================================================================================================================================================
USB Descriptors
============================================================================================================================================================================*/
#define USB_PID   (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4))

#define _PID_MAP(itf, n)  ((CFG_TUD_##itf) << (n))

tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const* tud_descriptor_device_cb(void)
{
    return (uint8_t const*) &desc_device;
}

enum
{
    ITF_NUM_HID = 0,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN   (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)
#define EPNUM_HID          0x81

uint8_t const desc_hid_report[] =
{
    /* ============================================================
     * Report ID 1
     * Battery / Power summary input report
     *
     * Layout kept at 32 bytes to match your current battery_hid.h:
     *   1) CMD                     -> vendor defined / debug field
     *   2) BATTERY_STATE           -> PresentStatus bitfield
     *   3) BATTERY_PRESENT_RATE    -> Current
     *   4) BATTEY_REMAINING_CAPACITY
     *   5) BATTERY_VOLTAGE
     *   6) LAST_FULL_CHARGE_CAPACITY
     *   7) DESIGN_CAPACITY
     *   8) CYCLE_COUNT
     * ============================================================ */

    0x05, 0x84,                    // Usage Page (Power Device)
    0x09, 0x04,                    // Usage (UPS)
    0xA1, 0x01,                    // Collection (Application)

    0x09, 0x24,                    // Usage (PowerSummary)
    0xA1, 0x00,                    // Collection (Physical)

    0x05, 0x85,                    // Usage Page (Battery System)
    0x85, THEBATTERYB_REPORT_ID_BATTERY,

    /* Field 1: CMD (debug / reserved) */
    0x06, 0x00, 0xFF,              // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,                    // Usage (1)
    0x75, 0x20,                    // Report Size (32)
    0x95, 0x01,                    // Report Count (1)
    0x15, 0x00,                    // Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,  // Logical Maximum (2147483647)
    0x81, 0x02,                    // Input (Data,Var,Abs)

    /* Field 2: BATTERY_STATE as PresentStatus bitfield */
    0x05, 0x84,                    // Usage Page (Power Device)
    0x09, 0x02,                    // Usage (PresentStatus)
    0xA1, 0x02,                    // Collection (Logical)
    0x05, 0x85,                    // Usage Page (Battery System)
    0x09, 0xD0,                    // Usage (ACPresent)
    0x09, 0x42,                    // Usage (BelowRemainingCapacityLimit)
    0x09, 0x44,                    // Usage (Charging)
    0x09, 0x45,                    // Usage (Discharging)
    0x0B, 0x69, 0x00, 0x84, 0x00,  // Usage (ShutdownImminent)
    0x0B, 0x65, 0x00, 0x84, 0x00,  // Usage (Overload)
    0x0B, 0x00, 0x00, 0x84, 0x00,  // Usage (Undefined)
    0x0B, 0x00, 0x00, 0x84, 0x00,  // Usage (Undefined)
    0x75, 0x01,                    // Report Size (1)
    0x95, 0x08,                    // Report Count (8)
    0x25, 0x01,                    // Logical Maximum (1)
    0x81, 0x02,                    // Input (Data,Var,Abs)
    0xC0,                          // End Collection

    /* Field 3: BATTERY_PRESENT_RATE (Current) */
    0x05, 0x84,                    // Usage Page (Power Device)
    0x09, 0x31,                    // Usage (Current)
    0x75, 0x20,                    // Report Size (32)
    0x95, 0x01,                    // Report Count (1)
    0x15, 0x00,                    // Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,  // Logical Maximum (2147483647)
    0x81, 0x02,                    // Input (Data,Var,Abs)

    /* Field 4: BATTEY_REMAINING_CAPACITY */
    0x05, 0x85,                    // Usage Page (Battery System)
    0x09, 0x66,                    // Usage (RemainingCapacity)
    0x75, 0x20,                    // Report Size (32)
    0x95, 0x01,                    // Report Count (1)
    0x15, 0x00,
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,
    0x81, 0x02,

    /* Field 5: BATTERY_VOLTAGE */
    0x05, 0x84,                    // Usage Page (Power Device)
    0x09, 0x30,                    // Usage (Voltage)
    0x75, 0x20,                    // Report Size (32)
    0x95, 0x01,
    0x15, 0x00,
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,
    0x81, 0x02,

    /* Field 6: LAST_FULL_CHARGE_CAPACITY */
    0x05, 0x85,                    // Usage Page (Battery System)
    0x09, 0x67,                    // Usage (FullChargeCapacity)
    0x75, 0x20,                    // Report Size (32)
    0x95, 0x01,
    0x15, 0x00,
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,
    0x81, 0x02,

    /* Field 7: DESIGN_CAPACITY */
    0x09, 0x83,                    // Usage (DesignCapacity)
    0x75, 0x20,                    // Report Size (32)
    0x95, 0x01,
    0x15, 0x00,
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,
    0x81, 0x02,

    /* Field 8: CYCLE_COUNT */
    0x09, 0x6B,                    // Usage (CycleCount)
    0x75, 0x20,                    // Report Size (32)
    0x95, 0x01,
    0x15, 0x00,
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,
    0x81, 0x02,

    0xC0,                          // End Collection (Physical)
    0xC0,                          // End Collection (Application)

    /* ============================================================
     * Report ID 2
     * Control report (kept vendor-defined so your existing callback
     * and CMD/TARGET_SOC logic still works)
     * ============================================================ */

    0x06, 0x00, 0xFF,              // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x02,                    // Usage (2)
    0xA1, 0x01,                    // Collection (Application)

    0x85, THEBATTERYB_REPORT_ID_CONTROL,
    0x09, 0x01,
    0x75, 0x20,                    // 32-bit CMD
    0x95, 0x02,                    // CMD + TARGET_SOC
    0x15, 0x00,
    0x27, 0xFF, 0xFF, 0xFF, 0x7F,
    0xB1, 0x02,                    // Feature (Data,Var,Abs)

    0xC0
};

uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void)instance;
    return desc_hid_report;
}

uint8_t const desc_configuration[] =
{
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),

    TUD_HID_DESCRIPTOR(ITF_NUM_HID, 0, HID_ITF_PROTOCOL_NONE,
                       sizeof(desc_hid_report), EPNUM_HID,
                       CFG_TUD_HID_EP_BUFSIZE, 10)
};

uint8_t const* tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return desc_configuration;
}

enum
{
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
};

char const* string_desc_arr[] =
{
    (const char[]) { 0x09, 0x04 }, // English
    "Kn45nb",
    "TheBatteryB",
    NULL
};

static uint16_t _desc_str[32 + 1];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;
    size_t chr_count;

    switch (index)
    {
    case STRID_LANGID:
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
        break;

    case STRID_SERIAL:
        chr_count = board_usb_get_serial(_desc_str + 1, 32);
        break;

    default:
        if (!(index < (sizeof(string_desc_arr) / sizeof(string_desc_arr[0])))) {
            return NULL;
        }

        {
            const char *str = string_desc_arr[index];
            chr_count = strlen(str);
            size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1;
            if (chr_count > max_count) {
                chr_count = max_count;
            }

            for (size_t i = 0; i < chr_count; i++) {
                _desc_str[1 + i] = str[i];
            }
        }
        break;
    }

    _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return _desc_str;
}

/*============================================================================================================================================================================
USB HID callbacks
============================================================================================================================================================================*/
uint16_t tud_hid_get_report_cb(uint8_t instance,
                               uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t* buffer,
                               uint16_t reqlen)
{
    (void)instance;
    (void)reqlen;

    if (report_type == HID_REPORT_TYPE_INPUT && report_id == THEBATTERYB_REPORT_ID_BATTERY) {
        memcpy(buffer, &g_input_report, sizeof(g_input_report));
        return (uint16_t)sizeof(g_input_report);
    }

    if (report_type == HID_REPORT_TYPE_FEATURE && report_id == THEBATTERYB_REPORT_ID_CONTROL) {
        memcpy(buffer, &g_control_report, sizeof(g_control_report));
        return (uint16_t)sizeof(g_control_report);
    }

    return 0;
}

void tud_hid_set_report_cb(uint8_t instance,
                           uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const* buffer,
                           uint16_t bufsize)
{
    (void)instance;

    if (report_type != HID_REPORT_TYPE_FEATURE) {
        return;
    }

    if (report_id != THEBATTERYB_REPORT_ID_CONTROL) {
        return;
    }

    if (bufsize < sizeof(g_control_report)) {
        return;
    }

    memcpy(&g_control_report, buffer, sizeof(g_control_report));
    battery_hid_apply_control_report(&g_control_report);
}

/*============================================================================================================================================================================
USB state callbacks
============================================================================================================================================================================*/
void tud_mount_cb(void)
{
    g_blink_interval_ms = 1000;
}

void tud_umount_cb(void)
{
    g_blink_interval_ms = 250;
}

void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;
    g_blink_interval_ms = 2500;
}

void tud_resume_cb(void)
{
    g_blink_interval_ms = tud_mounted() ? 1000 : 250;
}



/*============================================================================================================================================================================
Main Function
============================================================================================================================================================================*/
int main(void)
{
     board_init();

    if (cyw43_arch_init()) {
        blink_error_forever(3);
        return -1;
    }

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    battery_logic_init_defaults();
    battery_refresh_debug_snapshots();

    const tusb_rhport_init_t rh_init =
    {
        .role = TUSB_ROLE_DEVICE,
        .speed = TUD_OPT_HIGH_SPEED ? TUSB_SPEED_HIGH : TUSB_SPEED_FULL
    };

    TU_ASSERT(tud_rhport_init(BOARD_TUD_RHPORT, &rh_init));
    board_init_after_tusb();

    g_last_logic_ms = board_millis();
    g_last_hid_ms = board_millis();
    g_last_led_ms = board_millis();

    while (1)
    {
        tud_task();

        uint32_t now = board_millis();

        if ((now - g_last_logic_ms) >= UPDATE_INTERVAL_MS) {
            g_last_logic_ms = now;
            battery_logic_tick(UPDATE_INTERVAL_MS);
            battery_refresh_debug_snapshots();
        }

        if ((now - g_last_hid_ms) >= HID_REPORT_INTERVAL_MS) {
            g_last_hid_ms = now;
            battery_send_report();
        }

        led_task();
    }




    // stdio_init_all();

    // if (cyw43_arch_init())
    // {
    //     // printf("Wi-Fi init failed\n");
    //     return -1;
    // }

    // i2c_init(I2C_PORT_0, 100*1000);
    // gpio_set_function(I2C_SDA_0, GPIO_FUNC_I2C);
    // gpio_set_function(I2C_SCL_0, GPIO_FUNC_I2C);
    // gpio_pull_up(I2C_SDA_0);
    // gpio_pull_up(I2C_SCL_0);



    // https://github.com/raspberrypi/pico-examples/tree/master/i2c

        // // SPI initialisation
    // spi_init(SPI_PORT, 1000*1000);                  // 1MHz
    // gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    // gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    // gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    // gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    // // Chip select is active-low, so we'll initialise it to a driven-high state
    // gpio_set_dir(PIN_CS, GPIO_OUT);
    // gpio_put(PIN_CS, 1);
    // https://github.com/raspberrypi/pico-examples/tree/master/spi

    // // Set up our UART
    // uart_init(UART_ID, BAUD_RATE);
    // // Set the TX and RX pins by using the function select on the GPIO
    // // Set datasheet for more information on function select
    // gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    // gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    // Send out a string, with CR/LF conversions
    // uart_puts(UART_ID, " Hello, UART!\n");
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart


    // while (1)
    // {
    //     blink(1000);
    // }
}
