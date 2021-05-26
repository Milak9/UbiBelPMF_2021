#pragma once

#include "core/core.h"
#include "core/engine.h"
#include "core/system.h"
#include "core/input/inputs.h"

using namespace dagger;

namespace brawler
{

class Menu;

struct Button
{
    Entity background;
    Entity text;
    String textureDefault;
    String textureSelected;
    entt::delegate<void()> onSelected;
};

class Menu : public System
{
public:
    Menu(Bool isShown_ = false, Bool shouldPause_ = true);

    void SpinUp() override;
    void WindDown() override;

    void Toggle();

    Bool isVisible() { return isShown; };

protected:
    Button& AddButton(Float32 x, Float32 y, String text, String textureDefault, String textureSelected);

private:
    virtual void CreateMenu() = 0;
    virtual void DestroyMenu() = 0;

    void OnKeyboardEvent(KeyboardEvent kEvent_);

    Sequence<Button> buttons;
    Bool isShown;
    Bool shouldPause;
    int selected;

};

}