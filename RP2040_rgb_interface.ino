/* ----------------------------------------------------------------------
"Simple" Protomatter library example sketch (once you get past all
the various pin configurations at the top, and all the comments).
Shows basic use of Adafruit_Protomatter library with different devices.

This example is written for a 64x32 matrix but can be adapted to others.

Once the RGB matrix is initialized, most functions of the Adafruit_GFX
library are available for drawing -- code from other projects that use
LCDs or OLEDs can be easily adapted, or may be insightful for reference.
GFX library is documented here:
https://learn.adafruit.com/adafruit-gfx-graphics-library
------------------------------------------------------------------------- */

#include <Adafruit_Protomatter.h>
#include <Wire.h>
#include <Fonts/FreeSansBold18pt7b.h> // Large friendly font

/* ----------------------------------------------------------------------
The RGB matrix must be wired to VERY SPECIFIC pins, different for each
microcontroller board. This first section sets that up for a number of
supported boards. Notes have been moved to the bottom of the code.
------------------------------------------------------------------------- */

// RP2040 support requires the Earle Philhower board support package;
// will not compile with the Arduino Mbed OS board package.
// The following pinout works with the Adafruit Feather RP2040 and
// original RGB Matrix FeatherWing (M0/M4/RP2040, not nRF version).
// Pin numbers here are GP## numbers, which may be different than
// the pins printed on some boards' top silkscreen.
uint8_t rgbPins[]  = {0, 1, 2, 3, 4, 5};
uint8_t addrPins[] = {6, 7, 8, 9};
uint8_t clockPin   = 11;
uint8_t latchPin   = 12;
uint8_t oePin      = 13;


Adafruit_Protomatter matrix(
  32,          // Width of matrix (or matrix chain) in pixels
  1,           // Bit depth, 1-6
  1, rgbPins,  // # of matrix chains, array of 6 RGB pins for each
  3, addrPins, // # of address pins (height is inferred), array of pins
  clockPin, latchPin, oePin, // Other matrix control pins
  false,       // No double-buffering here (see "doublebuffer" example)
  -2);

// SETUP - RUNS ONCE AT PROGRAM START --------------------------------------

//TwoWire Wire2(i2c0, 20, 21);

struct r_data {
  uint8_t len;
  uint8_t *payload;
};

struct cmd_buffer {
  uint8_t id;
  uint8_t op;
  uint8_t len;
  uint8_t payload[10];
};

#define MAX_COMMAND_LIST (10)
// Command processing buffer
struct cmd_buffer cmd_list[MAX_COMMAND_LIST];
volatile int cmd_list_write_idx = 0;
volatile int cmd_list_read_idx = 0;
volatile uint8_t cmd_list_id = 0;
static volatile bool running = false;

enum G_COMMAND {
    G_NONE = 0,
    G_CLEAR,
    G_POINT,
    G_LINE,
    G_CIRCLE,
    G_RECTANGLE,
    G_TEXT,
    G_SCROLL,
  G_INIT
};

#define PALETTE_LENGTH (8)
uint16_t pallete[PALETTE_LENGTH] = {
  0x0000,
  0xf800,
  0xf81f,
  0xffe0,
  0x07E0,
  0x07FF,
  0x001F,
  0xFFFF
};

void receiveEvent(int howMany)
{
//  Serial.print(Wire.available());
//  Serial.print("  = ");
  if (Wire.available() == 10) // loop through all 
  {
    uint8_t c = Wire.read();     // receive byte as a character
//    Serial.print(c);         // print the character

    // Store command in command list.
    if ( ((cmd_list_write_idx + 1) % MAX_COMMAND_LIST) == cmd_list_read_idx)
    {
      // Full!
      cmd_list_read_idx = (cmd_list_read_idx + 1) % MAX_COMMAND_LIST;
    }
    else
    {
      cmd_list[cmd_list_write_idx].id = cmd_list_id++;
      cmd_list[cmd_list_write_idx].op = c;

      for (int i=0; i<9 && Wire.available() > 0; i++)
        cmd_list[cmd_list_write_idx].payload[i] = Wire.read();
      cmd_list_write_idx = (cmd_list_write_idx + 1) % MAX_COMMAND_LIST;
    }
  }
  else
  {
    // Discard all?
    while (Wire.available() > 0)
    {
      char c = Wire.read(); // receive byte as a character
      //Serial.print(c);         // print the character
    }
//    Serial.println();         // print the integer
  }
}

