#include <SDL.h>
#include "event.h"
/*
WARNING:
To simplify matters, the keymap used here is based on my (GM's) SDL_keysym.h
file as it currently stands. While I do believe they won't wreck screw with the
key numbers, there IS a chance that they will. If they do this, you WILL need
to redo the keymap. Cheers. --GM

P.S. If WE screw with our symbols, it'll need to be redone, too.
     But that's OUR job. --GM
*/

// template:
/*
uint16_t keymap[322] = {
	SDLK_FIRST,0,0,0,0,0,0,0,
	SDLK_BACKSPACE,SDLK_TAB,0,0,
	SDLK_CLEAR,SDLK_RETURN,0,0,0,0,0,
	SDLK_PAUSE,0,0,0,0,0,0,0,
	SDLK_ESCAPE,0,0,0,0,
	SDLK_SPACE,SDLK_EXCLAIM,SDLK_QUOTEDBL,SDLK_HASH,SDLK_DOLLAR,0,
	SDLK_AMPERSAND,SDLK_QUOTE,SDLK_LEFTPAREN,SDLK_RIGHTPAREN,SDLK_ASTERISK,
	SDLK_PLUS,SDLK_COMMA,SDLK_MINUS,SDLK_PERIOD,SDLK_SLASH,
	SDLK_0,SDLK_1,SDLK_2,SDLK_3,SDLK_4,SDLK_5,SDLK_6,SDLK_7,SDLK_8,SDLK_9,
	SDLK_COLON,SDLK_SEMICOLON,SDLK_LESS,SDLK_EQUALS,SDLK_GREATER,
	SDLK_QUESTION,SDLK_AT,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	SDLK_LEFTBRACKET,SDLK_BACKSLASH,SDLK_RIGHTBRACKET,SDLK_CARET,
	SDLK_UNDERSCORE,SDLK_BACKQUOTE,
	SDLK_a,SDLK_b,SDLK_c,SDLK_d,SDLK_e,SDLK_f,SDLK_g,SDLK_h,SDLK_i,SDLK_j,
	SDLK_k,SDLK_l,SDLK_m,SDLK_n,SDLK_o,SDLK_p,SDLK_q,SDLK_r,SDLK_s,SDLK_t,
	SDLK_u,SDLK_v,SDLK_w,SDLK_x,SDLK_y,SDLK_z,0,0,0,0,
	SDLK_DELETE,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	SDLK_WORLD_1,SDLK_WORLD_2,SDLK_WORLD_3,SDLK_WORLD_4,SDLK_WORLD_5,
	SDLK_WORLD_6,SDLK_WORLD_7,SDLK_WORLD_8,SDLK_WORLD_9,SDLK_WORLD_10,
	SDLK_WORLD_11,SDLK_WORLD_12,SDLK_WORLD_13,SDLK_WORLD_14,SDLK_WORLD_15,
	SDLK_WORLD_16,SDLK_WORLD_17,SDLK_WORLD_18,SDLK_WORLD_19,SDLK_WORLD_20,
	SDLK_WORLD_21,SDLK_WORLD_22,SDLK_WORLD_23,SDLK_WORLD_24,SDLK_WORLD_25,
	SDLK_WORLD_26,SDLK_WORLD_27,SDLK_WORLD_28,SDLK_WORLD_29,SDLK_WORLD_30,
	SDLK_WORLD_31,SDLK_WORLD_32,SDLK_WORLD_33,SDLK_WORLD_34,SDLK_WORLD_35,
	SDLK_WORLD_36,SDLK_WORLD_37,SDLK_WORLD_38,SDLK_WORLD_39,SDLK_WORLD_40,
	SDLK_WORLD_41,SDLK_WORLD_42,SDLK_WORLD_43,SDLK_WORLD_44,SDLK_WORLD_45,
	SDLK_WORLD_46,SDLK_WORLD_47,SDLK_WORLD_48,SDLK_WORLD_49,SDLK_WORLD_50,
	SDLK_WORLD_51,SDLK_WORLD_52,SDLK_WORLD_53,SDLK_WORLD_54,SDLK_WORLD_55,
	SDLK_WORLD_56,SDLK_WORLD_57,SDLK_WORLD_58,SDLK_WORLD_59,SDLK_WORLD_60,
	SDLK_WORLD_61,SDLK_WORLD_62,SDLK_WORLD_63,SDLK_WORLD_64,SDLK_WORLD_65,
	SDLK_WORLD_66,SDLK_WORLD_67,SDLK_WORLD_68,SDLK_WORLD_69,SDLK_WORLD_70,
	SDLK_WORLD_71,SDLK_WORLD_72,SDLK_WORLD_73,SDLK_WORLD_74,SDLK_WORLD_75,
	SDLK_WORLD_76,SDLK_WORLD_77,SDLK_WORLD_78,SDLK_WORLD_79,SDLK_WORLD_80,
	SDLK_WORLD_81,SDLK_WORLD_82,SDLK_WORLD_83,SDLK_WORLD_84,SDLK_WORLD_85,
	SDLK_WORLD_86,SDLK_WORLD_87,SDLK_WORLD_88,SDLK_WORLD_89,SDLK_WORLD_90,
	SDLK_WORLD_91,SDLK_WORLD_92,SDLK_WORLD_93,SDLK_WORLD_94,0,
	SDLK_KP0,SDLK_KP1,SDLK_KP2,SDLK_KP3,SDLK_KP4,
	SDLK_KP5,SDLK_KP6,SDLK_KP7,SDLK_KP8,SDLK_KP9,
	SDLK_KP_PERIOD,SDLK_KP_DIVIDE,SDLK_KP_MULTIPLY,SDLK_KP_MINUS,
	SDLK_KP_PLUS,SDLK_KP_ENTER,SDLK_KP_EQUALS,
	SDLK_UP,SDLK_DOWN,SDLK_RIGHT,SDLK_LEFT,SDLK_INSERT,SDLK_HOME,SDLK_END,
	SDLK_PAGEUP,SDLK_PAGEDOWN,
	SDLK_F1,SDLK_F2,SDLK_F3,SDLK_F4,SDLK_F5,SDLK_F6,
	SDLK_F7,SDLK_F8,SDLK_F9,SDLK_F10,SDLK_F11,SDLK_F12,
	SDLK_F13,SDLK_F14,SDLK_F15,0,0,0,
	SDLK_NUMLOCK,SDLK_CAPSLOCK,SDLK_SCROLLOCK,
	SDLK_RSHIFT,SDLK_LSHIFT,SDLK_RCTRL,SDLK_LCTRL,SDLK_RALT,SDLK_LALT,
	SDLK_RMETA,SDLK_LMETA,0,0,0,0,
	SDLK_HELP,SDLK_PRINT,SDLK_SYSREQ,SDLK_BREAK,SDLK_MENU,0,0
};
*/

