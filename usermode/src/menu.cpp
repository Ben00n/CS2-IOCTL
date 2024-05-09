#include "menu.h"

Menu::Menu() : espEnabled(true) {}

void Menu::Render() {
    ImGui::Begin("asd", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Checkbox("ESP", &espEnabled);
    ImGui::End();
}

bool Menu::IsESPEnabled() const {
    return espEnabled;
}