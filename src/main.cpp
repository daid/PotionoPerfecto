#include <sp2/engine.h>
#include <sp2/window.h>
#include <sp2/logging.h>
#include <sp2/random.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/io/zipResourceProvider.h>
#include <sp2/io/fileSelectionDialog.h>
#include <sp2/audio/sound.h>
#include <sp2/audio/music.h>
#include <sp2/audio/musicPlayer.h>
#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/gui/widget/image.h>
#include <sp2/graphics/scene/graphicslayer.h>
#include <sp2/graphics/scene/basicnoderenderpass.h>
#include <sp2/graphics/scene/collisionrenderpass.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/renderTexture.h>
#include <sp2/graphics/opengl.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/camera.h>
#include <sp2/io/keybinding.h>

#include "main.h"
#include "mainScene.h"
#include "levelSelection.h"


sp::P<sp::Window> window;
sp::io::Keybinding escape_key("ESCAPE", {"Escape", "AC Back"});
Controller controller;

sp::RenderTexture* water_texture;

//Set to true to enable mod loading support.
static bool can_load_mod = false;


static void openOptionsMenu();
static void openCreditsMenu();
static void openRebindMenu();
void openMainMenu()
{
    sp::P<sp::gui::Widget> menu = sp::gui::Loader::load("gui/main_menu.gui", "MAIN_MENU");
    menu->getWidgetWithID("START")->setEventCallback([=](sp::Variant v) mutable {
        can_load_mod = false;
        menu.destroy();
        //new Scene();
        openLevelSelection();
    });
    menu->getWidgetWithID("OPTIONS")->setEventCallback([=](sp::Variant v) mutable {
        menu.destroy();
        openOptionsMenu();
    });
    menu->getWidgetWithID("CREDITS")->setEventCallback([=](sp::Variant v) mutable {
        menu.destroy();
        openCreditsMenu();
    });
    menu->getWidgetWithID("QUIT")->setEventCallback([](sp::Variant v){
        sp::Engine::getInstance()->shutdown();
    });
    menu->getWidgetWithID("MOD")->setVisible(can_load_mod);
    menu->getWidgetWithID("MOD")->setEventCallback([=](sp::Variant v) mutable {
        sp::io::openFileDialog(".zip", [=](const sp::string& filename) mutable {
            if (can_load_mod)
            {
                can_load_mod = false;
                LOG(Info, "Loading mod:", filename);
                new sp::io::ZipResourceProvider(filename, 1);
                if (menu)
                {
                    menu.destroy();
                    openMainMenu();
                }
            }
        });
    });
#ifdef EMSCRIPTEN
    menu->getWidgetWithID("QUIT")->hide();
#endif
}

static void openOptionsMenu()
{
    sp::P<sp::gui::Widget> menu = sp::gui::Loader::load("gui/main_menu.gui", "OPTIONS_MENU");
    menu->getWidgetWithID("EFFECT_VOLUME")->getWidgetWithID("VALUE")->setAttribute("caption", sp::string(int(sp::audio::Sound::getVolume())) + "%");
    menu->getWidgetWithID("EFFECT_VOLUME")->getWidgetWithID("SLIDER")->setAttribute("value", sp::string(sp::audio::Sound::getVolume()));
    menu->getWidgetWithID("EFFECT_VOLUME")->getWidgetWithID("SLIDER")->setEventCallback([=](sp::Variant v) mutable {
        menu->getWidgetWithID("EFFECT_VOLUME")->getWidgetWithID("VALUE")->setAttribute("caption", sp::string(v.getInteger()) + "%");
        sp::audio::Sound::setVolume(v.getInteger());
    });
    menu->getWidgetWithID("MUSIC_VOLUME")->getWidgetWithID("VALUE")->setAttribute("caption", sp::string(int(sp::audio::Music::getVolume())) + "%");
    menu->getWidgetWithID("MUSIC_VOLUME")->getWidgetWithID("SLIDER")->setAttribute("value", sp::string(sp::audio::Music::getVolume()));
    menu->getWidgetWithID("MUSIC_VOLUME")->getWidgetWithID("SLIDER")->setEventCallback([=](sp::Variant v) mutable {
        menu->getWidgetWithID("MUSIC_VOLUME")->getWidgetWithID("VALUE")->setAttribute("caption", sp::string(v.getInteger()) + "%");
        sp::audio::Music::setVolume(v.getInteger());
    });
    menu->getWidgetWithID("FULLSCREEN")->setEventCallback([=](sp::Variant v) mutable {
        window->setFullScreen(!window->getFullScreen());
    });
#ifdef EMSCRIPTEN
    menu->getWidgetWithID("FULLSCREEN")->hide();
#endif
#ifdef ANDROID
    menu->getWidgetWithID("FULLSCREEN")->hide();
#endif
    menu->getWidgetWithID("REBIND")->setEventCallback([=](sp::Variant v) mutable {
        menu.destroy();
        openRebindMenu();
    });
    menu->getWidgetWithID("BACK")->setEventCallback([=](sp::Variant v) mutable {
        menu.destroy();
        openMainMenu();
    });

    for(auto keybinding : controller.all)
    {
        sp::P<sp::gui::Widget> keybinding_menu = sp::gui::Loader::load("gui/main_menu.gui", "@OPTIONS_KEYBINDING", menu->getWidgetWithID("OPTIONS"));
        keybinding_menu->getWidgetWithID("NAME")->setAttribute("caption", keybinding->getLabel() + ":");
        sp::string value = keybinding->getHumanReadableKeyName(0);
        //for(int n=1; keybinding->getHumanReadableKeyName(n) != ""; n++)
        //    value += ", " + keybinding->getHumanReadableKeyName(n);
        keybinding_menu->getWidgetWithID("VALUE")->setAttribute("caption", value.strip());
    }
}

