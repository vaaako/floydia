/*
*
*	RGFW 1.8.1

* Copyright (C) 2022-25 Riley Mabb (@ColleagueRiley)
*
* libpng license
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.

* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
*
*/

// Changes:
//	- Enum names

#pragma once

#include <cstdint>

namespace floyd {

enum class Keycode : uint8_t {
	KEYNULL = 0,
	ESCAPE = '\033',
	BACKTICK = '`',
	
	K_0 = '0',
	K_1 = '1',
	K_2 = '2',
	K_3 = '3',
	K_4 = '4',
	K_5 = '5',
	K_6 = '6',
	K_7 = '7',
	K_8 = '8',
	K_9 = '9',

	MINUS = '-',
	EQUALS = '=',
	BACKSPACE = '\b',
	TAB = '\t',
	SPACE = ' ',
	
	A = 'a',
	B = 'b',
	C = 'c',
	D = 'd',
	E = 'e',
	F = 'f',
	G = 'g',
	H = 'h',
	I = 'i',
	J = 'j',
	K = 'k',
	L = 'l',
	M = 'm',
	N = 'n',
	O = 'o',
	P = 'p',
	Q = 'q',
	R = 'r',
	S = 's',
	T = 't',
	U = 'u',
	V = 'v',
	W = 'w',
	X = 'x',
	Y = 'y',
	Z = 'z',

	PERIOD = '.',
	COMMA = ',',
	SLASH = '/',
	BRACKET = '[',
	CLOSEBRACKET = ']',
	SEMICOLON = ';',
	APOSTROPHE = '\'',
	BACKSLASH = '\\',
	RETURN = '\n',
	ENTER = RETURN,
	DELETE = '\177', /* 127 */
	
	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	F13,
	F14,
	F15,
	F16,
	F17,
	F18,
	F19,
	F20,
	F21,
	F22,
	F23,
	F24,
	F25,

	CAPSLOCK,
	SHIFTL,
	CONTROLL,
	ALTL,
	SUPERL,
	SHIFTR,
	CONTROLR,
	ALTR,
	SUPERR,

	UP,
	DOWN,
	LEFT,
	RIGHT,

	INSERT,
	MENU,
	END,
	HOME,
	PAGEUP,
	PAGEDOWN,
	NUMLOCK,

	KP_SLASH,
	KP_MULTIPLY,
	KP_PLUS,
	KP_MINUS,
	KP_EQUAL,
	KP_1,
	KP_2,
	KP_3,
	KP_4,
	KP_5,
	KP_6,
	KP_7,
	KP_8,
	KP_9,
	KP_0,
	KP_PERIOD,
	KP_RETURN,

	SCROLLLOCK,
	PRINTSCREEN,
	PAUSE,
	WORLD1,
	WORLD2,

	NUM_KEY = 255 /* PADDING FOR ALIGNMENT ~(175 BY DEFAULT) */
};

} // namespace floyd
