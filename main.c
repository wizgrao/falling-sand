#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"
static PlaydateAPI* pd = NULL;
#define SCALE 2

static inline void setBit(uint8_t *buffer, int x, int y) {
	int bit_index = LCD_ROWSIZE * 8 * y + x;
	int byte_index = bit_index / 8;
	int bit_in_byte = bit_index % 8;
	uint8_t mask = 0x80 >> bit_in_byte;
	buffer[byte_index] = buffer[byte_index] | mask;
}

static inline void clearBit(uint8_t *buffer, int x, int y) {
	int bit_index = LCD_ROWSIZE * 8 * y + x;
	int byte_index = bit_index / 8;
	int bit_in_byte = bit_index % 8;
	uint8_t mask = ~(0x80 >> bit_in_byte);
	buffer[byte_index] = buffer[byte_index] & mask;
}

static inline int getBit(uint8_t *buffer, int x, int y) {
	int bit_index = LCD_ROWSIZE * 8 * y + x;
	int byte_index = bit_index / 8;
	int bit_in_byte = bit_index % 8;
	uint8_t mask = (0x80 >> bit_in_byte);
	return buffer[byte_index] & mask;
}

static inline void doFall(uint8_t *buffer, int x, int y) {
	if (!getBit(buffer, x, y)) return;
	if (!getBit(buffer, x, y+1)) {
		setBit(buffer, x, y+1);
		clearBit(buffer, x, y);
		return;
	}
	if(x < LCD_COLUMNS/SCALE -1 && !getBit(buffer, x+1, y+1)) {
		setBit(buffer, x+1, y+1);
		clearBit(buffer, x, y);
		return;
	}
	if(x > 0 && !getBit(buffer, x-1, y+1)) {
		setBit(buffer, x-1, y+1);
		clearBit(buffer, x, y);
		return;
	}
}

static void randomize(void)
{
	int x, y;
	uint8_t* frame = pd->graphics->getDisplayFrame();

	for ( y = 0; y < LCD_ROWS; ++y )
	{
		uint8_t* row = &frame[y * LCD_ROWSIZE];
		
		for ( x = 0; x < LCD_COLUMNS / 8; ++x )
			row[x] = rand();
	}
}

static int
update(void* ud)
{
	PDButtons pushed;
	pd->system->getButtonState(NULL, &pushed, NULL);
	
	if ( pushed & kButtonA )
		randomize();
	
	uint8_t* nextframe = pd->graphics->getFrame(); // working buffer
	uint8_t* frame = pd->graphics->getDisplayFrame(); // buffer currently on screen (or headed there, anyway)
	
	if ( frame != NULL )
	{

		//int randX = rand() % LCD_ROWSIZE;
		//int randY = rand() % LCD_ROWS;
		//setBit(frame, 50, 50);
		//doFall(frame, randX, randY);
		frame[0] = 0;
		for (int i =0; i < LCD_ROWSIZE * LCD_ROWS; i ++) {
			nextframe[i] = frame[i];
		}
		for (int i=0; i < 10000;i++) {
			int randX = rand() % (LCD_COLUMNS/SCALE);
			int randY = rand() % (LCD_ROWS/SCALE-1);
			doFall(nextframe, randX, randY);
		}
		//randomize();


	}
	
	// we twiddled the framebuffer bits directly, so we have to tell the system about it
	pd->graphics->markUpdatedRows(0, LCD_ROWS);
	
	return 1;
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI* playdate, PDSystemEvent event, uint32_t arg)
{
	if ( event == kEventInit )
	{
		pd = playdate;
		pd->display->setRefreshRate(0); // run as fast as possible
		pd->display->setScale(SCALE);
		pd->system->setUpdateCallback(update, NULL);
		pd->system->logToConsole("%d, %d", LCD_COLUMNS, LCD_ROWS);

		randomize();
	}
	
	return 0;
}
