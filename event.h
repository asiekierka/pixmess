#ifndef _EVENT_H_
#define _EVENT_H_

#include "types.h"

enum
{
	SFP_KEY_NONE = 0,
	
	// keys which have ascii codes
	SFP_KEY_BACKSPACE = 8,
	SFP_KEY_TAB = 9,
	SFP_KEY_ENTER = 10,
	SFP_KEY_SPACE = 32,
	SFP_KEY_DELETE = 127,
	
	SFP_KEY_QUOTE = 39,
	SFP_KEY_COMMA = 44,
	SFP_KEY_MINUS = 45,
	SFP_KEY_DOT = 46,
	SFP_KEY_SLASH = 47,
	
	SFP_KEY_0 = 48,
	SFP_KEY_1,
	SFP_KEY_2,
	SFP_KEY_3,
	SFP_KEY_4,
	SFP_KEY_5,
	SFP_KEY_6,
	SFP_KEY_7,
	SFP_KEY_8,
	SFP_KEY_9,
	
	SFP_KEY_SEMICOLON = 59,
	SFP_KEY_EQUALS = 61,
	
	SFP_KEY_A = 65,
	SFP_KEY_B,
	SFP_KEY_C,
	SFP_KEY_D,
	SFP_KEY_E,
	SFP_KEY_F,
	SFP_KEY_G,
	SFP_KEY_H,
	SFP_KEY_I,
	SFP_KEY_J,
	SFP_KEY_K,
	SFP_KEY_L,
	SFP_KEY_M,
	SFP_KEY_N,
	SFP_KEY_O,
	SFP_KEY_P,
	SFP_KEY_Q,
	SFP_KEY_R,
	SFP_KEY_S,
	SFP_KEY_T,
	SFP_KEY_U,
	SFP_KEY_V,
	SFP_KEY_W,
	SFP_KEY_X,
	SFP_KEY_Y,
	SFP_KEY_Z,
	
	// using "square" to denote "square bracket"
	// in real countries we call ( and ) "brackets"
	// --GM
	SFP_KEY_LSQUARE = 91,
	SFP_KEY_BACKSLASH = 92,
	SFP_KEY_RSQUARE = 93,
	SFP_KEY_BACKTICK = 96,
	
	// keys w/o ascii codes which aren't modifiers
	SFP_KEY_UP = 256,
	SFP_KEY_LEFT,
	SFP_KEY_RIGHT,
	SFP_KEY_DOWN,
	SFP_KEY_ESC,
	SFP_KEY_F1,
	SFP_KEY_F2,
	SFP_KEY_F3,
	SFP_KEY_F4,
	SFP_KEY_F5,
	SFP_KEY_F6,
	SFP_KEY_F7,
	SFP_KEY_F8,
	SFP_KEY_F9,
	SFP_KEY_F10,
	SFP_KEY_F11,
	SFP_KEY_F12,
	SFP_KEY_INSERT,
	SFP_KEY_CAPSLOCK, // DON'T. EVER.
	SFP_KEY_NUMLOCK, // DON'T USE THIS EITHER.
	SFP_KEY_SCRLOCK, // NOT EVEN THIS.
	SFP_KEY_HOME,
	SFP_KEY_END,
	SFP_KEY_PGUP,
	SFP_KEY_PGDN,
	SFP_KEY_PRTSC,
	
	// note, this key is stupid.
	// it sends the same sequence on press as it does on release, IIRC.
	SFP_KEY_PAUSE,
	
	// these are pseudokeys.
	SFP_KEY_APP_QUIT,

	// keys which are duplicates of keys with ascii codes
	// OR numpad keys which have ASCII codes
	SFP_KEY_NUM_ENTER = 10|512,
	SFP_KEY_NUM_ASTERISK = 42|512,
	SFP_KEY_NUM_PLUS = 43|512,
	SFP_KEY_NUM_MINUS = 45|512,
	SFP_KEY_NUM_DOT = 46|512,
	SFP_KEY_NUM_SLASH = 47|512,
	
	SFP_KEY_NUM_0 = 48|512,
	SFP_KEY_NUM_1,
	SFP_KEY_NUM_2,
	SFP_KEY_NUM_3,
	SFP_KEY_NUM_4,
	SFP_KEY_NUM_5,
	SFP_KEY_NUM_6,
	SFP_KEY_NUM_7,
	SFP_KEY_NUM_8,
	SFP_KEY_NUM_9,
	
	// keys which at least kinda are modifiers
	SFP_KEY_LSHIFT = 768,
	SFP_KEY_RSHIFT,
	SFP_KEY_LCTRL,
	SFP_KEY_RCTRL,
	SFP_KEY_LALT,
	SFP_KEY_RALT,
	SFP_KEY_LSUPER,
	SFP_KEY_RSUPER,
	SFP_KEY_MENU,
};

int sfp_event_key(int key);
int sfp_event_getkey();
void sfp_event_poll();

#endif /* _EVENT_H_ */

