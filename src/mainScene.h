#ifndef MAIN_SCENE_H
#define MAIN_SCENE_H

#include <sp2/scene/scene.h>
#include <sp2/graphics/gui/widget/widget.h>

class Bottle;
class Objective;
class Scene : public sp::Scene
{
public:
    Scene();
    ~Scene();

    void onUpdate(float delta) override;
    bool onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id) override;
    void onPointerDrag(sp::Ray3d ray, int id) override;
    void onPointerUp(sp::Ray3d ray, int id) override;

    void onFixedUpdate() override;

    struct PullData
    {
        sp::P<Bottle> object;
        sp::Vector2d position;
        sp::Vector2d target;
    };
    std::unordered_map<int, PullData> pointer_pull_data;

    sp::PList<Objective> objectives;
    sp::P<sp::gui::Widget> hud;

    static int level_number;
};

#endif//MAIN_SCENE_H