class Rebinder : public sp::Node
{
public:
    Rebinder(sp::P<sp::Node> parent)
    : sp::Node(parent), binding(controller.all.begin())
    {
        (*binding)->clearKeys();
        (*binding)->startUserRebind();
        sp::P<sp::gui::Widget> w = getParent();
        w->getWidgetWithID("KEY_NAME")->setAttribute("caption", (*binding)->getLabel());
    }

    virtual void onUpdate(float delta) override
    {
        if (!(*binding)->isUserRebinding())
        {
            ++binding;
            if (binding == controller.all.end())
            {
                getParent().destroy();
                openOptionsMenu();
            }
            else
            {
                (*binding)->clearKeys();
                (*binding)->startUserRebind();
                sp::P<sp::gui::Widget> w = getParent();
                w->getWidgetWithID("KEY_NAME")->setAttribute("caption", (*binding)->getLabel());
            }
        }
    }

private:
    sp::PList<sp::io::Keybinding>::Iterator binding;
};

static void openRebindMenu()
{
    sp::P<sp::gui::Widget> menu = sp::gui::Loader::load("gui/main_menu.gui", "REBIND_MENU");
    new Rebinder(menu);
}

class AutoCreditScroller : public sp::Node
{
public:
    using sp::Node::Node;

    virtual void onUpdate(float delta) override
    {
        sp::P<sp::gui::Widget> w = getParent();
        w->layout.position.y -= delta * 30.0f;
        if (w->layout.position.y < -w->layout.size.y)
            w->layout.position.y = 480.0f;
    }
};

static void openCreditsMenu()
{
    sp::P<sp::gui::Widget> menu = sp::gui::Loader::load("gui/main_menu.gui", "CREDITS_MENU");
    new AutoCreditScroller(menu->getWidgetWithID("CREDITS"));
    menu->getWidgetWithID("BACK")->setEventCallback([=](sp::Variant v) mutable {
        menu.destroy();
        openMainMenu();
    });
}

class WaterRenderPass : public sp::BasicNodeRenderPass
{
protected:
    void render(sp::RenderQueue& queue) override
    {
        queue.add([]()
        {
            glBlendFuncSeparate(GL_ONE, GL_ZERO, GL_SRC_ALPHA, GL_ONE);
        });
        sp::BasicNodeRenderPass::render(queue);
    }
    void addNodeToRenderQueue(sp::RenderQueue& queue, sp::P<sp::Node>& node) override
    {
        if (node->render_data.type == sp::RenderData::Type::None && node->render_data.texture && node->render_data.order == 1)
        {
            sp::RenderData render_data = node->render_data;
            render_data.type = sp::RenderData::Type::None;
            queue.add(node->getGlobalTransform(), render_data);
        }
    }
};

int main(int argc, char** argv)
{
    sp::P<sp::Engine> engine = new sp::Engine();

    //Create resource providers, so we can load things.
    sp::io::ResourceProvider::createDefault();

    //Disable or enable smooth filtering by default, enabling it gives nice smooth looks, but disabling it gives a more pixel art look.
    sp::texture_manager.setDefaultSmoothFiltering(true);

    //Create a window to render on, and our engine.
#ifdef ANDROID
    window = new sp::Window(2.0/1.0);
#else
    window = new sp::Window();
#endif
#if !defined(DEBUG) && !defined(EMSCRIPTEN)
    window->setFullScreen(true);
#endif

    sp::gui::Theme::loadTheme("default", "gui/theme/basic.theme.txt");
    new sp::gui::Scene(sp::Vector2d(640, 480));

    sp::P<sp::SceneGraphicsLayer> water_layer = new sp::SceneGraphicsLayer(0);
    water_texture = new sp::RenderTexture("water", sp::Vector2i(256, 128), false);
    water_layer->setTarget(water_texture);
    water_layer->addRenderPass(new WaterRenderPass());
    window->addLayer(water_layer);

    sp::P<sp::SceneGraphicsLayer> scene_layer = new sp::SceneGraphicsLayer(1);
    scene_layer->addRenderPass(new sp::BasicNodeRenderPass());
#ifdef DEBUG
    scene_layer->addRenderPass(new sp::CollisionRenderPass());
#endif
    window->addLayer(scene_layer);

    new sp::audio::MusicPlayer("music");
    new DemoScene();
    openMainMenu();

    engine->run();

    return 0;
}
