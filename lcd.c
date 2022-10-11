
#include "pico/stdlib.h"

/* INCLUDES *****************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "EVE.h"
#include "eve_ui.h"
#include "eve_ram_g.h"

#include "assets.h"


#if (DISPLAY_RES == WQVGA)
#define ZOOM 1
#elif (DISPLAY_RES == WVGA)
#define ZOOM 1
#endif 

/* EXTERNALS ****************************************************************/

extern const uint8_t img_startup_jpg[];
extern const uint8_t img_background_jpg[];
extern const Image_header_t img_gbox_format[];
extern const uint8_t img_gbox[];

/* CONSTANTS ****************************************************************/

const int32_t colours[24] =	{
		EVE_ENC_COLOR_RGB(255, 63, 0),
		EVE_ENC_COLOR_RGB(255, 127, 0), //  1
		EVE_ENC_COLOR_RGB(255, 191, 0),
		EVE_ENC_COLOR_RGB(255, 255, 0), //  2
		EVE_ENC_COLOR_RGB(191, 255, 0),
		EVE_ENC_COLOR_RGB(127, 255, 0), //  3
		EVE_ENC_COLOR_RGB(63, 255, 0),
		EVE_ENC_COLOR_RGB(0, 255, 0), //  4
		EVE_ENC_COLOR_RGB(0, 255, 63),
		EVE_ENC_COLOR_RGB(0, 255, 127), //  5
		EVE_ENC_COLOR_RGB(0, 255, 191),
		EVE_ENC_COLOR_RGB(0, 255, 255), //  6
		EVE_ENC_COLOR_RGB(0, 191, 255),
		EVE_ENC_COLOR_RGB(0, 127, 255), //  7
		EVE_ENC_COLOR_RGB(0, 63, 255),
		EVE_ENC_COLOR_RGB(0, 0, 255), //  8
		EVE_ENC_COLOR_RGB(63, 0, 255),
		EVE_ENC_COLOR_RGB(127, 0, 255), //  9
		EVE_ENC_COLOR_RGB(191, 0, 255),
		EVE_ENC_COLOR_RGB(255, 0, 255), // 10
		EVE_ENC_COLOR_RGB(255, 0, 191),
		EVE_ENC_COLOR_RGB(255, 0, 127), // 11
		EVE_ENC_COLOR_RGB(255, 0, 63),
		EVE_ENC_COLOR_RGB(255, 0, 0) // 12
};

// Do not send any debug information to the stdio. The stdio channel is connected
// to the host via the USB CDC_ACM interface and is used as a channel for game data.
#undef DEBUG

// Allow the use of the pico LED for feedback.
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

// Bitmap handles for EVE.
#define HANDLE_GUI_1_STARTUP  (0)
#define HANDLE_GUI_1_BACKGROUND  (1)
#define FIXED_FONT 31

#define BUTTON_START 0x08
#define REFRESH_GENERAL 0x17

#define GAME_AREA_WIDTH 224
#define GAME_AREA_HEIGHT 256

// Offsets to centre on display ...
#define DISPLAY_XOFF ((EVE_DISP_WIDTH - (GAME_AREA_WIDTH * ZOOM)) / 2)
#define DISPLAY_YOFF ((EVE_DISP_HEIGHT - (GAME_AREA_HEIGHT * ZOOM)) / 2)

// Bottom of game area.
#define GAME_AREA_BOTTOM (GAME_AREA_HEIGHT - 18)

volatile uint16_t updateTimer[1];

/* LOCAL FUNCTIONS / INLINES ************************************************/

// Forward declaration of board setup.
static void setup(void);
static void led_state(uint8_t state);

static void Draw_Bitmap(uint8_t handle, int16_t x, int16_t y);
static void renderStartButton(void);

/* EVE FUNCTIONS ************************************************************/
static void wait_ms(uint16_t wait)
{
	updateTimer[REFRESH_GENERAL] = wait;
	while (!updateTimer[REFRESH_GENERAL]) {};
}

static void wait_for_button(uint8_t button)
{
	do
	{
		wait_ms(200);

			led_state(1);
			
		uint8_t tagTouch = HAL_MemRead8(EVE_REG_TOUCH_TAG);

		if (tagTouch == button)
		{
			led_state(1);

			break;
		}

	led_state(0);

	} while (1);

}


static void display_list_start()
{
	// Start of new display list ...
	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
}

static void display_list_end()
{
	// end of display list ...
	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();
}

static void wait_for_start()
{
/* 	// load the opening screen
	eve_ui_load_jpg(img_startup_jpg, HANDLE_GUI_1_STARTUP, NULL, NULL);	

	// get eve in command mode
	display_list_start();
		
	Draw_Bitmap(HANDLE_GUI_1_STARTUP, 0, 0);

	display_list_end();

	 */
	// renderStartButton();
	// wait until 'START' button is pressed and released ...
	// wait_for_button(BUTTON_START);
}


