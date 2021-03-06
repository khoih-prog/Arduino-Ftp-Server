/*
 * This sketch demonstrate how to retieve FTP server state
 * Copyright (c) 2014-2020 by Jean-Michel Gallego
 *
 * You may have to modify some of the definitions
 * Please read comments in example FtpServerTest
 *
 * Server state is returned by ftpSrv.service() and his value is
 *    cmdStage | ( transferStage << 3 ) | ( dataConn << 6 )
 *  where cmdStage, transferStage and dataConn are private variables of class
 *  FtpServer. The values that they can take are defined respectively by
 *  ftpCmd, ftpTransfer and ftpDataConn in FtpServer.h
 *  
 * For example:
 *   if(( ftpSrv.service() & 0x07 ) > 0 ) ...
 *  to determine that a client is connected
 *  
 *   if(( ftpSrv.service() & 0x38 ) == 0x10 ) ...
 *  to determine that a STORE command is being performed (uploading a file)
 *  
 *   if(( ftpSrv.service() & 0xC0 ) == 0x40 ) ...
 *  to determine that the server is in pasive mode
 *  
 */

#include <FtpServer.h>

// Define Chip Select for your SD card according to hardware 
// #define CS_SDCARD 4  // SD card reader of Ehernet shield
#define CS_SDCARD 53 // Chip Select for SD card reader on Due

// Define Reset pin for W5200 or W5500
// set to -1 for other ethernet chip or if Arduino reset board is used
#define P_RESET -1
// #define P_RESET 8

// Define pin for led
#define LED_PIN LED_BUILTIN
// #define LED_PIN 5

FtpServer ftpSrv;

// Mac address of ethernet adapter
// byte mac[] = { 0x90, 0xa2, 0xda, 0x00, 0x00, 0x00 };
// byte mac[] = { 0x00, 0xaa, 0xbb, 0xcc, 0xde, 0xef };
byte mac[] = { 0xde, 0xad, 0xbe, 0xef, 0xfe, 0xef };

// IP address of ethernet adapter
// if set to 0, use DHCP for the routeur to assign IP
// IPAddress serverIp( 192, 168, 1, 40 );
IPAddress serverIp( 0, 0, 0, 0 );

/*******************************************************************************
**                                                                            **
**                               INITIALISATION                               **
**                                                                            **
*******************************************************************************/

void setup()
{
  Serial.begin( 115200 );
  Serial << F( "=== FTP Server state Led ===" ) << eol;

  // initialize digital pin LED_PIN as an output.
  pinMode( LED_PIN, OUTPUT );
  // turn the LED off
  digitalWrite( LED_PIN, LOW );

  // If other chips are connected to SPI bus, set to high the pin connected to their CS
  // pinMode( 4, OUTPUT ); 
  // digitalWrite( 4, HIGH );

  // Initialize the SD card.
  Serial << F("Mount the SD card with library ");
  #if FAT_USE == FAT_SDFAT
    Serial << F("SdFat ... ");
  #else
    Serial << F("FatFs ... ");
  #endif
  if( ! FAT_FS.begin( CS_SDCARD, SD_SCK_MHZ( 50 )))
  {
    Serial << F("Unable to mount SD card") << eol;
    while( true ) ;
  }
  pinMode( CS_SDCARD, OUTPUT ); 
  digitalWrite( CS_SDCARD, HIGH );
  Serial << F("ok") << eol;

  // Send reset to Ethernet module
  if( P_RESET > -1 )
  {
    pinMode( P_RESET, OUTPUT );
    digitalWrite( P_RESET, LOW );
    delay( 200 );
    digitalWrite( P_RESET, HIGH );
    delay( 200 );
  }

  // Initialize the network
  Serial << F("Initialize ethernet module ... ");
  if((uint32_t) serverIp != 0 )
    Ethernet.begin( mac, serverIp );
  else if( Ethernet.begin( mac ) == 0 )
  {
    Serial << F("failed!") << eol;
    while( true ) ;
  }
  Serial << F("ok") << eol;

  // Initialize the FTP server
  ftpSrv.init();
  ftpSrv.credentials( "myname", "123" );
}

/*******************************************************************************
**                                                                            **
**                                 MAIN LOOP                                  **
**                                                                            **
*******************************************************************************/

void loop()
{
  static uint8_t state0;
  static uint32_t tick = 0;
  
  uint8_t state = ftpSrv.service();
  if( state0 != state )
  {
    tick = 0;
    if(( state & 0x07 ) <= 2 )       // no client connected
      digitalWrite( LED_PIN, LOW );
    else if(( state & 0x38 ) == 0 )  // client connected but no data transfer
      digitalWrite( LED_PIN, HIGH );
    else
    {
      digitalWrite( LED_PIN, LOW );
      tick = millis() + 100;
    }
    state0 = state;
  }

  if( tick > 0 && (int32_t) ( millis() - tick ) > 0 )
  {
    digitalWrite( LED_PIN, ! digitalRead( LED_PIN ));
    tick = millis() + 100;
  }
 
  // more processes... 
}
