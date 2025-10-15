#pragma once
#include "core/input/input_enums.h"
#include "core/os/keyboard.h"
#include <SDL2/SDL.h>

#define GODOT_KEY_PHYSICAL_MASK 0x800000
// Return the logical Godot Key, or Key::NONE if no mapping exists
static inline Key sdl2godot_keycode(SDL_Keycode key) {
	// Special keys and symbols
	switch (key) {
		case SDLK_RETURN:
			return Key::ENTER;
		case SDLK_ESCAPE:
			return Key::ESCAPE;
		case SDLK_BACKSPACE:
			return Key::BACKSPACE;
		case SDLK_TAB:
			return Key::TAB;
		case SDLK_SPACE:
			return Key::SPACE;
		case SDLK_MINUS:
			return Key::MINUS;
		case SDLK_EQUALS:
			return Key::EQUAL;
		case SDLK_LEFTBRACKET:
			return Key::BRACKETLEFT;
		case SDLK_RIGHTBRACKET:
			return Key::BRACKETRIGHT;
		case SDLK_BACKSLASH:
			return Key::BACKSLASH;
		case SDLK_SEMICOLON:
			return Key::SEMICOLON;
		// case SDLK_APOSTROPHE: return Key::APOSTROPHE;
		// case SDLK_GRAVE: return Key::QUOTELEFT;
		case SDLK_COMMA:
			return Key::COMMA;
		case SDLK_PERIOD:
			return Key::PERIOD;
		case SDLK_SLASH:
			return Key::SLASH;
		case SDLK_CAPSLOCK:
			return Key::CAPSLOCK;
		case SDLK_PRINTSCREEN:
			return Key::PRINT;
		case SDLK_SCROLLLOCK:
			return Key::SCROLLLOCK;
		case SDLK_PAUSE:
			return Key::PAUSE;
		case SDLK_INSERT:
			return Key::INSERT;
		case SDLK_HOME:
			return Key::HOME;
		case SDLK_PAGEUP:
			return Key::PAGEUP;
		case SDLK_DELETE:
			return Key::KEY_DELETE;
		case SDLK_END:
			return Key::END;
		case SDLK_PAGEDOWN:
			return Key::PAGEDOWN;
		case SDLK_RIGHT:
			return Key::RIGHT;
		case SDLK_LEFT:
			return Key::LEFT;
		case SDLK_DOWN:
			return Key::DOWN;
		case SDLK_UP:
			return Key::UP;
		case SDLK_NUMLOCKCLEAR:
			return Key::NUMLOCK;
		case SDLK_KP_DIVIDE:
			return Key::KP_DIVIDE;
		case SDLK_KP_MULTIPLY:
			return Key::KP_MULTIPLY;
		case SDLK_KP_MINUS:
			return Key::KP_SUBTRACT;
		case SDLK_KP_PLUS:
			return Key::KP_ADD;
		case SDLK_KP_ENTER:
			return Key::KP_ENTER;
		case SDLK_KP_PERIOD:
			return Key::KP_PERIOD;
		case SDLK_APPLICATION:
			return Key::MENU;
		case SDLK_KP_EQUALS:
			return Key::EQUAL;
		case SDLK_HELP:
			return Key::HELP;
		case SDLK_MENU:
			return Key::MENU;
		case SDLK_MUTE:
			return Key::VOLUMEMUTE;
		case SDLK_VOLUMEUP:
			return Key::VOLUMEUP;
		case SDLK_VOLUMEDOWN:
			return Key::VOLUMEDOWN;
		case SDLK_KP_COMMA:
			return Key::COMMA;
		case SDLK_KP_EQUALSAS400:
			return Key::EQUAL;
		case SDLK_SYSREQ:
			return Key::SYSREQ;
		case SDLK_CLEAR:
			return Key::CLEAR;
		case SDLK_PRIOR:
			return Key::PAGEUP;
		case SDLK_RETURN2:
			return Key::ENTER;
		case SDLK_LCTRL:
			return Key::CTRL;
		case SDLK_LSHIFT:
			return Key::SHIFT;
		case SDLK_LALT:
			return Key::ALT;
		case SDLK_LGUI:
			return Key::META;
		case SDLK_RCTRL:
			return Key::CTRL;
		case SDLK_RSHIFT:
			return Key::SHIFT;
		case SDLK_RALT:
			return Key::ALT;
		case SDLK_RGUI:
			return Key::META;
		case SDLK_a:
			return (Key)'A';
		case SDLK_b:
			return (Key)'B';
		case SDLK_c:
			return (Key)'C';
		case SDLK_d:
			return (Key)'D';
		case SDLK_e:
			return (Key)'E';
		case SDLK_f:
			return (Key)'F';
		case SDLK_g:
			return (Key)'G';
		case SDLK_h:
			return (Key)'H';
		case SDLK_i:
			return (Key)'I';
		case SDLK_j:
			return (Key)'J';
		case SDLK_k:
			return (Key)'K';
		case SDLK_l:
			return (Key)'L';
		case SDLK_m:
			return (Key)'M';
		case SDLK_n:
			return (Key)'N';
		case SDLK_o:
			return (Key)'O';
		case SDLK_p:
			return (Key)'P';
		case SDLK_q:
			return (Key)'Q';
		case SDLK_r:
			return (Key)'R';
		case SDLK_s:
			return (Key)'S';
		case SDLK_t:
			return (Key)'T';
		case SDLK_u:
			return (Key)'U';
		case SDLK_v:
			return (Key)'V';
		case SDLK_w:
			return (Key)'W';
		case SDLK_x:
			return (Key)'X';
		case SDLK_y:
			return (Key)'Y';
		case SDLK_z:
			return (Key)'Z';
		case SDLK_0:
			return (Key)'0';
		case SDLK_1:
			return (Key)'1';
		case SDLK_2:
			return (Key)'2';
		case SDLK_3:
			return (Key)'3';
		case SDLK_4:
			return (Key)'4';
		case SDLK_5:
			return (Key)'5';
		case SDLK_6:
			return (Key)'6';
		case SDLK_7:
			return (Key)'7';
		case SDLK_8:
			return (Key)'8';
		case SDLK_9:
			return (Key)'9';
		case SDLK_F1:
			return Key::F1;
		case SDLK_F2:
			return Key::F2;
		case SDLK_F3:
			return Key::F3;
		case SDLK_F4:
			return Key::F4;
		case SDLK_F5:
			return Key::F5;
		case SDLK_F6:
			return Key::F6;
		case SDLK_F7:
			return Key::F7;
		case SDLK_F8:
			return Key::F8;
		case SDLK_F9:
			return Key::F9;
		case SDLK_F10:
			return Key::F10;
		case SDLK_F11:
			return Key::F11;
		case SDLK_F12:
			return Key::F12;
		case SDLK_KP_0:
			return Key::KP_0;
		case SDLK_KP_1:
			return Key::KP_1;
		case SDLK_KP_2:
			return Key::KP_2;
		case SDLK_KP_3:
			return Key::KP_3;
		case SDLK_KP_4:
			return Key::KP_4;
		case SDLK_KP_5:
			return Key::KP_5;
		case SDLK_KP_6:
			return Key::KP_6;
		case SDLK_KP_7:
			return Key::KP_7;
		case SDLK_KP_8:
			return Key::KP_8;
		case SDLK_KP_9:
			return Key::KP_9;
		default:
			break;
	}

	// Letters
	if (key >= SDLK_a && key <= SDLK_z) {
		return static_cast<Key>(Key::A + (key - SDLK_a));
	}
	// Upper Row Numbers
	if (key >= SDLK_0 && key <= SDLK_9) {
		return static_cast<Key>(Key::KEY_0 + (key - SDLK_0));
	}
	// Function Keys F1-F24
	if (key >= SDLK_F1 && key <= SDLK_F24) {
		return static_cast<Key>(Key::F1 + (key - SDLK_F1));
	}

	// Numeric Keypad
	switch (key) {
		case SDLK_KP_0:
			return Key::KP_0;
		case SDLK_KP_1:
			return Key::KP_1;
		case SDLK_KP_2:
			return Key::KP_2;
		case SDLK_KP_3:
			return Key::KP_3;
		case SDLK_KP_4:
			return Key::KP_4;
		case SDLK_KP_5:
			return Key::KP_5;
		case SDLK_KP_6:
			return Key::KP_6;
		case SDLK_KP_7:
			return Key::KP_7;
		case SDLK_KP_8:
			return Key::KP_8;
		case SDLK_KP_9:
			return Key::KP_9;
		default:
			break;
	}

	// If no logical mapping exists, return Key::NONE
	return Key::NONE;
}

// Always returns the physical keycode (physical mask + scancode)
static inline Key sdl2godot_physical_keycode(SDL_Scancode scancode) {
	// Godot physical keys: (1 << 23) | scancode (0x800000)
	return (Key)(GODOT_KEY_PHYSICAL_MASK | (unsigned int)scancode);
}

static inline int utf8_to_unicode(const char *s) {
	if ((s[0] & 0x80) == 0) {
		return s[0];
	} else if ((s[0] & 0xe0) == 0xc0) {
		return ((s[0] & 0x1f) << 6) | (s[1] & 0x3f);
	} else if ((s[0] & 0xf0) == 0xe0) {
		return ((s[0] & 0x0f) << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
	} else {
		return 0;
	}
}