void setup(void) {
  Wire.setSDA(20);
  Wire.setSCL(21);
  Wire.begin(126);                // join i2c bus with address #4
  Wire.onReceive(receiveEvent); // register event
  Serial.begin(9600);

  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
  if(status != PROTOMATTER_OK) {
    // DO NOT CONTINUE if matrix setup encountered an error.
    for(;;);
  }

  matrix.setFont(&FreeSansBold18pt7b); // Use nice bitmap font
  matrix.setTextWrap(false);           // Allow text off edge
  
  // AFTER DRAWING, A show() CALL IS REQUIRED TO UPDATE THE MATRIX!
  matrix.show(); // Copy data to matrix buffers
}

// LOOP - RUNS REPEATEDLY AFTER SETUP --------------------------------------

void loop(void) {
  
  handle_i2cs_xfer();
}

/**
 * This loop waits for incoming I2CS requests, and processes any read/write
 * requests in an appropriate order.
 */
void handle_i2cs_xfer(void)
{
  struct cmd_buffer pdata;

  if (cmd_list_read_idx != cmd_list_write_idx)
  {
    // Copy next command
    int read_idx = cmd_list_read_idx;
    memcpy(&pdata, &cmd_list[read_idx], sizeof(struct cmd_buffer));
    cmd_list_read_idx = (cmd_list_read_idx + 1) % MAX_COMMAND_LIST;

//    if (pdata.len != 10) return 0;
//    Serial.print(pdata.op);
//    Serial.print(" ");
//    Serial.print(pdata.payload[0]);
//    Serial.print(" ");
//    Serial.print(pdata.payload[1]);
//    Serial.print(" ");
//    Serial.println(pdata.payload[2]);

    // Process command
    switch(pdata.op)
    {
    case G_INIT:
    {
      // Re-init the display
//      running = false;
//      SysTick_DelayTicks(2);
//
//      uint8_t wdth = pdata.payload[0];
//      uint8_t hight = pdata.payload[1];
//      if (wdth == 0) wdth = 1;
//      if (wdth > 2) wdth = 2;
//      if (hight == 0) hight = 1;
//      if (hight > 2) hight = 2;
//
//      DMD_free();
//      DMD(wdth, hight); // todo

      running = true;
    }
    break;

    case G_CLEAR:
    {
      matrix.fillScreen( 0 );
    }
    break;

    case G_POINT:
    {
      matrix.drawPixel(pdata.payload[0],
            pdata.payload[1],
            pallete[ pdata.payload[2] ] );
    }
    break;

    case G_LINE:
    {
      matrix.drawLine(pdata.payload[0],
            pdata.payload[1],
            pdata.payload[2],
            pdata.payload[3],
            pallete[ pdata.payload[4] ] );
    }
    break;

    case G_CIRCLE:
    {
      if (pdata.payload[3] == 0)
      {
        matrix.drawCircle(pdata.payload[0], pdata.payload[1], pdata.payload[2], 
                            pallete[ pdata.payload[4] ] );
      }
      else
      {
        matrix.fillCircle(pdata.payload[0], pdata.payload[1], pdata.payload[2], 
                            pallete[ pdata.payload[4] ] );
      }

    }
    break;

    case G_RECTANGLE:
    {
      if (pdata.payload[4] == 0)
      {
        matrix.drawRect(pdata.payload[0],
            pdata.payload[1],
            pdata.payload[2],
            pdata.payload[3],
            pallete[ pdata.payload[4] ] );
      }
      else
      {
        matrix.fillRect(pdata.payload[0],
            pdata.payload[1],
            pdata.payload[2],
            pdata.payload[3],
            pallete[ pdata.payload[4] ] );
      }
    }
    break;

    case G_TEXT:
    {
//      Serial.println("Text");
////      char str[6] = {0};
////      int len;
////      for (int c; c<5; c++)
////      {
////        len = c;
////        if (pdata.payload[3 + c] == 0)
////          break;
////        str[c] = pdata.payload[3 + c];
////      }
//      pdata.payload[9] = 0;
//      matrix.setTextColor( pallete[ pdata.payload[2] ] );
//      matrix.setCursor(pdata.payload[0], pdata.payload[1]);
//      matrix.print(pdata.payload[3]);
    }
    break;

    default: break;
    }

    matrix.show();
  }

}
