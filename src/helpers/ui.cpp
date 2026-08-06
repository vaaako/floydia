#include "floydia/helpers/ui.hpp"

#if defined(FLOYD_EDITOR_PANEL)
#include "floydia/core/core.hpp"
#include "floydia/rgfwimpl.hpp"
#include <algorithm>
#endif

namespace floyd {

#if defined(FLOYD_EDITOR_PANEL)

bool ui::slider_scroll_float(const char* label, float* value, float min, float max, const char* format) noexcept {
	bool changed = ImGui::SliderFloat(label, value, min, max, format);

	if(ImGui::IsItemHovered()) {
		ImGuiIO& io = ImGui::GetIO();

		if(io.MouseWheel != 0.0f) {
			const float range = max - min;
			float step = range * 0.01f;

			// Dynamic precision
			if(io.KeyShift) step *= 10.0f;
			else if(io.KeyCtrl) step *= 0.1f;

			// Update
			*value += io.MouseWheel * step;
			*value = std::clamp(*value, min, max);
			changed = true;
		}
	}

	return changed;
}

bool ui::drag_scroll_float(const char* label, float* value, float speed, float min, float max, const char* format) noexcept {
	bool changed = ImGui::DragFloat(label, value, speed, min, max, format);

	if(ImGui::IsItemHovered()) {
		ImGuiIO& io = ImGui::GetIO();

		if(io.MouseWheel != 0.0f) {
			const float range = max - min;
			float step = range * 0.01f;

			// Dynamic precision
			if(io.KeyShift) step *= 10.0f;
			else if(io.KeyCtrl) step *= 0.1f;

			// Update
			*value += io.MouseWheel * step;
			*value = std::clamp(*value, min, max);
			changed = true;
		}
	}

	return changed;
}

bool ui::drag_scroll_float3(const char* label, float v[3], float speed, float min, float max, const char* format) noexcept {
	bool changed = ImGui::DragFloat3(label, v, speed, min, max, format);

	if(ImGui::IsItemHovered()) {
		ImGuiIO& io = ImGui::GetIO();

		if(io.MouseWheel != 0.0f) {
			const ImVec2 item_min = ImGui::GetItemRectMin();
			const ImVec2 item_max = ImGui::GetItemRectMax();

			const float label_width = ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemInnerSpacing.x;
			const float usable_width = (item_max.x - item_min.x - label_width) / 3.0f;

			int axis = int((io.MousePos.x - item_min.x) / usable_width);
			axis = std::clamp(axis, 0, 2);

			// Dynamic precision
			float step = speed;
			if(io.KeyShift)     step *= 10.0f;
			else if(io.KeyCtrl) step *= 0.1f;

			v[axis] += io.MouseWheel * step;
			if(min != max) v[axis] = std::clamp(v[axis], min, max);

			changed = true;
		}
	}

	return changed;
}

bool ui::color_edit_scroll4(const char* label, float color[4]) noexcept {
	bool changed = ImGui::ColorEdit4(label, color);

	if(ImGui::IsItemHovered()) {
		ImGuiIO& io = ImGui::GetIO();

		if(io.MouseWheel != 0.0f) {
			const ImVec2 item_min = ImGui::GetItemRectMin();
			const ImVec2 item_max = ImGui::GetItemRectMax();

			const float button_width = ImGui::GetFrameHeight() + ImGui::GetStyle().ItemInnerSpacing.x;
			const float label_width = ImGui::CalcTextSize(label).x + ImGui::GetStyle().ItemInnerSpacing.x;
			const float usable_min = item_min.x + button_width;
			const float usable_width = (item_max.x - usable_min - label_width) / 4.0f;

			int channel = int((io.MousePos.x - usable_min) / usable_width);
			channel = std::clamp(channel, 0, 3);

			// Dynamic precision
			float step = 0.01f;
			if(io.KeyShift)     step *= 10.0f;
			else if(io.KeyCtrl) step *= 0.1f;

			color[channel] += io.MouseWheel * step;
			color[channel] = std::clamp(color[channel], 0.0f, 1.0f);

			changed = true;
		}
	}

	return changed;
}

void ui::draw_editor_panel(Renderable* obj) noexcept {
	ImGui::Begin("Properties");
	if(obj == nullptr) {
		ImGui::TextDisabled("No object selected");
		ImGui::End();
		return;
	}

	ImGui::Text("UUID: %lu", obj->uuid());
	if(!obj->name.empty()) ImGui::Text("Name: %s", obj->name.c_str());
	ImGui::Separator();

	// Material
	if(ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
		const vec4<float> color = obj->color_norm();
		float c[4] = { color.x, color.y, color.z, color.w };
		if(ui::color_edit_scroll4("Color", c)) {
			obj->set_color_norm({ c[0], c[1], c[2], c[3] });
		}

		Material& mat = obj->material(); // per-instance data, safe to mutate directly
		float metallic = mat.metallic;
		float roughness = mat.roughness;
		if(ui::drag_scroll_float("Metallic", &metallic, 0.1f, 0.0f, 1.0f))   mat.metallic = metallic;
		if(ui::drag_scroll_float("Roughness", &roughness, 0.1f, 0.0f, 1.0f)) mat.roughness = roughness;
	}

	ImGui::Separator();

	// Transform
	if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		const vec3<float> pos = obj->transform.position();
		float p[3] = { pos.x, pos.y, pos.z };
		if(ui::drag_scroll_float3("Position", p, 0.1f,
				std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
			obj->transform.set_position({ p[0], p[1], p[2] });
		}

		const vec3<float> euler = obj->transform.euler_degrees();
		float r[3] = { euler.x, euler.y, euler.z };
		if(ui::drag_scroll_float3("Rotation", r, 0.25f, -360.0f, 360.0f)) {
			obj->transform.set_rotation({ r[0], r[1], r[2] });
		}
	}

	ImGui::Separator();

	// Textures
	if(ImGui::CollapsingHeader("Textures")) {
		static std::shared_ptr<Texture> selected = nullptr;
		if(selected == nullptr) selected = obj->material().albedo;

		ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, 8.0f);
		ImGui::BeginChild("##tex_scroll", ImVec2(0, 64 + 16), false, ImGuiWindowFlags_HorizontalScrollbar);
		for(auto& [_, texentry] : assets().textures) {
			std::shared_ptr<Texture> tex = texentry.texture;

			ImGui::Image((ImTextureID)(intptr_t)tex->id(), ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
			if(ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("%s", texentry.path.c_str());
				ImGui::EndTooltip();
			}
			if(ImGui::IsItemClicked()) {
				selected = tex;
				obj->set_texture(tex); // notifies the Renderer if persistent
			}
			ImGui::SameLine();
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImGui::Separator();
		ImGui::Text("Filter:");
		if(ImGui::Button("Nearest")) selected->set_filter(Texture::Filter::Nearest);
		ImGui::SameLine();
		if(ImGui::Button("Linear")) selected->set_filter(Texture::Filter::Linear);
		ImGui::Separator();
		ImGui::Text("Wrap:");
		if(ImGui::Button("Repeat")) selected->set_filter(Texture::Filter::Repeat);
		ImGui::SameLine();
		if(ImGui::Button("Clamp")) selected->set_filter(Texture::Filter::Clamp);
		ImGui::SameLine();
		if(ImGui::Button("Mirrored")) selected->set_filter(Texture::Filter::Mirrored);
	}

	ImGui::End();
}

// https://github.com/Hedgehogsoft/imgui_impl_rgfw.h/blob/main/imgui_impl_rgfw.h
ImGuiKey ui::KeyToImGuiKey(const int key) noexcept {
	switch(key) {
		case RGFW_escape:       return ImGuiKey_Escape;
		case RGFW_backtick:     return ImGuiKey_GraveAccent;
		case RGFW_0:            return ImGuiKey_0;
		case RGFW_1:            return ImGuiKey_1;
		case RGFW_2:            return ImGuiKey_2;
		case RGFW_3:            return ImGuiKey_3;
		case RGFW_4:            return ImGuiKey_4;
		case RGFW_5:            return ImGuiKey_5;
		case RGFW_6:            return ImGuiKey_6;
		case RGFW_7:            return ImGuiKey_7;
		case RGFW_8:            return ImGuiKey_8;
		case RGFW_9:            return ImGuiKey_9;
		case RGFW_minus:        return ImGuiKey_Minus;
		case RGFW_equals:       return ImGuiKey_Equal;
		case RGFW_backSpace:    return ImGuiKey_Backspace;
		case RGFW_tab:          return ImGuiKey_Tab;
		case RGFW_space:        return ImGuiKey_Space;
		case RGFW_a:            return ImGuiKey_A;
		case RGFW_b:            return ImGuiKey_B;
		case RGFW_c:            return ImGuiKey_C;
		case RGFW_d:            return ImGuiKey_D;
		case RGFW_e:            return ImGuiKey_E;
		case RGFW_f:            return ImGuiKey_F;
		case RGFW_g:            return ImGuiKey_G;
		case RGFW_h:            return ImGuiKey_H;
		case RGFW_i:            return ImGuiKey_I;
		case RGFW_j:            return ImGuiKey_J;
		case RGFW_k:            return ImGuiKey_K;
		case RGFW_l:            return ImGuiKey_L;
		case RGFW_m:            return ImGuiKey_M;
		case RGFW_n:            return ImGuiKey_N;
		case RGFW_o:            return ImGuiKey_O;
		case RGFW_p:            return ImGuiKey_P;
		case RGFW_q:            return ImGuiKey_Q;
		case RGFW_r:            return ImGuiKey_R;
		case RGFW_s:            return ImGuiKey_S;
		case RGFW_t:            return ImGuiKey_T;
		case RGFW_u:            return ImGuiKey_U;
		case RGFW_v:            return ImGuiKey_V;
		case RGFW_w:            return ImGuiKey_W;
		case RGFW_x:            return ImGuiKey_X;
		case RGFW_y:            return ImGuiKey_Y;
		case RGFW_z:            return ImGuiKey_Z;
		case RGFW_period:       return ImGuiKey_Period;
		case RGFW_comma:        return ImGuiKey_Comma;
		case RGFW_slash:        return ImGuiKey_Slash;
		case RGFW_bracket:      return ImGuiKey_LeftBracket;
		case RGFW_closeBracket: return ImGuiKey_RightBracket;
		case RGFW_semicolon:    return ImGuiKey_Semicolon;
		case RGFW_apostrophe:   return ImGuiKey_Apostrophe;
		case RGFW_backSlash:    return ImGuiKey_Backslash;
		case RGFW_return:       return ImGuiKey_Enter;  // Same as RGFW_enter
		case RGFW_delete:       return ImGuiKey_Delete;
		case RGFW_F1:           return ImGuiKey_F1;
		case RGFW_F2:           return ImGuiKey_F2;
		case RGFW_F3:           return ImGuiKey_F3;
		case RGFW_F4:           return ImGuiKey_F4;
		case RGFW_F5:           return ImGuiKey_F5;
		case RGFW_F6:           return ImGuiKey_F6;
		case RGFW_F7:           return ImGuiKey_F7;
		case RGFW_F8:           return ImGuiKey_F8;
		case RGFW_F9:           return ImGuiKey_F9;
		case RGFW_F10:          return ImGuiKey_F10;
		case RGFW_F11:          return ImGuiKey_F11;
		case RGFW_F12:          return ImGuiKey_F12;
		case RGFW_F13:          return ImGuiKey_F13;
		case RGFW_F14:          return ImGuiKey_F14;
		case RGFW_F15:          return ImGuiKey_F15;
		case RGFW_F16:          return ImGuiKey_F16;
		case RGFW_F17:          return ImGuiKey_F17;
		case RGFW_F18:          return ImGuiKey_F18;
		case RGFW_F19:          return ImGuiKey_F19;
		case RGFW_F20:          return ImGuiKey_F20;
		case RGFW_F21:          return ImGuiKey_F21;
		case RGFW_F22:          return ImGuiKey_F22;
		case RGFW_F23:          return ImGuiKey_F23;
		case RGFW_F24:          return ImGuiKey_F24;
		case RGFW_F25:          return ImGuiKey_None;  // No ImGuiKey_F25
		case RGFW_capsLock:     return ImGuiKey_CapsLock;
		case RGFW_shiftL:       return ImGuiKey_LeftShift;
		case RGFW_controlL:     return ImGuiKey_LeftCtrl;
		case RGFW_altL:         return ImGuiKey_LeftAlt;
		case RGFW_superL:       return ImGuiKey_LeftSuper;
		case RGFW_shiftR:       return ImGuiKey_RightShift;
		case RGFW_controlR:     return ImGuiKey_RightCtrl;
		case RGFW_altR:         return ImGuiKey_RightAlt;
		case RGFW_superR:       return ImGuiKey_RightSuper;
		case RGFW_up:           return ImGuiKey_UpArrow;
		case RGFW_down:         return ImGuiKey_DownArrow;
		case RGFW_left:         return ImGuiKey_LeftArrow;
		case RGFW_right:        return ImGuiKey_RightArrow;
		case RGFW_insert:       return ImGuiKey_Insert;
		case RGFW_menu:         return ImGuiKey_Menu;
		case RGFW_end:          return ImGuiKey_End;
		case RGFW_home:         return ImGuiKey_Home;
		case RGFW_pageUp:       return ImGuiKey_PageUp;
		case RGFW_pageDown:     return ImGuiKey_PageDown;
		case RGFW_numLock:      return ImGuiKey_NumLock;
		case RGFW_kpSlash:      return ImGuiKey_KeypadDivide;
		case RGFW_kpMultiply:   return ImGuiKey_KeypadMultiply;
		case RGFW_kpPlus:       return ImGuiKey_KeypadAdd;
		case RGFW_kpMinus:      return ImGuiKey_KeypadSubtract;
		case RGFW_kpEqual:      return ImGuiKey_KeypadEqual;
		case RGFW_kp1:          return ImGuiKey_Keypad1;
		case RGFW_kp2:          return ImGuiKey_Keypad2;
		case RGFW_kp3:          return ImGuiKey_Keypad3;
		case RGFW_kp4:          return ImGuiKey_Keypad4;
		case RGFW_kp5:          return ImGuiKey_Keypad5;
		case RGFW_kp6:          return ImGuiKey_Keypad6;
		case RGFW_kp7:          return ImGuiKey_Keypad7;
		case RGFW_kp8:          return ImGuiKey_Keypad8;
		case RGFW_kp9:          return ImGuiKey_Keypad9;
		case RGFW_kp0:          return ImGuiKey_Keypad0;
		case RGFW_kpPeriod:     return ImGuiKey_KeypadDecimal;
		case RGFW_kpReturn:     return ImGuiKey_KeypadEnter;
		case RGFW_scrollLock:   return ImGuiKey_ScrollLock;
		case RGFW_printScreen:  return ImGuiKey_PrintScreen;
		case RGFW_pause:        return ImGuiKey_Pause;
		case RGFW_world1:       return ImGuiKey_None;  // Not supported
		case RGFW_world2:       return ImGuiKey_None;  // Not supported
		default:                return ImGuiKey_None;
	}
}

#else

bool slider_scroll_float(const char* label, float* value, float min, float max, const char* format = "%.3f") noexcept {
	(void)label; (void)value; (void)min; (void)max; (void)format;
	return false;
}

bool drag_scroll_float(const char* label, float* value, float speed, float min, float max, const char* format = "%.3f") noexcept {
	(void)label; (void)value; (void)speed; (void)min; (void)max; (void)format;
	return false;
}

bool drag_scroll_float3(const char* label, float v[3], float speed, float min, float max, const char* format = "%.3f") noexcept {
	(void)label; (void)v; (void)speed; (void)min; (void)max; (void)format;
	return false;
}

bool color_edit_scroll4(const char* label, float color[4]) noexcept {
	(void)label; (void)color;
	return false;
}

void draw_editor_panel(Renderable* obj) noexcept {
	(void)obj;
}

#endif

} // namespace floyd