// Draw a bitmap at the required location.
static void Draw_Bitmap(uint8_t handle, int16_t x, int16_t y)
{
	EVE_BITMAP_HANDLE(handle);
	EVE_BEGIN(EVE_BEGIN_BITMAPS);
	EVE_VERTEX_FORMAT(3);
	EVE_VERTEX2F(8 * x, 8 * y);
	EVE_VERTEX_FORMAT(4);
	EVE_END();
}   

static inline void bitmap_draw(uint16_t xcrd, uint16_t ycrd, char hndl, char cell)
{

	EVE_BITMAP_HANDLE(hndl);
	EVE_BEGIN(EVE_BEGIN_BITMAPS);
	EVE_BITMAP_SIZE(EVE_FILTER_NEAREST, EVE_WRAP_BORDER, EVE_WRAP_BORDER, 125, 250);
	EVE_COLOR_A(128);
	EVE_COLOR_RGB(255, 255, 255);

	#if !defined (EVE1_ENABLE)
		EVE_VERTEX2II((xcrd * ZOOM), (ycrd * ZOOM), FIXED_FONT, cell);
	#else // !defined (EVE1_ENABLE)
		EVE_VERTEX2II(DISPLAY_XOFF + (xcrd * ZOOM), DISPLAY_YOFF + (ycrd * ZOOM), hndl, cell);
	#endif // !defined (EVE1_ENABLE)
	EVE_END();


}

static void drawMainTemp()
{
	// display the currentTemp
	const char *currentTemp = "450";	
	int xcrd = 80;
	int ycrd = 100;

	do
	{
		bitmap_draw(xcrd, ycrd, FIXED_FONT, *currentTemp);
		xcrd += 21;

	} while (*++currentTemp != 0);

	xcrd = 80;
	ycrd = 80;
}

static void renderStartButton()
{
/* /* #if defined (EVE2_ENABLE) || defined (EVE3_ENABLE) || defined (EVE4_ENABLE)
	EVE_VERTEX_TRANSLATE_X(0 << 4);
	EVE_VERTEX_TRANSLATE_Y(0 << 4);
#else
	EVE_CMD_TRANSLATE(DISPLAY_XOFF << 16, DISPLAY_YOFF << 16);
#endif // defined (EVE2_ENABLE) || defined (EVE3_ENABLE) || defined (EVE4_ENABLE)

	// Add invisible control buttons to display list ...
	EVE_COLOR_MASK(0, 0, 0, 0);

	EVE_CMD_FGCOLOR(0x880000);
	EVE_CMD_GRADCOLOR(0xff0000);
	EVE_BEGIN(EVE_BEGIN_RECTS);
	EVE_TAG(BUTTON_START);
	EVE_VERTEX2F(0, 0);
	EVE_VERTEX2F(100, 100);

	EVE_COLOR_MASK(255, 255, 255, 255);


	EVE_END(); */


}

static void Draw_GUI_1()
{
	
	// Draw the dashboard background.
	Draw_Bitmap(HANDLE_GUI_1_BACKGROUND, 0, 0);

	// draw a temp 
	drawMainTemp();
	
}



static void cluster_draw(void)
{
	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CLEAR_COLOR_RGB(125, 125, 100);
	EVE_CLEAR(1, 1, 1);
	EVE_COLOR_RGB(255, 255, 255);

	Draw_GUI_1();

	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();
}

static void cluster_setup(void)
{
		
	eve_ui_load_jpg(img_background_jpg, HANDLE_GUI_1_BACKGROUND, NULL, NULL);		

    EVE_LIB_BeginCoProList();
    EVE_CMD_DLSTART();
    EVE_DISPLAY();
    EVE_CMD_SWAP();
    EVE_LIB_EndCoProList();
    EVE_LIB_AwaitCoProEmpty();

	
}

static void cluster_loop(void)
{	
	// Draw the dashboard.
	cluster_draw();
}

/* TINYUSB FUNCTIONS ********************************************************/

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
}

/* FUNCTIONS ****************************************************************/

int main(void)
{
    uint8_t report_id;

    /* Setup UART */
    setup();
	eve_ui_setup();
	eve_ui_splash("Starting REC grill controller");

	sleep_ms(2000);
	wait_for_start();

	EVE_LIB_BeginCoProList();
	EVE_CMD_DLSTART();
	EVE_CLEAR_COLOR_RGB(0, 0, 0);
	EVE_CLEAR(1,1,1);
	EVE_COLOR_RGB(255, 255, 255);
	EVE_DISPLAY();
	EVE_CMD_SWAP();
	EVE_LIB_EndCoProList();
	EVE_LIB_AwaitCoProEmpty();


	cluster_setup();
    while (1)
    {			
		cluster_loop();
    }

    // function never returns
    for (;;) ;
}

static void led_state(uint8_t state)
{
    gpio_put(LED_PIN, state);
}

static void setup(void)
{

    // Initialise stdio ports as configured in CMakeLists.txt
    stdio_init_all();

    // Turn on the pico LED to show activity
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_put(LED_PIN, 0);
   
}
