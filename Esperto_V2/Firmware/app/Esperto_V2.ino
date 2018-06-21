#include <U8g2lib.h>
#include <SparkFunMPU9250-DMP.h>
#include <RTCZero.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <SparkFunBLEMate2.h>
#include "Adafruit_FRAM_I2C.h"
#include "avdweb_SAMDtimer.h"
#include <SPI.h>
#include <STBLE.h>
#include "esperto.h"

extern "C" char *sbrk(int i);

// Define display I2C bus
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// Define FRAM
Adafruit_FRAM_I2C FRAM = Adafruit_FRAM_I2C();

// BLE variables
char dateBT[15]; // date info MM/DD/YYYY
char timeBT[15]; // time info HH:MM:SS MM
char callBT[20]; // caller number info
char textBT[20]; // text number info
uint8_t ble_rx_buffer[21];
uint8_t ble_rx_buffer_len = 0;
uint8_t ble_connection_state = false;
int connected = FALSE;
volatile uint8_t set_connectable = 1;
uint16_t connection_handle = 0;
uint16_t UARTServHandle, UARTTXCharHandle, UARTRXCharHandle;

// Real Time Clock Variables
RTCZero rtc; // instance of RTCZero class
bool isRTCInit; // determines if RTC has been initialized
bool deviceInit = 0; // prevents time showing up without initial BLE connection

// MPU9250
MPU9250_DMP imu;
float iir_Av = 0; // IIR filter average
const int stepDiffMinThreshold = 20; // minimum difference between step max and min for considered step
int stepCount = 0; // total number of steps taken
double stepMax = 0; // peak of gyration data
double stepMin = 0; // trough of gyration data
double gyroData[3]; // array storing recent gyration readings

// Heart Rate Variables
MAX30105 heartRateSensor; // instance of MAX30105 class
const byte arraySizeHR = 5; // size of array containing latest HR values
byte heartRates[arraySizeHR]; // array containing latest HR values
byte heartRateIndex = 0; // index latest value was inputted into array
long prevHeartBeat = 0; // time at which the last heart beat occurred
float heartRate; // current heart rate
int heartRateAvg = 0; // average heart rate which will we be displayed
long timeLastHRBeat = 0;

// FRAM Variables
uint16_t countFRAM = 0; // stores the address which is going to be used by the FRAM

// 1Hz Timer ISR -- 1Hz timer which times writing to FRAM, BLE, display
volatile uint8_t ISR_CTR = 0; // counter used in addition to timer to determine when memory is being stored/sent
bool updateDisplay_flag = 0; // control when display is updated
void ISR_timer3(struct tc_module *const module_inst) 
{ 
  ISR_CTR = ISR_CTR + 1;
  updateDisplay_flag = 1; // Update display
}
SAMDtimer timer3_1Hz = SAMDtimer(3, ISR_timer3, 1e6); // FRAM timer interrupt

// Battery Variables
const uint8_t chargePin = 0; // TODO: needs to be implemented
const uint8_t batteryPin = 1; // analog 1: used to determine when battery is low/high
const float referenceVolts = 5.0; // the default reference on a 5-volt board

// Bootloader
void setup()
{
  // Used for debugging purposes
  SerialUSB.begin(9600);
  
  // Initialize display and display boot screen
  u8g2.begin();
  u8g2.firstPage();
  do {
    u8g2.drawXBMP(32, 0, 64, 64, boot);
  } while ( u8g2.nextPage() );
  
  // Setup MPU9250
  init_MPU9250();
  
  // Initialize Real Time Clock
  rtc.begin();

  // Turn on and setup heart rate sensor
  heartRateSensor.begin(Wire, I2C_SPEED_STANDARD); // 100 KHz
  heartRateSensor.setup(); // configure sensor with default settings
  heartRateSensor.setPulseAmplitudeRed(0x0A); // red LED to low to indicate sensor is running
  heartRateSensor.setPulseAmplitudeGreen(0); // turn off Green LED

  // Initialize FRAM 
  FRAM.begin();
  
  // Initialize STPBTLE-RF
  BLEsetup();

  // Initialize 1Hz timer interrupt
  timer3_1Hz.attachInterrupt(ISR_timer3);
}

