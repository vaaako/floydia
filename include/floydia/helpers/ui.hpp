#pragma once

#include "imgui/imgui.h"

namespace floyd {
namespace ui {
#if !defined(NO_EDITOR_PANEL)
	bool slider_scroll_float(const char* label, float* value, float min, float max, const char* format = "%.3f") noexcept;
	bool drag_scroll_float(const char* label, float* value, float speed, float min, float max, const char* format = "%.3f") noexcept;
	bool drag_scroll_float3(const char* label, float v[3], float speed, float min, float max, const char* format = "%.3f") noexcept;
	bool color_edit_scroll4(const char* label, float color[4]) noexcept;
	// https://github.com/Hedgehogsoft/imgui_impl_rgfw.h/blob/main/imgui_impl_rgfw.h
	ImGuiKey KeyToImGuiKey(const int key) noexcept;
#endif
} // namespce ui
} // namespace floyd
