#pragma once
#include "../external/ImGui/imgui.h"

typedef struct
{
	ImU32 R;
	ImU32 G;
	ImU32 B;
} RGB;

ImU32 Color(RGB color)
{
	return IM_COL32(color.R, color.G, color.B, 255);
}

namespace Render
{
	void DrawLine(float x1, float x2, float y1, float y2, float thickness, RGB color)
	{
		ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), Color(color), thickness);
	}
	void DrawRect(int x, int y, int w, int h, float thickness, RGB color)
	{
		ImGui::GetBackgroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), Color(color), 0, 0, thickness);
	}
	void DrawCircle(float x, float y, float radius, RGB color)
	{
		ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(x, y), radius, Color(color), 0, 1);
	}
	void DrawLabel(const char* text, float x, float y, RGB color, bool outlined = false)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text);
		ImVec2 pos = ImVec2(x - textSize.x / 2.0f, y);

		if (outlined) {
			ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(pos.x - 1, pos.y), ImColor(0, 0, 0), text);
			ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(pos.x + 1, pos.y), ImColor(0, 0, 0), text);
			ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(pos.x, pos.y - 1), ImColor(0, 0, 0), text);
			ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(pos.x, pos.y + 1), ImColor(0, 0, 0), text);
		}

		ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos, Color(color), text);
	}
}