#define KEY_COUNT_SDL 322
u16 keymap_sdl[KEY_COUNT_SDL] = {
	0,0,0,0,0,0,0,0,
	SFP_KEY_BACKSPACE,SFP_KEY_TAB,0,0,
	0,SFP_KEY_ENTER,0,0,0,0,0,
	SFP_KEY_PAUSE,0,0,0,0,0,0,0,
	SFP_KEY_ESC,0,0,0,0,
	SFP_KEY_SPACE,SFP_KEY_1,SFP_KEY_QUOTE,SFP_KEY_3,SFP_KEY_4,0,
	SFP_KEY_7,SFP_KEY_QUOTE,SFP_KEY_9,SFP_KEY_0,SFP_KEY_8,
	SFP_KEY_EQUALS,SFP_KEY_COMMA,SFP_KEY_MINUS,SFP_KEY_DOT,SFP_KEY_SLASH,
	SFP_KEY_0,SFP_KEY_1,SFP_KEY_2,SFP_KEY_3,SFP_KEY_4,
	SFP_KEY_5,SFP_KEY_6,SFP_KEY_7,SFP_KEY_8,SFP_KEY_9,
	SFP_KEY_SEMICOLON,SFP_KEY_SEMICOLON,SFP_KEY_COMMA,SFP_KEY_EQUALS,
	SFP_KEY_DOT,SFP_KEY_SLASH,SFP_KEY_2,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	SFP_KEY_LSQUARE,SFP_KEY_BACKSLASH,SFP_KEY_RSQUARE,SFP_KEY_6,
	SFP_KEY_MINUS,SFP_KEY_BACKTICK,
	SFP_KEY_A,SFP_KEY_B,SFP_KEY_C,SFP_KEY_D,SFP_KEY_E,SFP_KEY_F,SFP_KEY_G,
	SFP_KEY_H,SFP_KEY_I,SFP_KEY_J,SFP_KEY_K,SFP_KEY_L,SFP_KEY_M,SFP_KEY_N,
	SFP_KEY_O,SFP_KEY_P,SFP_KEY_Q,SFP_KEY_R,SFP_KEY_S,SFP_KEY_T,SFP_KEY_U,
	SFP_KEY_V,SFP_KEY_W,SFP_KEY_X,SFP_KEY_Y,SFP_KEY_Z,0,0,0,0,
	SFP_KEY_DELETE,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,
	SFP_KEY_NUM_0,SFP_KEY_NUM_1,SFP_KEY_NUM_2,SFP_KEY_NUM_3,SFP_KEY_NUM_4,
	SFP_KEY_NUM_5,SFP_KEY_NUM_6,SFP_KEY_NUM_7,SFP_KEY_NUM_8,SFP_KEY_NUM_9,
	SFP_KEY_NUM_DOT,SFP_KEY_NUM_SLASH,SFP_KEY_NUM_ASTERISK,
	SFP_KEY_NUM_MINUS,SFP_KEY_NUM_PLUS,SFP_KEY_NUM_ENTER,0,
	SFP_KEY_UP,SFP_KEY_DOWN,SFP_KEY_RIGHT,SFP_KEY_LEFT,
	SFP_KEY_INSERT,SFP_KEY_HOME,SFP_KEY_END,SFP_KEY_PGUP,
	SFP_KEY_PGDN,
	SFP_KEY_F1,SFP_KEY_F2,SFP_KEY_F3,SFP_KEY_F4,SFP_KEY_F5,SFP_KEY_F6,
	SFP_KEY_F7,SFP_KEY_F8,SFP_KEY_F9,SFP_KEY_F10,SFP_KEY_F11,SFP_KEY_F12,
	0,0,0,0,0,0,
	SFP_KEY_NUMLOCK,SFP_KEY_CAPSLOCK,SFP_KEY_SCRLOCK,
	SFP_KEY_RSHIFT,SFP_KEY_LSHIFT,SFP_KEY_RCTRL,SFP_KEY_LCTRL,
	SFP_KEY_RALT,SFP_KEY_LALT,
	0,0,0,0,0,0,
	0,0,SFP_KEY_PRTSC,SDLK_PAUSE,SDLK_MENU,0,0
};

