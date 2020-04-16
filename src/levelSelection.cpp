#include "levelSelection.h"
#include "mainScene.h"
#include "main.h"

#include <sp2/graphics/gui/loader.h>

void openLevelSelection()
{
    sp::gui::Loader loader("gui/level_select.gui");

    sp::P<sp::gui::Widget> menu = loader.create("LEVEL_SELECT");

    auto addLevel = [&](int index, const sp::string& label)
    {
        auto widget = loader.create("@MAIN_BUTTON", menu->getWidgetWithID("LEVELS"));
        widget->setAttribute("caption", sp::string(index) + ") " + label);
        widget->setEventCallback([=](sp::Variant v) mutable
        {
            menu.destroy();
            Scene::level_number = index;
            new Scene();
        });
    };
    addLevel(1, "Glass of water");
    addLevel(2, "Mix paint");
    addLevel(3, "Fire it up");
    addLevel(4, "Stronger!");
    addLevel(5, "Explosive");
    addLevel(6, "Make love");

    menu->getWidgetWithID("QUIT")->setEventCallback([=](sp::Variant v) mutable
    {
        menu.destroy();
        openMainMenu();
    });
}
