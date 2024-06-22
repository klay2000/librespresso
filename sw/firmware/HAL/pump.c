#include "HAL/pump.h"

#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "HAL/pins.h"
#include "HAL/serial.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "semphr.h"
#include "task.h"

#define MAX_PKT_LEN 520

// CRC Table from the VESC firmware
const unsigned short crc16_tab[] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108,
    0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
    0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b,
    0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee,
    0xf5cf, 0xc5ac, 0xd58d, 0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
    0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d,
    0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5,
    0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
    0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a, 0x6ca6, 0x7c87, 0x4ce4,
    0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
    0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13,
    0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
    0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e,
    0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1,
    0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
    0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0,
    0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657,
    0x7676, 0x4615, 0x5634, 0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
    0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882,
    0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e,
    0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
    0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1, 0xef1f, 0xff3e, 0xcf5d,
    0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
    0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

// packet ids from vesc fw
typedef enum {
  VESC_COMM_FW_VERSION                         = 0,
  VESC_COMM_JUMP_TO_BOOTLOADER                 = 1,
  VESC_COMM_ERASE_NEW_APP                      = 2,
  VESC_COMM_WRITE_NEW_APP_DATA                 = 3,
  VESC_COMM_GET_VALUES                         = 4,
  VESC_COMM_SET_DUTY                           = 5,
  VESC_COMM_SET_CURRENT                        = 6,
  VESC_COMM_SET_CURRENT_BRAKE                  = 7,
  VESC_COMM_SET_RPM                            = 8,
  VESC_COMM_SET_POS                            = 9,
  VESC_COMM_SET_HANDBRAKE                      = 10,
  VESC_COMM_SET_DETECT                         = 11,
  VESC_COMM_SET_SERVO_POS                      = 12,
  VESC_COMM_SET_MCCONF                         = 13,
  VESC_COMM_GET_MCCONF                         = 14,
  VESC_COMM_GET_MCCONF_DEFAULT                 = 15,
  VESC_COMM_SET_APPCONF                        = 16,
  VESC_COMM_GET_APPCONF                        = 17,
  VESC_COMM_GET_APPCONF_DEFAULT                = 18,
  VESC_COMM_SAMPLE_PRINT                       = 19,
  VESC_COMM_TERMINAL_CMD                       = 20,
  VESC_COMM_PRINT                              = 21,
  VESC_COMM_ROTOR_POSITION                     = 22,
  VESC_COMM_EXPERIMENT_SAMPLE                  = 23,
  VESC_COMM_DETECT_MOTOR_PARAM                 = 24,
  VESC_COMM_DETECT_MOTOR_R_L                   = 25,
  VESC_COMM_DETECT_MOTOR_FLUX_LINKAGE          = 26,
  VESC_COMM_DETECT_ENCODER                     = 27,
  VESC_COMM_DETECT_HALL_FOC                    = 28,
  VESC_COMM_REBOOT                             = 29,
  VESC_COMM_ALIVE                              = 30,
  VESC_COMM_GET_DECODED_PPM                    = 31,
  VESC_COMM_GET_DECODED_ADC                    = 32,
  VESC_COMM_GET_DECODED_CHUK                   = 33,
  VESC_COMM_FORWARD_CAN                        = 34,
  VESC_COMM_SET_CHUCK_DATA                     = 35,
  VESC_COMM_CUSTOM_APP_DATA                    = 36,
  VESC_COMM_NRF_START_PAIRING                  = 37,
  VESC_COMM_GPD_SET_FSW                        = 38,
  VESC_COMM_GPD_BUFFER_NOTIFY                  = 39,
  VESC_COMM_GPD_BUFFER_SIZE_LEFT               = 40,
  VESC_COMM_GPD_FILL_BUFFER                    = 41,
  VESC_COMM_GPD_OUTPUT_SAMPLE                  = 42,
  VESC_COMM_GPD_SET_MODE                       = 43,
  VESC_COMM_GPD_FILL_BUFFER_INT8               = 44,
  VESC_COMM_GPD_FILL_BUFFER_INT16              = 45,
  VESC_COMM_GPD_SET_BUFFER_INT_SCALE           = 46,
  VESC_COMM_GET_VALUES_SETUP                   = 47,
  VESC_COMM_SET_MCCONF_TEMP                    = 48,
  VESC_COMM_SET_MCCONF_TEMP_SETUP              = 49,
  VESC_COMM_GET_VALUES_SELECTIVE               = 50,
  VESC_COMM_GET_VALUES_SETUP_SELECTIVE         = 51,
  VESC_COMM_EXT_NRF_PRESENT                    = 52,
  VESC_COMM_EXT_NRF_ESB_SET_CH_ADDR            = 53,
  VESC_COMM_EXT_NRF_ESB_SEND_DATA              = 54,
  VESC_COMM_EXT_NRF_ESB_RX_DATA                = 55,
  VESC_COMM_EXT_NRF_SET_ENABLED                = 56,
  VESC_COMM_DETECT_MOTOR_FLUX_LINKAGE_OPENLOOP = 57,
  VESC_COMM_DETECT_APPLY_ALL_FOC               = 58,
  VESC_COMM_JUMP_TO_BOOTLOADER_ALL_CAN         = 59,
  VESC_COMM_ERASE_NEW_APP_ALL_CAN              = 60,
  VESC_COMM_WRITE_NEW_APP_DATA_ALL_CAN         = 61,
  VESC_COMM_PING_CAN                           = 62,
  VESC_COMM_APP_DISABLE_OUTPUT                 = 63,
  VESC_COMM_TERMINAL_CMD_SYNC                  = 64,
  VESC_COMM_GET_IMU_DATA                       = 65,
  VESC_COMM_BM_CONNECT                         = 66,
  VESC_COMM_BM_ERASE_FLASH_ALL                 = 67,
  VESC_COMM_BM_WRITE_FLASH                     = 68,
  VESC_COMM_BM_REBOOT                          = 69,
  VESC_COMM_BM_DISCONNECT                      = 70,
  VESC_COMM_BM_MAP_PINS_DEFAULT                = 71,
  VESC_COMM_BM_MAP_PINS_NRF5X                  = 72,
  VESC_COMM_ERASE_BOOTLOADER                   = 73,
  VESC_COMM_ERASE_BOOTLOADER_ALL_CAN           = 74,
  VESC_COMM_PLOT_INIT                          = 75,
  VESC_COMM_PLOT_DATA                          = 76,
  VESC_COMM_PLOT_ADD_GRAPH                     = 77,
  VESC_COMM_PLOT_SET_GRAPH                     = 78,
  VESC_COMM_GET_DECODED_BALANCE                = 79,
  VESC_COMM_BM_MEM_READ                        = 80,
  VESC_COMM_WRITE_NEW_APP_DATA_LZO             = 81,
  VESC_COMM_WRITE_NEW_APP_DATA_ALL_CAN_LZO     = 82,
  VESC_COMM_BM_WRITE_FLASH_LZO                 = 83,
  VESC_COMM_SET_CURRENT_REL                    = 84,
  VESC_COMM_CAN_FWD_FRAME                      = 85,
  VESC_COMM_SET_BATTERY_CUT                    = 86,
  VESC_COMM_SET_BLE_NAME                       = 87,
  VESC_COMM_SET_BLE_PIN                        = 88,
  VESC_COMM_SET_CAN_MODE                       = 89,
  VESC_COMM_GET_IMU_CALIBRATION                = 90,
  VESC_COMM_GET_MCCONF_TEMP                    = 91,

  // Custom configuration for hardware
  VESC_COMM_GET_CUSTOM_CONFIG_XML     = 92,
  VESC_COMM_GET_CUSTOM_CONFIG         = 93,
  VESC_COMM_GET_CUSTOM_CONFIG_DEFAULT = 94,
  VESC_COMM_SET_CUSTOM_CONFIG         = 95,

  // BMS commands
  VESC_COMM_BMS_GET_VALUES           = 96,
  VESC_COMM_BMS_SET_CHARGE_ALLOWED   = 97,
  VESC_COMM_BMS_SET_BALANCE_OVERRIDE = 98,
  VESC_COMM_BMS_RESET_COUNTERS       = 99,
  VESC_COMM_BMS_FORCE_BALANCE        = 100,
  VESC_COMM_BMS_ZERO_CURRENT_OFFSET  = 101,

  // FW updates commands for different HW types
  VESC_COMM_JUMP_TO_BOOTLOADER_HW         = 102,
  VESC_COMM_ERASE_NEW_APP_HW              = 103,
  VESC_COMM_WRITE_NEW_APP_DATA_HW         = 104,
  VESC_COMM_ERASE_BOOTLOADER_HW           = 105,
  VESC_COMM_JUMP_TO_BOOTLOADER_ALL_CAN_HW = 106,
  VESC_COMM_ERASE_NEW_APP_ALL_CAN_HW      = 107,
  VESC_COMM_WRITE_NEW_APP_DATA_ALL_CAN_HW = 108,
  VESC_COMM_ERASE_BOOTLOADER_ALL_CAN_HW   = 109,

  VESC_COMM_SET_ODOMETER = 110,

  // Power switch commands
  VESC_COMM_PSW_GET_STATUS = 111,
  VESC_COMM_PSW_SWITCH     = 112,

  VESC_COMM_BMS_FWD_CAN_RX  = 113,
  VESC_COMM_BMS_HW_DATA     = 114,
  VESC_COMM_GET_BATTERY_CUT = 115,
  VESC_COMM_BM_HALT_REQ     = 116,
  VESC_COMM_GET_QML_UI_HW   = 117,
  VESC_COMM_GET_QML_UI_APP  = 118,
  VESC_COMM_CUSTOM_HW_DATA  = 119,
  VESC_COMM_QMLUI_ERASE     = 120,
  VESC_COMM_QMLUI_WRITE     = 121,

  // IO Board
  VESC_COMM_IO_BOARD_GET_ALL     = 122,
  VESC_COMM_IO_BOARD_SET_PWM     = 123,
  VESC_COMM_IO_BOARD_SET_DIGITAL = 124,

  VESC_COMM_BM_MEM_WRITE      = 125,
  VESC_COMM_BMS_BLNC_SELFTEST = 126,
  VESC_COMM_GET_EXT_HUM_TMP   = 127,
  VESC_COMM_GET_STATS         = 128,
  VESC_COMM_RESET_STATS       = 129,

  // Lisp
  VESC_COMM_LISP_READ_CODE   = 130,
  VESC_COMM_LISP_WRITE_CODE  = 131,
  VESC_COMM_LISP_ERASE_CODE  = 132,
  VESC_COMM_LISP_SET_RUNNING = 133,
  VESC_COMM_LISP_GET_STATS   = 134,
  VESC_COMM_LISP_PRINT       = 135,

  VESC_COMM_BMS_SET_BATT_TYPE = 136,
  VESC_COMM_BMS_GET_BATT_TYPE = 137,

  VESC_COMM_LISP_REPL_CMD    = 138,
  VESC_COMM_LISP_STREAM_CODE = 139,

  VESC_COMM_FILE_LIST   = 140,
  VESC_COMM_FILE_READ   = 141,
  VESC_COMM_FILE_WRITE  = 142,
  VESC_COMM_FILE_MKDIR  = 143,
  VESC_COMM_FILE_REMOVE = 144,

  VESC_COMM_LOG_START        = 145,
  VESC_COMM_LOG_STOP         = 146,
  VESC_COMM_LOG_CONFIG_FIELD = 147,
  VESC_COMM_LOG_DATA_F32     = 148,

  VESC_COMM_SET_APPCONF_NO_STORE = 149,
  VESC_COMM_GET_GNSS             = 150,

  VESC_COMM_LOG_DATA_F64 = 151,

  VESC_COMM_LISP_RMSG = 152,

  // Placeholders for pinlock commands
  // VESC_COMM_PINLOCK1							= 153,
  // VESC_COMM_PINLOCK2							= 154,
  // VESC_COMM_PINLOCK3							= 155,

  VESC_COMM_SHUTDOWN = 156,
} VESC_VESC_COMM_PACKET_ID;