u16 keyqueue[256];
int keyqueue_start = 0;
int keyqueue_end = 0;

u8 keysel[1024];
u8 waiting_time[1024];
int event_initialised = 0;
int mouse_pos_x = -1, mouse_pos_y = -1;
u32 mouse_buttons = 0;
u32 mouse_wheels = 0;

void sfp_event_init()
{
	if(event_initialised)
		return;
	
	memset(keysel, 0, 1024);
	memset(waiting_time, 0, 1024);
	
	event_initialised = 255;
}

int sfp_event_key(int key)
{
	sfp_event_init();
	
	return keysel[key];
}

void sfp_event_keywait(int key, int time)
{
	sfp_event_init();
	waiting_time[key]=time;
}

int sfp_event_getkeywait(int key)
{
	sfp_event_init();
	return waiting_time[key];
}

void sfp_event_tick()
{
	u16 i;
	for(i=0;i<1024;i++)
		if(waiting_time[i]>0) waiting_time[i]--;
}

int sfp_event_getkey()
{
	sfp_event_init();
	
	if(keyqueue_start == keyqueue_end)
		return 0;
	
	int ret = keyqueue[keyqueue_start];
	keyqueue_start = (keyqueue_start+1)&255;
	
	return ret;
}

int sfp_event_mouse_x()
{
	return mouse_pos_x;
}

int sfp_event_mouse_y()
{
	return mouse_pos_y;
}

// SUPPORTS UP TO 32 BUTTONS WOW THAT'S 1/3 OF THE WAY TOWARDS A KEYBOARD
int sfp_event_mouse_button(int btn)
{
	return (mouse_buttons>>btn)&1;
}
int sfp_event_mouse_button_press(int btn)
{
	if((mouse_wheels>>btn)&1)
	{
		mouse_wheels &= ~(1<<btn);
		return 1;
	}
	return 0;
}

void sfp_event_poll()
{
	sfp_event_init();
	
	SDL_Event sdlev;
	while(SDL_PollEvent(&sdlev))
	{
		switch(sdlev.type)
		{
			case SDL_QUIT:
				keysel[SFP_KEY_APP_QUIT] = 1;
				break;
			case SDL_KEYDOWN:
				keysel[keymap_sdl[sdlev.key.keysym.sym]] = 1;
				keyqueue[keyqueue_end] = keymap_sdl[sdlev.key.keysym.sym];
				keyqueue_end = (keyqueue_end+1)&255;
				
				// if the ring buffer overflows,
				// knock off the oldest key code
				if(keyqueue_end == keyqueue_start)
					keyqueue_start = (keyqueue_start+1)&255;
				break;
			case SDL_KEYUP:
				keysel[keymap_sdl[sdlev.key.keysym.sym]] = 0;
				break;
			case SDL_MOUSEMOTION:
				mouse_pos_x = sdlev.motion.x;
				mouse_pos_y = sdlev.motion.y;
				break;
			case SDL_MOUSEBUTTONDOWN:
				mouse_pos_x = sdlev.button.x;
				mouse_pos_y = sdlev.button.y;
				mouse_buttons |= (1<<(sdlev.button.button-1));
				mouse_wheels |= (1<<(sdlev.button.button-1));
				break;
			case SDL_MOUSEBUTTONUP:
				mouse_pos_x = sdlev.button.x;
				mouse_pos_y = sdlev.button.y;
				mouse_buttons &= ~(1<<(sdlev.button.button-1));
				break;
		}
	}
}

