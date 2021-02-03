#include <bluefruit.h>
#include <Bluebird_Teknikio.h>


#define BLUEBIRD_BEACON_LENGTH            12 


static uint8_t bluebird_beacon_data[BLUEBIRD_BEACON_LENGTH] =                /**< BLUEBIRD beacon data container. */
{
    0x01, 0x02, 0x03, 0x04, 0x05, 0x00,                                         
    0x01, 0x02, 0x03, 0x04, 0x05, 0x00                                          

};

bool tmp_blink = false;
int lastButtonState = -1;

void setup() {
  // Initialize hardware:
  Serial.begin(9600); // Serial is the USB serial port
  bluebird.begin();
  
  // Initialize Bluetooth:
  Bluefruit.begin();
  // Set max power. Accepted values are: -40, -30, -20, -16, -12, -8, -4, 0, 4
  Bluefruit.setTxPower(4);
  // Initialize the name for the board
  Bluefruit.setName("Bluebird");
  
  // Start advertising device and bleuart services
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  Bluefruit.ScanResponse.addName();

  Bluefruit.Advertising.restartOnDisconnect(true);
  // Set advertising interval (in unit of 0.625ms):
  Bluefruit.Advertising.setInterval(160, 160);
  // number of seconds in fast mode:
  Bluefruit.Advertising.setFastTimeout(30);

  // Fill the buffer for the advertised data
  bluebird_beacon_data[0]= 0x00;
  bluebird_beacon_data[1]= 0x0f;
  bluebird_beacon_data[2]= 0x11;
  bluebird_beacon_data[3]= 0x22;
  bluebird_beacon_data[4]= 0x33;
  bluebird_beacon_data[5]= 0x44;
  bluebird_beacon_data[6]= 0x55;
  bluebird_beacon_data[7]= 0x66;
  bluebird_beacon_data[8]= 0x77;
  bluebird_beacon_data[9]= 0x88;
  bluebird_beacon_data[10]= 0x99;
  bluebird_beacon_data[11]= 0xAA;

  // Add the new data on the manufacturer Data
  Bluefruit.Advertising.addManufacturerData(bluebird_beacon_data,sizeof(bluebird_beacon_data));
  // Start the advertisement
  Bluefruit.Advertising.start(0);  
}

void loop() {
  if(tmp_blink)
  {
    bluebird.setPixelColor(0,255,0,0 ); // Red
    delay(200);
    tmp_blink = false;
  }
  else{
    bluebird.setPixelColor(0,255,105,0 ); // Orange
    delay(200);
    tmp_blink = true;
  }
}
