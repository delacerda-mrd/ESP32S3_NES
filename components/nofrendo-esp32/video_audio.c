// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <freertos/queue.h>
//Nes stuff wants to define this as well...
#undef false
#undef true
#undef bool


#include <math.h>
#include <string.h>
#include <noftypes.h>
#include <bitmap.h>
#include <nofconfig.h>
#include <event.h>
#include <gui.h>
#include <log.h>
#include <nes.h>
#include <nes_pal.h>
#include <nesinput.h>
#include <osd.h>
#include <stdint.h>
#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "es8311.h"
#include "sdkconfig.h"
#include <spi_lcd.h>

#include <board_i2c.h>
#include <touch_controller.h>

#define  DEFAULT_SAMPLERATE   22050
#define  DEFAULT_FRAGSIZE     128

#define  DEFAULT_WIDTH        256
#define  DEFAULT_HEIGHT       NES_VISIBLE_HEIGHT

#define  GAME_X_OFFSET        48   // left D-pad bar width

#define  AMP_ENABLE_GPIO       1
#define  I2S_MCK_GPIO          4
#define  I2S_BCK_GPIO          5
#define  I2S_DIN_GPIO          6
#define  I2S_WS_GPIO           7
#define  I2S_DOUT_GPIO         8

TimerHandle_t timer;

//Seemingly, this will be called only once. Should call func with a freq of frequency,
int osd_installtimer(int frequency, void *func, int funcsize, void *counter, int countersize)
{
	printf("Timer install, freq=%d\n", frequency);
	timer=xTimerCreate("nes",configTICK_RATE_HZ/frequency, pdTRUE, NULL, func);
	xTimerStart(timer, 0);
   return 0;
}


/*
** Audio
*/
static void (*audio_callback)(void *buffer, int length) = NULL;
#if CONFIG_SOUND_ENA
QueueHandle_t queue;
static uint16_t *audio_frame;
static i2s_chan_handle_t tx_chan;
#endif

static void do_audio_frame() {

#if CONFIG_SOUND_ENA
	int left=DEFAULT_SAMPLERATE/NES_REFRESH_RATE;
	while(left) {
		int n=DEFAULT_FRAGSIZE;
		if (n>left) n=left;
		audio_callback(audio_frame, n); //get more data
		//16 bit mono -> 32-bit (16 bit r+l)
		for (int i=n-1; i>=0; i--) {
			audio_frame[i*2+1]=audio_frame[i];
			audio_frame[i*2]=audio_frame[i];
		}
		size_t written;
		i2s_channel_write(tx_chan, audio_frame, 4*n, &written, portMAX_DELAY);
		left-=n;
	}
#endif
}

void osd_setsound(void (*playfunc)(void *buffer, int length))
{
   //Indicates we should call playfunc() to get more data.
   audio_callback = playfunc;
}

static void osd_stopsound(void)
{
   audio_callback = NULL;
}


static int osd_init_sound(void)
{
#if CONFIG_SOUND_ENA
	audio_frame=malloc(4*DEFAULT_FRAGSIZE);

	i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
	i2s_new_channel(&chan_cfg, &tx_chan, NULL);

	i2s_std_config_t std_cfg = {
		.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(DEFAULT_SAMPLERATE),
		.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
		.gpio_cfg = {
			.mclk = I2S_MCK_GPIO,
			.bclk = I2S_BCK_GPIO,
			.ws = I2S_WS_GPIO,
			.dout = I2S_DOUT_GPIO,
			.din = I2S_DIN_GPIO,
		},
	};
	std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;
	i2s_channel_init_std_mode(tx_chan, &std_cfg);
	i2s_channel_enable(tx_chan);

	board_i2c_init();
	es8311_handle_t codec = es8311_create(BOARD_I2C_PORT, ES8311_ADDRESS_0);
	es8311_clock_config_t clk_cfg = {
		.mclk_inverted = false,
		.sclk_inverted = false,
		.mclk_from_mclk_pin = true,
		.mclk_frequency = DEFAULT_SAMPLERATE * 256,
		.sample_frequency = DEFAULT_SAMPLERATE,
	};
	es8311_init(codec, &clk_cfg, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16);
	es8311_voice_volume_set(codec, 75, NULL);

	// Amp enable is active-low; must not glitch low before codec init completes.
	gpio_config_t amp_conf = {
		.pin_bit_mask = 1ULL << AMP_ENABLE_GPIO,
		.mode = GPIO_MODE_OUTPUT,
	};
	gpio_config(&amp_conf);
	gpio_set_level(AMP_ENABLE_GPIO, 0);
#endif

	audio_callback = NULL;

	return 0;
}

void osd_getsoundinfo(sndinfo_t *info)
{
   info->sample_rate = DEFAULT_SAMPLERATE;
   info->bps = 16;
}

/*
** Video
*/

static int init(int width, int height);
static void shutdown(void);
static int set_mode(int width, int height);
static void set_palette(rgb_t *pal);
static void clear(uint8 color);
static bitmap_t *lock_write(void);
static void free_write(int num_dirties, rect_t *dirty_rects);
static void custom_blit(bitmap_t *bmp, int num_dirties, rect_t *dirty_rects);
static char fb[1]; //dummy

QueueHandle_t vidQueue;

viddriver_t sdlDriver =
{
   "Simple DirectMedia Layer",         /* name */
   init,          /* init */
   shutdown,      /* shutdown */
   set_mode,      /* set_mode */
   set_palette,   /* set_palette */
   clear,         /* clear */
   lock_write,    /* lock_write */
   free_write,    /* free_write */
   custom_blit,   /* custom_blit */
   false          /* invalidate flag */
};


bitmap_t *myBitmap;