// Function to update the display with latest information
// Display is updated at 1HZ or when a BLE message is recieved
void updateDisplay()
{
  // if there is an incoming phone call
  if (strlen(callBT) >= 10 && ble_connection_state == true)
  {
    u8g2.setFont(u8g2_font_profont11_tf);
    // print time top left corner
    u8g2.drawStr(0, 10, timeBT);

    u8g2.setFont(u8g2_font_profont22_tf);
    // display call text and phone number
    u8g2.setCursor(40, 38);
    u8g2.print("Call");
    u8g2.drawStr(0, 58, callBT);
  }
  // if there is an incoming text
  else if (strlen(textBT) >= 10 && ble_connection_state == true)
  {
    u8g2.setFont(u8g2_font_profont11_tf);
    // print time top left corner
    u8g2.drawStr(0, 10, timeBT);

    u8g2.setFont(u8g2_font_profont22_tf);
    // display text text and phone number
    u8g2.setCursor(40, 38);
    u8g2.print("Text");
    u8g2.drawStr(0, 58, textBT);
  }
  // if no incoming call or text
  else
  {
    u8g2.setFont(u8g2_font_profont11_tf);
    // display date
    if(ble_connection_state == true)
      u8g2.drawStr(0, 10, dateBT);
    else
    {
      u8g2.setCursor(0, 10);
      u8g2.print(String(rtc.getMonth()) + "/" + String(rtc.getDay()) + "/20" + String(rtc.getYear()));
    }

    // display heart rate
    u8g2.setCursor(14, 62);
    u8g2.print(String(heartRateAvg) + " bpm");
    u8g2.drawXBMP(0, 54, 10, 10, heart);

    // display steps
    u8g2.setCursor(78, 62);
    u8g2.print(String(stepCount*2) + " stp");
    u8g2.drawXBMP(64, 54, 10, 10, mountain);

    // display time
    u8g2.setFont(u8g2_font_profont22_tf);
    // if connected to Bluetooth
    if(ble_connection_state == true)
    {
      // center time on display
      if (strlen(timeBT) == 7)
        u8g2.drawStr(20, 38, timeBT);
      else if (strlen(timeBT) == 8)
        u8g2.drawStr(13, 38, timeBT);
    }
    // if not connected to Bluetooth
    else
    {
      u8g2.setCursor(13, 38);
      // time does not come with leading 0's, add to display string if needed
      if(rtc.getMinutes() < 10 && rtc.getSeconds() < 10)
        u8g2.print(String(rtc.getHours()) + ":0" + String(rtc.getMinutes()) + ":0" + String(rtc.getSeconds()));
      else if(rtc.getMinutes() < 10)
        u8g2.print(String(rtc.getHours()) + ":0" + String(rtc.getMinutes()) + ":" + String(rtc.getSeconds()));
      else if(rtc.getSeconds() < 10)
        u8g2.print(String(rtc.getHours()) + ":" + String(rtc.getMinutes()) + ":0" + String(rtc.getSeconds()));
      else
        u8g2.print(String(rtc.getHours()) + ":" + String(rtc.getMinutes()) + ":" + String(rtc.getSeconds()));
    }   
  }
  
  // draw BT logo only if connected
  if(ble_connection_state == true)
    u8g2.drawXBMP(118, 0, 10, 10, BT);

  // obtain and display battery status
  int battVoltRaw = analogRead(batteryPin);
  float battVolt = (battVoltRaw / 1023.0) * referenceVolts;
  // display full battery
  if (battVolt >= 3.6)
    u8g2.drawXBMP(105, 0, 10, 10, battHigh);
  // display low battery
  if (battVolt < 3.6)
    u8g2.drawXBMP(105, 0, 10, 10, battLow);
  // TO DO: ADD CHARGING PICTURE
}

void setRTCTime()
{
  byte setTimeDate;
  
  // set time (HH:MM PM)
  // determine how many bytes to read as format can be HH:MM or H:MM
  if(strlen(timeBT) == 7)
    setTimeDate = stringToByte(timeBT, 1);
  else
    setTimeDate = stringToByte(timeBT, 2);
  // determine if PM and add 12 hours if so
  if(memchr(timeBT, 'P', strlen(timeBT)) != NULL)
     rtc.setHours(setTimeDate + 12);
  else
    rtc.setHours(setTimeDate);
  // determine position of pointer (start of MM)as format can be HH:MM or H:MM
  if(strlen(timeBT) == 7)
    setTimeDate = stringToByte(timeBT+2, 2);
  else
    setTimeDate = stringToByte(timeBT+3, 2);
  rtc.setMinutes(setTimeDate);
  rtc.setSeconds(30);
  
  // set date (MM/DD/YYYY)
  setTimeDate = stringToByte(dateBT, 2);
  rtc.setMonth(setTimeDate);
  setTimeDate = stringToByte(dateBT+3, 2);
  rtc.setDay(setTimeDate);
  setTimeDate = stringToByte(dateBT+8, 2); // only get the last two digits of the year
  rtc.setYear(setTimeDate);
}

