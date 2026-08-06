#pragma once

#include "floydia/rendering/renderable.hpp"

#if defined(FLOYD_EDITOR_PANEL)
#include "imgui/imgui.h"
#endif

namespace floyd {
namespace ui {

	bool slider_scroll_float(const char* label, float* value, float min, float max, const char* format = "%.3f") noexcept;
	bool drag_scroll_float(const char* label, float* value, float speed, float min, float max, const char* format = "%.3f") noexcept;
	bool drag_scroll_float3(const char* label, float v[3], float speed, float min, float max, const char* format = "%.3f") noexcept;
	bool color_edit_scroll4(const char* label, float color[4]) noexcept;
	void draw_editor_panel(Renderable* obj) noexcept;

#if defined(FLOYD_EDITOR_PANEL)
	// https://github.com/Hedgehogsoft/imgui_impl_rgfw.h/blob/main/imgui_impl_rgfw.h
	ImGuiKey KeyToImGuiKey(const int key) noexcept;
#endif

} // namespce ui
} // namespace floyd