// VESC state, TODO: use values other than rpm
static struct {
  int32_t rpm;
  int32_t current;
  uint8_t fault_code;
  uint16_t temp_fet;
} VESC_state;

static uint8_t pkt_buf[MAX_PKT_LEN];  // 520 is the maximum packet size in the
                                      // vesc firmware
static uint16_t pkt_pos = 0;          // current position in the packet buffer

static SemaphoreHandle_t xPumpSemaphore = NULL;

// sends a small payload to the VESC, allocates on stack so keep it small
static void send_payload(uint8_t *payload, uint8_t len);
// crc16 function from the VESC firmware
static unsigned short crc16(unsigned char *buf, unsigned int len);
// tries to decode a packet from the buffer
static bool try_decode_pkt();
// adds byte to buffer and shifts if necessary
static void push_byte(uint8_t byte);
// reads uart and stores in buffer
static void vReadUARTTask();
// safely sends a string to the uart
static void ts_uart_puts(char *str, uint8_t len);

// sends a small payload to the VESC
static void send_payload(uint8_t *payload, uint8_t len) {
  // calculate the crc
  uint16_t crc = crc16(payload, len);

  // create the packet
  uint8_t pkt[len + 5];
  pkt[0] = 0x02;
  pkt[1] = len;
  memcpy(&pkt[2], payload, len);
  pkt[len + 2] = (crc & 0xFF00) >> 8;
  pkt[len + 3] = (crc & 0x00FF) >> 0;
  pkt[len + 4] = 0x03;

  // send the packet
  ts_uart_puts(pkt, len + 5);
}

