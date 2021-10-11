#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// PINS
#define SCK_PIN 10 //SClock
#define MOSI_PIN 11 //SData
#define RES_PIN 12 //Reset
#define DC_PIN 13  //Data/Command
#define BK_PIN 14 //Backlight

#define SPI_PORT spi1

// Display Settings
#define DISP_WIDTH 240
#define DISP_HEIGHT 240
#define CENTER_Y (DISP_WIDTH/2)
#define CENTER_X (DISP_HEIGHT/2)

/*******************************************************************************/
// commands
static const uint8_t  ST7789_NOP = 0x00;
static const uint8_t  ST7789_SWRESET = 0x01;
static const uint8_t  ST7789_RDDID = 0x04;
static const uint8_t  ST7789_RDDST = 0x09;

static const uint8_t  ST7789_SLPIN = 0x10;
static const uint8_t  ST7789_SLPOUT = 0x11;
static const uint8_t  ST7789_PTLON = 0x12;
static const uint8_t  ST7789_NORON = 0x13;

static const uint8_t  ST7789_INVOFF = 0x20;
static const uint8_t  ST7789_INVON = 0x21;
static const uint8_t  ST7789_DISPOFF = 0x28;
static const uint8_t  ST7789_DISPON = 0x29;
static const uint8_t  ST7789_CASET = 0x2A;
static const uint8_t  ST7789_RASET = 0x2B;
static const uint8_t  ST7789_RAMWR = 0x2C;
static const uint8_t  ST7789_RAMRD = 0x2E;

static const uint8_t  ST7789_PTLAR = 0x30;
static const uint8_t  ST7789_VSCRDEF = 0x33;
static const uint8_t  ST7789_COLMOD = 0x3A;
static const uint8_t  ST7789_MADCTL = 0x36;
static const uint8_t  ST7789_VSCSAD = 0x37;

static const uint8_t  ST7789_MADCTL_MY = 0x80;
static const uint8_t  ST7789_MADCTL_MX = 0x40;
static const uint8_t  ST7789_MADCTL_MV = 0x20;
static const uint8_t  ST7789_MADCTL_ML = 0x10;
static const uint8_t  ST7789_MADCTL_BGR = 0x08;
static const uint8_t  ST7789_MADCTL_MH = 0x04;
static const uint8_t  ST7789_MADCTL_RGB = 0x00;

static const uint8_t  ST7789_RDID1 = 0xDA;
static const uint8_t  ST7789_RDID2 = 0xDB;
static const uint8_t  ST7789_RDID3 = 0xDC;
static const uint8_t  ST7789_RDID4 = 0xDD;

static const uint8_t  COLOR_MODE_65K = 0x50;
static const uint8_t  COLOR_MODE_262K = 0x60;
static const uint8_t  COLOR_MODE_12BIT = 0x03;
static const uint8_t  COLOR_MODE_16BIT = 0x05;
static const uint8_t  COLOR_MODE_18BIT = 0x06;
static const uint8_t  COLOR_MODE_16M = 0x07;

/*******************************************************************************/
// Color voidinitions
static const uint16_t  BLACK = 0x0000;
static const uint16_t  BLUE = 0x001F;
static const uint16_t  RED = 0xF800;
static const uint16_t  GREEN = 0x07E0;
static const uint16_t  CYAN = 0x07FF;
static const uint16_t  MAGENTA = 0xF81F;
static const uint16_t  YELLOW = 0xFFE0;
static const uint16_t  WHITE = 0xFFFF;

static const int _BUFFER_SIZE = 256;

static const uint8_t _BIT7 = 0x80;
static const uint8_t _BIT6 = 0x40;
static const uint8_t _BIT5 = 0x20;
static const uint8_t _BIT4 = 0x10;
static const uint8_t _BIT3 = 0x08;
static const uint8_t _BIT2 = 0x04;
static const uint8_t _BIT1 = 0x02;
static const uint8_t _BIT0 = 0x01;
/*******************************************************************************/
void display_init();
uint16_t convert_rgb_to_hex(int red, int green, int blue);
void _writeCommand(uint8_t command);
void _writeData(uint8_t data[], uint32_t Len);
void hard_reset();
void soft_reset();
void sleep_mode(bool value);
void inversion_mode(int value);
void _set_color_mode(uint8_t mode);
void rotation(int rotation); //Set display rotation. Args:rotation (int): 0-Portrait 1-Landscape 2-Inverted Portrait 3-Inverted Landscape
void _set_columns(int start, int end);
void _set_rows(int start, int end);
void _set_window(int x0, int y0, int x1, int y1);
void vline(int x, int y, int length, uint16_t color);
void hline(int x, int y, int length, uint16_t color);
void pixel(int x, int y, uint16_t color);
void blit_buffer(uint8_t buffer[], uint8_t bufferLen, int x, int y, int width, int height);
void rect(int x, int y, int w, int h, uint16_t color);
void fill_rect(int x, int y, int width, int height, uint16_t color);
void fill(uint16_t color);
void line(int x0, int y0, int x1, int y1, uint16_t color);
void vscrdef(int tfa, int vsa, int bfa);
void vscsad(int vssa);
/*******************************************************************************/
uint16_t convert_rgb_to_hex(int red, int green, int blue){
    //Convert red, green and blue values (0-255) into a 16-bit 565 encoding
    return (red & 0xf8) << 8 | (green & 0xfc) << 3 | blue >> 3;
}