void writeFRAM()
{
  // Ensure memory is not full
  if(countFRAM < 32768)
  {
     FRAM.write8(countFRAM, heartRate);
     FRAM.write8(countFRAM+1, stepCount >> 8);
     FRAM.write8(countFRAM+2, stepCount);
     countFRAM+=4; // 4 byte alligned 
  }
}

void burstTransferFRAM()
{
  for(int i = 0; i < countFRAM; i+=20) // 20 byte packages
  {
    uint8_t blePacket[25];
    // Compile data packet
    for(int j = 0; j < 20; j++)
    {
      blePacket[j] = FRAM.read8(i+j);
    }
    // TODO: Possibly fix - add intermediate display if too slow
    Write_UART_TX((char*)blePacket, 20);
  }
  countFRAM = 0; // Go back to first FRAM address
}

void countSteps()
{
  float gyroX = imu.calcGyro(imu.gx);
  float gyroY = imu.calcGyro(imu.gy);
  float gyroZ = imu.calcGyro(imu.gz);
  
  float magGyration = sqrt(sq(gyroX) + sq(gyroY) + sq(gyroZ));
  float i = iirFilter((int) magGyration);
  
  // update recent quaternion values
  gyroData[0] = gyroData[1];
  gyroData[1] = gyroData[2];
  gyroData[2] = i; 
  
  // if a peak/max is found - compare min to 100 to remove high freq data
  if(gyroData[1] > gyroData[0] && gyroData[1] > gyroData[2] && stepMin < 100)
  {
    stepMax = gyroData[1];// update peak value      
    double maxMinDiff = stepMax - stepMin; 
    // if a step is detected
    if (maxMinDiff > stepDiffMinThreshold)
    {
        stepCount++;
    }
  }
  // if a trough/min is found - 100 compared to reduce high freq noise
  else if(gyroData[1] < gyroData[0] && gyroData[1] < gyroData[2])
  {
    stepMin = gyroData[1];
  }
}

// Function used to calculate users heart rate
void calculateHR()
{
  // obtain infrared value
  long irValue = heartRateSensor.getIR();

  // if heart beat was detected and valid IR value
  if (checkForBeat(irValue) == true && irValue > 50000)
  {
    // calculate time difference between 2 beats
    long heartBeatTimeDiff = millis() - prevHeartBeat;
    prevHeartBeat = millis();

    // use the difference to calculate the heart rate
    heartRate = 60 / (heartBeatTimeDiff / 1000.0); // 60 s in 1 min, 1000 ms in 1 s

    // only use valid heart rates
    if (heartRate < 120 && heartRate > 40)
    {
      // store heart rate
      heartRates[heartRateIndex++] = (byte)heartRate;
      heartRateIndex %= arraySizeHR; // use modulus op. to determine current index

      // calcute average heart rate
      heartRateAvg = 0; // reset
      for (int i = 0; i < arraySizeHR; i++)
        heartRateAvg += heartRates[i]; // add up all heart rates
      heartRateAvg /= arraySizeHR; // determine average by dividing

      timeLastHRBeat = millis();
    }
  }
}