// function from the VESC firmware
static unsigned short crc16(unsigned char *buf, unsigned int len) {
  unsigned short cksum = 0;
  for (unsigned int i = 0; i < len; i++) {
    cksum = crc16_tab[(((cksum >> 8) ^ *buf++) & 0xFF)] ^ (cksum << 8);
  }
  return cksum;
}

static bool try_decode_pkt() {
  // check if end of packet is at the end of the buffer
  uint16_t end_pos = pkt_pos - 1;
  if (pkt_buf[end_pos] != 0x03) {
    return false;
  }

  // backtrack and look for the start of the packet
  uint16_t start_pos = 0;
  for (start_pos = end_pos; start_pos > 0; start_pos--) {
    if (pkt_buf[start_pos] >= 0x02 && pkt_buf[start_pos] <= 0x04) {
      // assuming this is the start of the packet, find the length and verify
      uint8_t len_len = pkt_buf[start_pos] - 1;
      uint16_t len    = 0;
      if (len_len == 0x01) {
        len = pkt_buf[start_pos + 1];
      } else if (len_len == 0x02) {
        len = pkt_buf[start_pos + 1] << 8 | pkt_buf[start_pos + 2];
      } else if (len_len == 0x03) {
        len = pkt_buf[start_pos + 1] << 16 | pkt_buf[start_pos + 2] << 8 |
              pkt_buf[start_pos + 3];
      }

      uint16_t dat_start = start_pos + 1 + len_len;
      uint16_t dat_end   = end_pos - 3;
      uint16_t dat_len   = dat_end - dat_start + 1;

      if (len != dat_len) {
        continue;
      }

      // calculate the crc and check if it matches the packet
      uint16_t crc     = crc16(&pkt_buf[dat_start], dat_len);
      uint16_t pkt_crc = pkt_buf[dat_end + 1] << 8 | pkt_buf[dat_end + 2];
      if (crc != pkt_crc) {
        continue;
      }

      // parse the packet and store the values
      if (pkt_buf[dat_start] == VESC_COMM_GET_VALUES) {
        uint16_t cur_offset   = dat_start + 9;
        uint16_t rpm_offset   = dat_start + 23;
        uint16_t fault_offset = dat_start + 59;
        uint16_t temp_offset  = dat_start + 1;

        VESC_state.current =
            pkt_buf[cur_offset] << 24 | pkt_buf[cur_offset + 1] << 16 |
            pkt_buf[cur_offset + 2] << 8 | pkt_buf[cur_offset + 3];

        VESC_state.rpm = pkt_buf[rpm_offset] << 24 |
                         pkt_buf[rpm_offset + 1] << 16 |
                         pkt_buf[rpm_offset + 2] << 8 | pkt_buf[rpm_offset + 3];

        VESC_state.fault_code = pkt_buf[fault_offset];

        VESC_state.temp_fet =
            pkt_buf[temp_offset] << 8 | pkt_buf[temp_offset + 1];
      }

      // reset the buffer
      pkt_pos = start_pos;

      return true;
    }
  }
  return false;
}