//Initialize display
void display_init(){
    hard_reset();
    soft_reset();
    sleep_mode(false);

    //_set_color_mode (COLOR_MODE_65K | COLOR_MODE_16BIT);
    _set_color_mode(COLOR_MODE_16BIT);
    sleep_ms(50);
    rotation(0);
    inversion_mode(true);
    sleep_ms(10);
    _writeCommand(ST7789_NORON);
    sleep_ms(10);
    gpio_put(BK_PIN, 1);    //Turn on backlight
    fill(BLACK);
    _writeCommand(ST7789_DISPON);
    sleep_ms(500);
}

void _writeCommand(uint8_t command){
    gpio_put(25, 0);
    gpio_put(DC_PIN, 0);
    spi_write_blocking(SPI_PORT, &command, 1);
}

void _writeData(uint8_t data[], uint32_t Len){
    gpio_put(25, 1);
    gpio_put(DC_PIN, 1);
    spi_write_blocking(SPI_PORT, data, Len);
}

void hard_reset()   // Hard reset display.
{
    gpio_put(RES_PIN, 1);
    sleep_ms(50);
    gpio_put(RES_PIN, 0);
    sleep_ms(50);
    gpio_put(RES_PIN, 1);
    sleep_ms(150);
}

void soft_reset()   // Soft reset display.
{
    _writeCommand(ST7789_SWRESET);
    sleep_ms(150);
}

void sleep_mode(bool value) //Enable(1) or disable(0) display sleep mode. 
{
    if(value){
        _writeCommand(ST7789_SLPIN);
    }else{
        _writeCommand(ST7789_SLPOUT);
    }
}

void inversion_mode(int value)  //Enable(1) or disable(0) display inversion mode.
{
    if(value){
        _writeCommand(ST7789_INVON);
    }else{
        _writeCommand(ST7789_INVOFF);
    }
}

void _set_color_mode(uint8_t mode)  //Set display color mode.
{
    _writeCommand(ST7789_COLMOD);
    uint8_t data[] = { mode, 0x77 };
    _writeData(data, 2);
}

void rotation(int rotation){
    /*Set display rotation.
        Args:
            rotation (int):
                - 0-Portrait
                - 1-Landscape
                - 2-Inverted Portrait
                - 3-Inverted Landscape*/
    _writeCommand(ST7789_MADCTL);
    uint8_t data[] = {0x00};
    _writeData(data, 1);
}

void _set_columns(int start, int end){
    //Send CASET (column address set) command to display.
    //Args: start (int): column start address end (int): column end address
    if(start > end > DISP_WIDTH) return;
    uint8_t _x = start;
    uint8_t _y = end;
    uint8_t result[] = { 0x00, 0x00, _x, _y };
    _writeCommand(ST7789_CASET);
    _writeData(result, 5);
}

void _set_rows(int start, int end){
    //Send RASET (row address set) command to display.
    //Args: start (int): row start address, end (int): row end address
    if(start > end > DISP_HEIGHT) return;
    _writeCommand(ST7789_RASET);
    uint8_t _x = start;
    uint8_t _y = end;
    uint8_t result[] = { 0x00, 0x00, _x, _y };
    _writeData(result, 5);
}

void _set_window(int x0, int y0, int x1, int y1){;
    //Set window to column and row address.
    //Args:x0 (int): column start address, y0 (int): row start address
    //x1 (int): column end address, y1 (int): row end address
    _set_columns(x0, x1);
    _set_rows(y0, y1);
    _writeCommand(ST7789_RAMWR);
}

void vline(int x, int y, int length, uint16_t color){
    //Draw vertical line at the given location and color.
    //Args: x (int): x coordinate, Y (int): y coordinate 
    //length (int): length of line, color (int): 565 encoded color
    fill_rect(x, y, 1, length, color);
}

void hline(int x, int y, int length, uint16_t color){
    //Draw horizontal line at the given location and color.
    //Args:x (int): x coordinate, Y (int): y coordinate
    //length (int): length of line, color (int): 565 encoded color
    fill_rect(x, y, length, 1, color);
}

