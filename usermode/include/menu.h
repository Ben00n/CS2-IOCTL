#pragma once
#include "../external/ImGui/imgui.h"

class Menu {
public:
    Menu();
    void Render();
    bool IsESPEnabled() const;

private:
    bool espEnabled;
};