static void push_byte(uint8_t byte) {
  // if we're at the end of the buffer, shift
  if (pkt_pos >= MAX_PKT_LEN) {
    pkt_pos = MAX_PKT_LEN - 1;
    memmove(pkt_buf, pkt_buf + sizeof(uint8_t), MAX_PKT_LEN - 1);
    // ts_debug_printf("buffer shift\n");
  }

  // add the byte to the buffer
  pkt_buf[pkt_pos++] = byte;
  // ts_debug_printf("byte 0x%02x added to buffer\n", byte);
}

void vReadUARTTask() {
  while (1) {
    uint8_t c = uart_getc(VESC_UART_PORT);
    push_byte(c);
    if (c == 0x03) {
      try_decode_pkt();
    }
    vTaskDelay(0);
  }
}

static void ts_uart_puts(char *str, uint8_t len) {
  xSemaphoreTake(xPumpSemaphore, portMAX_DELAY);

  for (int i = 0; i < len; i++) {
    uart_putc_raw(VESC_UART_PORT, str[i]);
  }

  xSemaphoreGive(xPumpSemaphore);
}

// PORTING: this task sets up the pump and handles interfacing with it,
// including reading the pump rpm
void vPumpTask() {
  ts_debug_printf("Pump task starting...\n");
  uart_init(VESC_UART_PORT, 115200);
  uart_set_format(VESC_UART_PORT, 8, 1, UART_PARITY_NONE);
  gpio_set_function(VESC_TX_PIN, GPIO_FUNC_UART);
  gpio_set_function(VESC_RX_PIN, GPIO_FUNC_UART);

  // create the semaphore
  xPumpSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xPumpSemaphore);

  // start the uart read task
  xTaskCreate(vReadUARTTask, "Read UART Task", 512, NULL, 1, NULL);

  char read_command[]       = {VESC_COMM_GET_VALUES};
  char keep_alive_command[] = {VESC_COMM_ALIVE};

  ts_debug_printf("Pump task initialized\n");

  while (1) {
    // send the read command
    send_payload(read_command, 1);
    // send the keep alive command
    send_payload(keep_alive_command, 1);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// PORTING: this function should set the pump speed
void set_pump_rpm(int32_t rpm) {
  uint8_t rpm_command[] = {VESC_COMM_SET_RPM, 0x00, 0x00, 0x00,
                           0x00};  // null terminated

  rpm_command[1] = (uint32_t)((rpm) >> 24);
  rpm_command[2] = (uint32_t)((rpm & 0xFF0000) >> 16);
  rpm_command[3] = (uint32_t)((rpm & 0xFF00) >> 8);
  rpm_command[4] = (uint32_t)((rpm & 0xFF) >> 0);

  // send the command to the VESC
  send_payload(rpm_command, 5);
  ts_debug_printf("Pump RPM set to %u\n", rpm);
}

// PORTING: this function should return the most recently read pump rpm
uint32_t get_pump_rpm() { return VESC_state.rpm; }