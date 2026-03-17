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

enum class MouseButton : uint8_t {
	LEFT = 0, /*!< left mouse button is pressed */
	MIDDLE, /*!< mouse-wheel-button is pressed */
	RIGHT, /*!< right mouse button is pressed */

	MISC1,
	MISC2,
	MISC3,
	MISC4,
	MISC5,

	NUM_MOUSE
};

} // namespace floyd