void loop()
{
  //Process any ACI commands or events from BLE
  aci_loop();
  
  //Check if data is available
  if (ble_rx_buffer_len) 
  { 
    if(strncmp((const char*)ble_rx_buffer, "D:", 2) == 0)
      strcpy(dateBT, (const char*)ble_rx_buffer+2);
    else if(strncmp((const char*)ble_rx_buffer, "T:", 2) == 0)
      strcpy(timeBT, (const char*)ble_rx_buffer+2);
    else if(strncmp((const char*)ble_rx_buffer, "C:", 2) == 0)
      strcpy(callBT, (const char*)ble_rx_buffer+2);
    else if(strncmp((const char*)ble_rx_buffer, "M:", 2) == 0)
      strcpy(textBT, (const char*)ble_rx_buffer+2);
    ble_rx_buffer_len = 0;//clear afer reading

    // update display
    u8g2.firstPage();
    do {
      updateDisplay();
    } while ( u8g2.nextPage() );

    // If device was just disconnected
    if(isRTCInit == true || !deviceInit)
    {
      burstTransferFRAM();
      isRTCInit = false;
    }

    deviceInit = 1; // Established initial BLE connection
  }

  if(ble_connection_state == false && updateDisplay_flag && deviceInit)
  {
    // set time based off last Bluetooth connection
    if(isRTCInit == false)
    {
      setRTCTime();
      isRTCInit = true;
    }
    
    // update display
    u8g2.firstPage();
    do {
      updateDisplay();
    } while ( u8g2.nextPage() );
    
    // Reset display flag
    updateDisplay_flag = 0;
  }

  // Check for new data in the MPU9250 FIFO
  if ( imu.fifoAvailable() )
  {
    // Update accel and gyro values
    if ( imu.dmpUpdateFifo() == INV_SUCCESS)
    {
        countSteps();
    }
  }
  
  // Calculate heart rate (Check for validity first - LPF)
  if(millis()-timeLastHRBeat > 500)
  {
      calculateHR(); 
  }

  // Determine whether data is sent to FRAM or BLE
  // Write to FRAM every 60 seconds
  if(ISR_CTR >= 60 && !ble_connection_state)
  {
    writeFRAM();
    ISR_CTR = 0;
  }
  // Write to BLE every 30 seconds
  else if(ISR_CTR >= 30 && ble_connection_state)
  {
    // TODO: Possibly fix
    uint8_t txBuf[5];
    txBuf[0] = heartRate;
    txBuf[1] = stepCount >> 8;
    txBuf[2] = stepCount;
    Write_UART_TX((char*)txBuf, 4);
    ISR_CTR = 0;
  }
}

// HELPER FUNCTIONS
// Initializes BLE and puts it into peripheral mode
void BLEsetup()
{
  HCI_Init();
  //Init SPI interface
  BNRG_SPI_Init();
  // Reset BlueNRG/BlueNRG-MS SPI interface
  BlueNRG_RST();

  // Set BD Address
  uint8_t bdaddr[] = {0x12, 0x34, 0x00, 0xE1, 0x80, 0x02};
  aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET, CONFIG_DATA_PUBADDR_LEN, bdaddr);

  // Initialize GAP
  aci_gatt_init();
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  aci_gap_init_IDB05A1(GAP_PERIPHERAL_ROLE_IDB05A1, 0, 0x07, &service_handle, &dev_name_char_handle, &appearance_char_handle);

  // Initialize BLE stack
  const char *name = "BlueNRG";
  aci_gatt_update_char_value(service_handle, dev_name_char_handle, 0, strlen(name), (uint8_t *)name);

  // Add UART service
  Add_UART_Service();

  // Set output power to +4 dBm
  aci_hal_set_tx_power_level(1, 3);
}

void aci_loop() {
  HCI_Process();
  ble_connection_state = connected;
  if (set_connectable) {
    setConnectable();
    set_connectable = 0;
  }
}

void Add_UART_Service(void)
{
  uint8_t uuid[16];

  COPY_UART_SERVICE_UUID(uuid);
  aci_gatt_add_serv(UUID_TYPE_128,  uuid, PRIMARY_SERVICE, 7, &UARTServHandle);

  COPY_UART_TX_CHAR_UUID(uuid);
  aci_gatt_add_char(UARTServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_WRITE_WITHOUT_RESP, ATTR_PERMISSION_NONE, GATT_NOTIFY_ATTRIBUTE_WRITE,
                           16, 1, &UARTTXCharHandle);

  COPY_UART_RX_CHAR_UUID(uuid);
  aci_gatt_add_char(UARTServHandle, UUID_TYPE_128, uuid, 20, CHAR_PROP_NOTIFY, ATTR_PERMISSION_NONE, 0,
                           16, 1, &UARTRXCharHandle);
}

void Write_UART_TX(char* TXdata, uint8_t datasize)
{
  aci_gatt_update_char_value(UARTServHandle, UARTRXCharHandle, 0, datasize, (uint8_t *)TXdata);
}