void pixel(int x, int y, uint16_t color){
    //Draw a pixel at the given location and color.
    //Args: x (int): x coordinate, Y (int): y coordinate
    //color (int): 565 encoded color

    _set_window(x, y, x, y);
    uint8_t result[] = { 0x3E, 0x48, color};
    _writeData(result, 3);
}

void blit_buffer(uint8_t buffer[], uint8_t bufferLen,int x, int y, int width, int height){
    //Copy buffer to display at the given location.
    //Args:buffer (bytes): Data to copy to display
    //x (int): Top left corner x coordinate, Y (int): Top left corner y coordinate
    //width (int): Width, height (int): Height
    _set_window(x, y, x + width - 1, y + height - 1);
    _writeData(buffer, bufferLen);
}

void rect(int x, int y, int w, int h, uint16_t color){
    //Draw a rectangle at the given location, size and color.
    //Args: x (int): Top left corner x coordinate, y (int): Top left corner y coordinate
    //width (int): Width in pixels, height (int): Height in pixels
    //color (int): 565 encoded color
    hline(x, y, w, color);
    vline(x, y, h, color);
    vline(x + w - 1, y, h, color);
    hline(x, y + h - 1, w, color);
}

void fill_rect(int x, int y, int width, int height, uint16_t color){
    //Draw a rectangle at the given location, size and filled with color.
    //Args: x (int): Top left corner x coordinate, y (int): Top left corner y coordinate
    //width (int): Width in pixels, height (int): Height in pixels
    //color (int): 565 encoded color
    _set_window(x, y, x + width - 1, y + height - 1);

    //uint16_t pixel16[1];
    //pixel16[0] = color;
    uint8_t pixel[2];
    pixel[0]=(color >> 8);
    pixel[1]=color & 0xff;

    div_t output;
    output = div(width *= height, _BUFFER_SIZE);
    int chunks = output.quot;

    gpio_put(DC_PIN, 1);    //Open to write
    uint sendbuffer = _BUFFER_SIZE * 2;
    uint8_t _drawpixel[sendbuffer];

    for (int i = 0; i < sendbuffer; i++)
    {
        _drawpixel[i] = pixel[i%2];
    }
    
    for(int i=_BUFFER_SIZE; i!=0; i--){
        _writeData(_drawpixel, sendbuffer);
    }
}

void fill(uint16_t color){
    //Fill the entire FrameBuffer with the specified color.
    //Args:color (int): 565 encoded color
    fill_rect(0, 0, DISP_WIDTH, DISP_HEIGHT, color);
}

void line(int x0, int y0, int x1, int y1, uint16_t color){
    //Draw a single pixel wide line starting at x0, y0 and ending at x1, y1.
    /*Args:
            x0 (int): Start point x coordinate
            y0 (int): Start point y coordinate
            x1 (int): End point x coordinate
            y1 (int): End point y coordinate
            color (int): 565 encoded color*/
    int a = abs(y1 - y0);
    int b = abs(x1 - x0);
    int steep = (a > b);
    if(steep){
        x0, y0 = y0, x0;
        x1, y1 = y1, x1;
    }

    if(x0 > x1){
        x0, x1 = x1, x0;
        y0, y1 = y1, y0;
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int err = dx / 2;

    int ystep;
    if(y0 < y1){
        ystep = 1;
    }else{
        ystep = -1;
    }

    while(x0 <= x1){
        if(steep){
            pixel(y0, x0, color);
        }else{
            pixel(x0, y0, color);
        }
        err -= dy;
        if(err < 0){
            y0 += ystep;
            err += dx;
        }
        x0 += 1;
    }
}

void vscrdef(int tfa, int vsa, int bfa){
    /*Set Vertical Scrolling Definition.
        To scroll a 135x240 display these values should be 40, 240, 40.
        There are 40 lines above the display that are not shown followed by
        240 lines that are shown followed by 40 more lines that are not shown.
        You could write to these areas off display and scroll them into view by
        changing the TFA, VSA and BFA values.
        Args:
            tfa (int): Top Fixed Area
            vsa (int): Vertical Scrolling Area
            bfa (int): Bottom Fixed Area*/
            //\x3e\x48\x48\x48
    uint8_t _vscrdef[] = { 0x3E, 0x48, 0x48, 0x48, tfa, vsa, bfa };
    _writeCommand(ST7789_VSCRDEF);
    _writeData(_vscrdef, 7);
}

void vscsad(int vssa){
    uint8_t _vscsad[] = { 0x3E, 0x48, vssa };
    _writeCommand(ST7789_VSCSAD);
    _writeData(_vscsad, 3);
}
////////////////////////////////////////////////////////////////////