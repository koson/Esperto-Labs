/*
  ******************************************************************************
  * @file    esperto.h
  * @author  Daniel De Sousa
  * @version V1.0.0
  * @date    18-May-2018
  * @brief   
  ******************************************************************************
*/ 

#ifndef __ESPERTO_H
#define __ESPERTO_H

#include <stdint.h>

// BLE Variables -- START

#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
  do {\
    uuid_struct[0] = uuid_0; uuid_struct[1] = uuid_1; uuid_struct[2] = uuid_2; uuid_struct[3] = uuid_3; \
    uuid_struct[4] = uuid_4; uuid_struct[5] = uuid_5; uuid_struct[6] = uuid_6; uuid_struct[7] = uuid_7; \
    uuid_struct[8] = uuid_8; uuid_struct[9] = uuid_9; uuid_struct[10] = uuid_10; uuid_struct[11] = uuid_11; \
    uuid_struct[12] = uuid_12; uuid_struct[13] = uuid_13; uuid_struct[14] = uuid_14; uuid_struct[15] = uuid_15; \
  }while(0)

#define COPY_UART_SERVICE_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x6E, 0x40, 0x00, 0x01, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)
#define COPY_UART_TX_CHAR_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x6E, 0x40, 0x00, 0x02, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)
#define COPY_UART_RX_CHAR_UUID(uuid_struct)  COPY_UUID_128(uuid_struct,0x6E, 0x40, 0x00, 0x03, 0xB5, 0xA3, 0xF3, 0x93, 0xE0, 0xA9, 0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E)

#define  ADV_INTERVAL_MIN_MS  50
#define  ADV_INTERVAL_MAX_MS  100

// BLE Variables -- END

// Display Variables -- START

// Bootloader logo: Width x Height = 64,64
static const uint8_t boot[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xf8, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0xff, 0xfd, 0x03, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff,
  0xef, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x3f, 0x00, 0x00,
  0x00, 0x00, 0xfe, 0xff, 0x7f, 0x5b, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
  0xef, 0xff, 0x01, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0xf6, 0x07, 0x00,
  0x00, 0xe0, 0xff, 0xff, 0xec, 0xdf, 0x06, 0x00, 0x00, 0xf0, 0xff, 0x3f,
  0xf8, 0xf7, 0x01, 0x00, 0x00, 0xe0, 0xff, 0x1f, 0xd0, 0x7e, 0x01, 0x00,
  0x00, 0xe0, 0xff, 0x07, 0xc0, 0x17, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x01,
  0x00, 0x1e, 0x00, 0x00, 0x00, 0xf0, 0x7f, 0x00, 0x00, 0x02, 0x00, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xf0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xf0, 0x7f, 0x00,
  0x80, 0x3f, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0xe0, 0x7f, 0x00, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0xf8, 0xff, 0x01, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0xfc, 0xff, 0x07, 0x00, 0x00, 0xf0, 0x7f, 0x00, 0xfe, 0xff, 0x07, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0xfc, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0xf0, 0xff, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0xc0, 0x7f, 0x00, 0x00,
  0x00, 0xf0, 0x7f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0x00, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xf0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x00, 0x00, 0x0f, 0x00, 0x00,
  0x00, 0xe0, 0xff, 0x03, 0x80, 0x3f, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x0f,
  0xe0, 0x7f, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 0xf8, 0xff, 0x01, 0x00,
  0x00, 0xf0, 0xff, 0x7f, 0xfe, 0xff, 0x07, 0x00, 0x00, 0xe0, 0xff, 0xff,
  0xff, 0xff, 0x0f, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0xfc, 0xff,
  0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x1f, 0x00, 0x00,
  0x00, 0x00, 0xe0, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
  0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
  0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Heartrate logo: Width x Height = 10,10
static const unsigned char heart[] = {
  0xce, 0x01, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xfe, 0x01,
  0xfc, 0x00, 0x78, 0x00, 0x30, 0x00, 0x20, 0x00
};

// Step logo: Width x Height = 10,10
static const unsigned char mountain[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0xfc, 0x00,
  0xfe, 0x01, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Bluetooth logo: Width x Height = 10,10
static const unsigned char BT[] = {
  0x20, 0x00, 0x70, 0x00, 0xec, 0x00, 0xf8, 0x00, 0x70, 0x00, 0x70, 0x00,
  0xf8, 0x00, 0xec, 0x00, 0x70, 0x00, 0x20, 0x00
};

// Full battery logo: Width x Height = 10,10
static const uint8_t battHigh[] = {
   0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03,
   0xff, 0x03, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00 
};
   
// Low battery logo: Width x Height = 10,10
static const uint8_t battLow[] = {
   0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 0x01, 0x03, 0x01, 0x03, 0x01, 0x03,
   0x01, 0x03, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00 
};

static const uint8_t battMed[] = {
   0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 0x1f, 0x03, 0x1f, 0x03, 0x1f, 0x03,
   0x1f, 0x03, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00 
};

// Display variables -- END

#endif 