void setConnectable(void)
{
  // Set device to connectable
  const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME, 'E', 's', 'p', 'e', 'r', 't', 'o', ' ', 'W', 'a', 't', 'c', 'h'};
  hci_le_set_scan_resp_data(0, NULL);
  aci_gap_set_discoverable(ADV_IND,
                           (ADV_INTERVAL_MIN_MS * 1000) / 625, (ADV_INTERVAL_MAX_MS * 1000) / 625,
                           STATIC_RANDOM_ADDR, NO_WHITE_LIST_USE,
                           sizeof(local_name), local_name, 0, NULL, 0, 0);
}

void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data)
{
  if (handle == UARTTXCharHandle + 1) {
    int i;
    for (i = 0; i < data_length; i++) {
      ble_rx_buffer[i] = att_data[i];
    }
    ble_rx_buffer[i] = '\0';
    ble_rx_buffer_len = data_length;
  }
}

void HCI_Event_CB(void *pckt)
{
  hci_uart_pckt *hci_pckt = (hci_uart_pckt *)pckt;
  hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;

  if (hci_pckt->type != HCI_EVENT_PKT)
    return;

  switch (event_pckt->evt) {

    case EVT_DISCONN_COMPLETE:
      {
        // Make the device connectable again
        connected = FALSE;
        set_connectable = TRUE;
      }
      break;

    case EVT_LE_META_EVENT:
      {
        evt_le_meta_event *evt = (evt_le_meta_event *)event_pckt->data;
        switch (evt->subevent)
        {
          case EVT_LE_CONN_COMPLETE:
            {
              evt_le_connection_complete *cc = (evt_le_connection_complete *)evt->data;
              connected = TRUE;
              connection_handle = cc->handle;
            }
            break;
        }
      }
      break;

    case EVT_VENDOR:
      {
        evt_blue_aci *blue_evt = (evt_blue_aci *)event_pckt->data;
        switch (blue_evt->ecode)
        {
          case EVT_BLUE_GATT_READ_PERMIT_REQ:
            {
              evt_gatt_read_permit_req *pr = (evt_gatt_read_permit_req *)blue_evt->data;
              if (connection_handle != 0)
                  aci_gatt_allow_read(connection_handle);
            }
            break;

          case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
            {
              evt_gatt_attr_modified_IDB05A1 *evt = (evt_gatt_attr_modified_IDB05A1*)blue_evt->data;
              Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data);
            }
            break;
        }
      }
      break;
  }
}

void init_MPU9250()
{
  // Verify communication and init default values
  imu.begin();

  // Enable 3DOF gyro and acceleromter
  imu.setSensors(INV_XYZ_GYRO | INV_XYZ_ACCEL); 
  imu.setGyroFSR(2000); // Set gyro to 2000 dps

  // Initialize gyro and step detection features
  unsigned short dmpFeatureMask = 0;
  dmpFeatureMask |= DMP_FEATURE_GYRO_CAL;
  dmpFeatureMask |= DMP_FEATURE_SEND_CAL_GYRO;
  imu.dmpBegin(dmpFeatureMask);
}

float iirFilter(int flexVal)
{
  iir_Av = iir_Av + ((float)flexVal - iir_Av)/16;
  return iir_Av;
}

// Function which replaces char(s) with new char(s) in a char array
void str_replace(char *src, char *oldchars, char *newchars)
{
  char *p = strstr(src, oldchars); // returns a pointer to the first occurrence of oldchars in src
  char buf[20];
  do {
    memset(buf, 0, strlen(buf));
    // until pointer points to null or zero
    if (p) {
      // if replacing char from beginining
      if (src == p) {
        strcpy(buf, newchars);
        strcat(buf, p + strlen(oldchars));
      } else {
        strncpy(buf, src, strlen(src) - strlen(p));
        strcat(buf, newchars);
        strcat(buf, p + strlen(oldchars));
      }
      memset(src, 0, strlen(src));
      strcpy(src, buf);
    }
  } while (p && (p = strstr(src, oldchars)));
}

// Function which convert a number in a char array to a byte data type
byte stringToByte(char *src, int numBytes)
{
  // intermediate buffers used during conversions
  char charBuffer[4];

  memcpy(charBuffer, src, numBytes);

  // cast from char[] to int to byte
  return (byte)atoi(charBuffer);
}

// Function which determines amount of free SRAM
int freeRam () 
{
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}