void osd_getvideoinfo(vidinfo_t *info)
{
   info->default_width = DEFAULT_WIDTH;
   info->default_height = DEFAULT_HEIGHT;
   info->driver = &sdlDriver;
}

/* flip between full screen and windowed */
void osd_togglefullscreen(int code)
{
}

/* initialise video */
static int init(int width, int height)
{
	return 0;
}

static void shutdown(void)
{
}

/* set a video mode */
static int set_mode(int width, int height)
{
   return 0;
}

uint16 myPalette[256];

/* copy nes palette over to hardware (pre-byte-swapped for SPI MSB-first transmission) */
static void set_palette(rgb_t *pal)
{
	uint16 c;

   int i;

   for (i = 0; i < 256; i++)
   {
      c=(pal[i].b>>3)+((pal[i].g>>2)<<5)+((pal[i].r>>3)<<11);
      myPalette[i]=(c>>8)|((c&0xff)<<8);
   }

}

/* clear all frames to a particular color */
static void clear(uint8 color)
{
//   SDL_FillRect(mySurface, 0, color);
}



/* acquire the directbuffer for writing */
static bitmap_t *lock_write(void)
{
//   SDL_LockSurface(mySurface);
   myBitmap = bmp_createhw((uint8*)fb, DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_WIDTH*2);
   return myBitmap;
}

/* release the resource */
static void free_write(int num_dirties, rect_t *dirty_rects)
{
   bmp_destroy(&myBitmap);
}


static void custom_blit(bitmap_t *bmp, int num_dirties, rect_t *dirty_rects) {
	xQueueSend(vidQueue, &bmp, 0);
	do_audio_frame();
}


//This runs on core 1.
static void videoTask(void *arg) {
	int x, y;
	bitmap_t *bmp=NULL;
	x = GAME_X_OFFSET;
    y = ((240-DEFAULT_HEIGHT)/2);
    while(1) {
		xQueueReceive(vidQueue, &bmp, portMAX_DELAY);
		ili9341_write_frame(x, y, DEFAULT_WIDTH, DEFAULT_HEIGHT, (const uint8_t **)bmp->line);
	}
}

/*
** Control overlay: D-pad on the left 48px bar, A/B/Start/Select on the right 16px bar.
** These zones match the touch_controller.c hit-test table exactly. Since per-frame
** blits only touch columns [GAME_X_OFFSET, GAME_X_OFFSET+DEFAULT_WIDTH), the bars
** are drawn once here and persist without needing to be redrawn.
*/
#define OVERLAY_DIM_GRAY  0x39E7
#define OVERLAY_WHITE     0xFFFF
#define OVERLAY_BLACK     0x0000

static void draw_control_overlay(void)
{
	// Left bar: 4 D-pad zones, 48x60 each.
	const struct { uint16_t y; char glyph; } left_zones[4] = {
		{ 0,   GLYPH_ARROW_UP },
		{ 60,  GLYPH_ARROW_LEFT },
		{ 120, GLYPH_ARROW_RIGHT },
		{ 180, GLYPH_ARROW_DOWN },
	};
	for (int i = 0; i < 4; i++) {
		uint16_t zy = left_zones[i].y;
		ili9341_fill_rect(2, zy + 2, 44, 56, OVERLAY_DIM_GRAY);
		ili9341_draw_char(24 - 8, zy + 30 - 8, left_zones[i].glyph, OVERLAY_WHITE, OVERLAY_DIM_GRAY, 2);
	}

	// Right bar: 4 button zones, 16 wide, single-character labels.
	const struct { uint16_t y; char glyph; } right_zones[4] = {
		{ 0,   'A' },
		{ 60,  'B' },
		{ 120, 'S' },
		{ 180, 'E' },
	};
	for (int i = 0; i < 4; i++) {
		uint16_t zy = right_zones[i].y;
		ili9341_fill_rect(304, zy, 16, 1, OVERLAY_WHITE); // separator line
		ili9341_draw_char(304 + 4, zy + 26, right_zones[i].glyph, OVERLAY_WHITE, OVERLAY_BLACK, 1);
	}
}


/*
** Input
*/

static void osd_initinput()
{
	touchInit();
}

void osd_getinput(void)
{
	const int ev[16]={
			event_joypad1_select,0,0,event_joypad1_start,event_joypad1_up,event_joypad1_right,event_joypad1_down,event_joypad1_left,
			0,0,0,0,event_soft_reset,event_joypad1_a,event_joypad1_b,event_hard_reset
		};
	static int oldb=0xffff;
	int b=touchReadInput();
	int chg=b^oldb;
	int x;
	oldb=b;
	event_t evh;
//	printf("Input: %x\n", b);
	for (x=0; x<16; x++) {
		if (chg&1) {
			evh=event_get(ev[x]);
			if (evh) evh((b&1)?INP_STATE_BREAK:INP_STATE_MAKE);
		}
		chg>>=1;
		b>>=1;
	}
}

static void osd_freeinput(void)
{
}

void osd_getmouse(int *x, int *y, int *button)
{
}

/*
** Shutdown
*/

/* this is at the bottom, to eliminate warnings */
void osd_shutdown()
{
	osd_stopsound();
	osd_freeinput();
}

static int logprint(const char *string)
{
   return printf("%s", string);
}

/*
** Startup
*/

int osd_init()
{
	log_chain_logfunc(logprint);

	if (osd_init_sound())
		return -1;

	ili9341_init();
	ili9341_write_frame(0,0,320,240,NULL);
	draw_control_overlay();
	vidQueue=xQueueCreate(1, sizeof(bitmap_t *));
	xTaskCreatePinnedToCore(&videoTask, "videoTask", 4096, NULL, 5, NULL, 1);
	osd_initinput();
	return 0;
}
