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
enum class Event : uint8_t {
	NONE = 0, /*!< no event has been sent */
	KEY_PRESSED, /* a key has been pressed */
	KEY_RELEASED, /*!< a key has been released */
	/*! key event note
		the code of the key pressed is stored in
		RGFW_event.key.value
		!!Keycodes defined at the bottom of the RGFW_HEADER part of this file!!

		while a string version is stored in
		RGFW_event.key.valueString

		RGFW_event.key.mod holds the current mod
		this means if CapsLock, NumLock are active or not
	*/
	MOUSE_BUTTON_PRESSED, /*!< a mouse button has been pressed (left,middle,right) */
	MOUSE_BUTTON_RELEASED, /*!< a mouse button has been released (left,middle,right) */
	MOUSE_SCROLL, /*!< a mouse scroll event */
	MOUSE_POS_CHANGED, /*!< the position of the mouse has been changed */
	/*! mouse event note
		the x and y of the mouse can be found in the vector, RGFW_x, y

		RGFW_event.button.value holds which mouse button was pressed
	*/
	WINDOW_MOVED, /*!< the window was moved (by the user) */
	WINDOW_RESIZED, /*!< the window was resized (by the user), [on WASM this means the browser was resized] */
	WINDOW_FOCUSIN, /*!< window is in focus now */
	WINDOW_FOCUSOUT, /*!< window is out of focus now */
	WINDOW_MOUSEENTER, /* mouse entered the window */
	WINDOW_MOUSELEAVE, /* mouse left the window */
	WINDOW_REFRESH, /* The window content needs to be refreshed */

	/* attribs change event note
		The event data is sent straight to the window structure
		with win->x, win->y, win->w and win->h
	*/
	WINDOW_QUIT, /*!< the user clicked the quit button */
	DATA_DROP, /*!< a file has been dropped into the window */
	DATA_DRAG, /*!< the start of a drag and drop event, when the file is being dragged */
	/* drop data note
		The x and y coords of the drop are stored in the vector RGFW_x, y

		RGFW_event.drop.count holds how many files were dropped

		This is also the size of the array which stores all the dropped file string,
		RGFW_event.drop.files
	*/
	WINDOW_MAXIMIZED, /*!< the window was maximized */
	WINDOW_MINIMIZED, /*!< the window was minimized */
	WINDOW_RESTORED, /*!< the window was restored */
	SCALE_UPDATED /*!< content scale factor changed */
};
} // namespace floyd
