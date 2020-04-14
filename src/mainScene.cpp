#include "mainScene.h"
#include "main.h"
#include "levelSelection.h"

#include <json11/json11.hpp>

#include <sp2/graphics/gui/loader.h>
#include <sp2/random.h>
#include <sp2/tween.h>
#include <sp2/io/resourceProvider.h>
#include <sp2/scene/camera.h>
#include <sp2/scene/particleEmitter.h>
#include <sp2/collision/2d/circle.h>
#include <sp2/collision/2d/box.h>
#include <sp2/collision/2d/chains.h>
#include <sp2/collision/2d/polygon.h>
#include <sp2/collision/2d/compound.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/script/environment.h>
#include <sp2/stringutil/convert.h>


static sp::io::Keybinding mouse_wheel("WHEEL", "wheel:y");
static sp::io::Keybinding pointer_press("POINTER", {"pointer:0", "pointer:1"});

int Scene::level_number = 1;

class FluidType
{
public:
    static constexpr size_t count = 8;

    sp::Color color;
    double gravity = -0.5;
    double randomMotion = 1.0;
    double energyChaos = 3.0;
    int energyTransformInto = -1;
    double velocityDamping = 0.0;
};
class FluidMix
{
public:
    double mixingRate = 0.05;
    double energyMixingRate = 0.0;
    int transformInto = -1;
    int energyTransformInto = -1;
};
std::array<FluidType, FluidType::count> fluidTypes;
std::array<std::array<FluidMix, FluidType::count>, FluidType::count> fluidMixes;

class FluidParticle : public sp::Node
{
public:
    FluidParticle(sp::P<sp::Node> parent, sp::Vector2d position, int type)
    : sp::Node(parent)
    {
        setPosition(position);
        amounts[type] = 1.0;

        sp::collision::Circle2D shape(0.3);
        shape.density = 0.04;
        shape.linear_damping = 1;
        shape.setMaskFilterCategory(1);
        setCollisionShape(shape);

        render_data.type = sp::RenderData::Type::None;
        static auto mesh = sp::MeshData::createQuad(sp::Vector2f(1.5, 1.5));
        render_data.mesh = mesh;
        render_data.shader = sp::Shader::get("internal:basic.shader");
        render_data.texture = sp::texture_manager.get("particle.png");
        render_data.order = 1;

        updateParameters();
    }

    void onFixedUpdate() override
    {
        energy = std::max(0.0, energy - 0.001);
        if (highEnergy && energy < 0.3)
        {
            highEnergy = false;
            updateParameters();
        }

        auto velocity = getLinearVelocity2D();
        velocity *= velocityDamping;
        velocity.y += gravity;
        double chaos = randomMotion + energyChaos * energy;
        velocity += sp::Vector2d(sp::random(-chaos, chaos), sp::random(-chaos, chaos));
        setLinearVelocity(velocity);

        if (std::abs(getPosition2D().y) > 100)
            delete this;
    }

    void onCollision(sp::CollisionInfo& info) override
    {
        sp::P<FluidParticle> fp = info.other;
        if (fp)
        {
            bool update = false;
            for(size_t n = 0; n<amounts.size(); n++)
            {
                auto delta = amounts[n] - fp->amounts[n];
                if (delta > 0.001)
                {
                    auto transfer = 0.0;
                    for(size_t m=0; m<amounts.size(); m++)
                    {
                        transfer += fluidMixes[n][m].mixingRate * fp->amounts[m] * delta;
                        if (highEnergy || fp->highEnergy)
                            transfer += fluidMixes[n][m].energyMixingRate * fp->amounts[m] * delta;
                    }

                    transfer = std::max(0.0, std::min(transfer, delta * 0.5));
                    if (transfer > 0.0)
                    {
                        fp->amounts[n] += transfer;
                        amounts[n] -= transfer;
                        update = true;
                    }
                }
            }
            if (update)
            {
                updateParameters();
                fp->updateParameters();
            }
        }
    }

    void addFireEnergy()
    {
        energy = std::min(1.0, energy + 0.02);
        if (!highEnergy && energy > 0.3)
        {
            highEnergy = true;
            updateParameters();
        }
    }

    void updateParameters()
    {
        for(size_t n=0; n<amounts.size(); n++)
        {
            if (fluidTypes[n].energyTransformInto > -1 && highEnergy && amounts[n] > 0.0)
            {
                auto createAmount = std::min(amounts[n], 1.0 - amounts[fluidTypes[n].energyTransformInto]);
                amounts[n] -= createAmount;
                amounts[fluidTypes[n].energyTransformInto] += createAmount;
            }
            for(size_t m=n+1; m<amounts.size(); m++)
            {
                if (fluidMixes[n][m].energyTransformInto > -1 && highEnergy && amounts[n] > 0.0 && amounts[m] > 0.0)
                {
                    auto createAmount = std::min(amounts[n], amounts[m]) * 2.0;
                    createAmount = std::min(createAmount, 1.0 - amounts[fluidMixes[n][m].energyTransformInto]);
                    amounts[n] -= createAmount * 0.5;
                    amounts[m] -= createAmount * 0.5;
                    amounts[fluidMixes[n][m].energyTransformInto] += createAmount;
                }
                if (fluidMixes[n][m].transformInto > -1 && amounts[n] > 0.0 && amounts[m] > 0.0)
                {
                    auto createAmount = std::min(amounts[n], amounts[m]) * 2.0;
                    createAmount = std::min(createAmount, 1.0 - amounts[fluidMixes[n][m].transformInto]);
                    amounts[n] -= createAmount * 0.5;
                    amounts[m] -= createAmount * 0.5;
                    amounts[fluidMixes[n][m].transformInto] += createAmount;
                }
            }
        }

        render_data.color = sp::Color(0, 0, 0, 1);
        gravity = 0.0;
        randomMotion = 0.0;
        energyChaos = 1.0;
        velocityDamping = 1.0;
        for(size_t n=0; n<amounts.size(); n++)
        {
            render_data.color.r += fluidTypes[n].color.r * amounts[n];
            render_data.color.g += fluidTypes[n].color.g * amounts[n];
            render_data.color.b += fluidTypes[n].color.b * amounts[n];
            gravity += fluidTypes[n].gravity * amounts[n];
            randomMotion += fluidTypes[n].randomMotion * amounts[n];
            energyChaos += fluidTypes[n].energyChaos * amounts[n];
            velocityDamping -= fluidTypes[n].velocityDamping * amounts[n];
        }
    }

    std::array<double, FluidType::count> amounts{0};

    double getEnergy() { return energy; }
private:
    double energy = 0.0;
    bool highEnergy = false;
    double gravity = 0.0;
    double randomMotion = 0.0;
    double energyChaos = 0.0;
    double velocityDamping = 1.0;
};

class Bottle : public sp::Node
{
public:
    Bottle(sp::P<sp::Node> parent, sp::string type, double size)
    : sp::Node(parent)
    {
        render_data.type = sp::RenderData::Type::Normal;
        render_data.mesh = sp::MeshData::createQuad(sp::Vector2f(size, size));
        render_data.shader = sp::Shader::get("internal:basic.shader");
        render_data.texture = sp::texture_manager.get("bottles/" + type +".svg");

        std::string err;
        auto json = json11::Json::parse(sp::io::ResourceProvider::get("bottles/" + type + ".json")->readAll(), err);
        float scale = size / 512.0f;
        sp::Vector2f offset(-256 * scale, 256 * scale);
        std::vector<sp::collision::Shape2D*> shapes;
        for(auto& layer : json["layers"].array_items())
        {
            for(auto& obj : layer["objects"].array_items())
            {
                sp::Vector2f position(obj["x"].number_value() * scale, -obj["y"].number_value() * scale);
                position += offset;
                if (obj["polygon"].is_array())
                {
                    auto shape = new sp::collision::Polygon2D();
                    for(auto& p : obj["polygon"].array_items())
                    {
                        auto vp = sp::Vector2f(p["x"].number_value() * scale, -p["y"].number_value() * scale) + position;
                        shape->add(vp);
                    }
                    shapes.push_back(shape);
                }
                else if (obj["height"].is_number() && obj["width"].is_number())
                {
                    float w = obj["width"].number_value() * scale;
                    float h = obj["height"].number_value() * scale;
                    if (w > 0.0 && h > 0.0)
                    {
                        if (obj["name"].string_value() == "contents")
                        {
                            contentsArea.position = sp::Vector2f(position.x, position.y - h);
                            contentsArea.size = sp::Vector2f(w, h);
                        }
                        else
                        {
                            auto shape = new sp::collision::Box2D(w, h, position.x + w * 0.5, position.y - h * 0.5);
                            shapes.push_back(shape);
                        }
                    }
                }
            }
        }

        sp::collision::Compound2D shape{};
        for(auto s : shapes)
            shape.add(s);
        shape.type = sp::collision::Shape::Type::Dynamic;
        setCollisionShape(shape);
        for(auto s : shapes)
            delete s;
    }

    void onFixedUpdate() override
    {
        setLinearVelocity(sp::Vector2d(0, 0));
        setAngularVelocity(0);
    }

    bool hitTest(sp::Vector2f position)
    {
        return contentsArea.contains(getGlobalTransform().inverse() * position);
    }

    sp::Vector2d getMovementCenter()
    {
        return getGlobalPoint2D(sp::Vector2d(contentsArea.center()));
    }

    void onRegisterScriptBindings(sp::ScriptBindingClass& script_binding_class) override
    {
        script_binding_class.bind("setPosition", &Bottle::luaSetPosition);
        script_binding_class.bind("createFluid", &Bottle::luaCreateFluid);
    }

    void luaSetPosition(double x, double y)
    {
        setPosition(sp::Vector2d(x, y));
    }

    void luaCreateFluid(int type, int amount)
    {
        double w = contentsArea.size.x * 0.2;
        double h = contentsArea.size.y * 0.2;
        sp::Vector2d center(contentsArea.center());
        center.y -= h * 0.5;
        for(int n=0; n<amount; n++)
        {
            auto p = getGlobalPoint2D(center + sp::Vector2d(sp::random(-w, w), sp::random(-h, h)));
            new FluidParticle(getParent(), p, type);
        }
    }

    sp::Rect2f contentsArea;
};

class Fire : public sp::Node
{
public:
    Fire(sp::P<sp::Node> parent)
    : sp::Node(parent)
    {
        render_data.type = sp::RenderData::Type::Normal;
        render_data.mesh = sp::MeshData::createQuad(sp::Vector2f(20, 20));
        render_data.shader = sp::Shader::get("internal:basic.shader");
        render_data.texture = sp::texture_manager.get("candle-holder.svg");

        auto pe = new sp::ParticleEmitter(this, "fire.particles");
        pe->setPosition(sp::Vector2d(1.5, 6));
    }

    void onFixedUpdate() override
    {
        auto p = getPosition2D() + sp::Vector2d(1.5, 6);
        getScene()->queryCollision(sp::Rect2d(p.x - 3, p.y, 6, 15), [](sp::P<sp::Node> object)
        {
            sp::P<FluidParticle> fp = object;
            if (fp)
            {
                fp->addFireEnergy();
            }
            return true;
        });
    }
};

class Objective : public sp::Node
{
public:
    Objective(sp::P<Bottle> parent, sp::P<sp::gui::Widget> widget)
    : sp::Node(parent), bottle(parent), widget(widget)
    {
    }

    void onUpdate(float delta) override
    {
        auto m = getGlobalTransform().inverse();
        std::array<double, FluidType::count> amounts{0};
        std::array<double, FluidType::count> stddev{0};
        double energy = 0.0;
        int count = 0;
        for(sp::P<FluidParticle> fp : bottle->getParent()->getChildren())
        {
            if (fp)
            {
                if (bottle->contentsArea.contains(m * sp::Vector2f(fp->getPosition2D())))
                {
                    for(size_t n=0; n<amounts.size(); n++)
                        amounts[n] += fp->amounts[n];
                    energy += fp->getEnergy();
                    count += 1;
                }
            }
        }

        if (count > 0)
        {
            for(size_t n=0; n<amounts.size(); n++)
                amounts[n] /= double(count);
            energy /= double(count);
            for(sp::P<FluidParticle> fp : bottle->getParent()->getChildren())
            {
                if (fp)
                {
                    if (bottle->contentsArea.contains(m * sp::Vector2f(fp->getPosition2D())))
                    {
                        for(size_t n=0; n<amounts.size(); n++)
                            stddev[n] += std::pow(fp->amounts[n] - amounts[n], 2.0);
                    }
                }
            }
            for(size_t n=0; n<amounts.size(); n++)
                stddev[n] /= double(count);
        }
        double level = amounts[targetType] * double(count) / targetAmount;
        if (energy < energyTarget)
            level = std::min(level, energy / energyTarget);

        widget->setAttribute("value", sp::string(level, 3));
        isCompleted = level >= minimalLevel;
        if (isCompleted)
            widget->setAttribute("style", "progressbar.green");
        else
            widget->setAttribute("style", "progressbar.red");
    }

    void onRegisterScriptBindings(sp::ScriptBindingClass& script_binding_class) override
    {
        script_binding_class.bind("setPosition", &Bottle::luaSetPosition);
        script_binding_class.bind("createFluid", &Bottle::luaCreateFluid);
        script_binding_class.bindProperty("energyTarget", energyTarget);
    }

    bool isCompleted = false;

    int targetType = 2;
    double targetAmount = 400;
    double minimalLevel = 0.8;
    double energyTarget = 0.0;

    sp::P<Bottle> bottle;
    sp::P<sp::gui::Widget> widget;
};

static void luaFluidColor(int index, sp::string color)
{
    fluidTypes[index].color = sp::stringutil::convert::toColor(color);
}

static void luaFluidGravity(int index, double gravity)
{
    fluidTypes[index].gravity = gravity;
}

static void luaFluidChaos(int index, double chaos)
{
    fluidTypes[index].randomMotion = chaos;
}

static void luaFluidEnergyChaos(int index, double chaos)
{
    fluidTypes[index].energyChaos = chaos;
}

static void luaFluidEnergyTransform(int index, int target)
{
    fluidTypes[index].energyTransformInto = target;
}

static void luaFluidVelocityDamping(int index, double damping)
{
    fluidTypes[index].velocityDamping = damping;
}

static void luaFluidMixRate(int index1, int index2, double rate)
{
    fluidMixes[index1][index2].mixingRate = fluidMixes[index2][index1].mixingRate = rate;
}

static void luaFluidEnergyMixRate(int index1, int index2, double rate)
{
    fluidMixes[index1][index2].energyMixingRate = fluidMixes[index2][index1].energyMixingRate = rate;
}

static void luaFluidMixesAs(int index1, int index2, int index3)
{
    fluidMixes[index1][index2].transformInto = fluidMixes[index2][index1].transformInto = index3;
}

static void luaFluidEnergyMixesAs(int index1, int index2, int index3)
{
    fluidMixes[index1][index2].energyTransformInto = fluidMixes[index2][index1].energyTransformInto = index3;
}

static sp::P<Bottle> luaBottle(sp::string type, double size)
{
    return new Bottle(sp::Scene::get("MAIN")->getRoot(), type, size);
}

static sp::P<Fire> luaFire(double x, double y)
{
    auto fire = new Fire(sp::Scene::get("MAIN")->getRoot());
    fire->setPosition(sp::Vector2d(x, y));
    return fire;
}

static void luaInstructions(sp::string instructions)
{
    sp::P<Scene> scene = sp::Scene::get("MAIN");
    scene->hud->getWidgetWithID("INSTRUCTIONS_TEXT")->setAttribute("caption", instructions);
}

static sp::P<Objective> luaObjective(sp::P<Bottle> bottle, sp::string label, int type, double amount, double level)
{
    sp::P<Scene> scene = sp::Scene::get("MAIN");
    auto widget = sp::gui::Loader::load("gui/hud.gui", "OBJECTIVE", scene->hud->getWidgetWithID("OBJECTIVES"));
    widget->getWidgetWithID("LABEL")->setAttribute("caption", label);
    auto objective = new Objective(bottle, widget);
    objective->targetType = type;
    objective->targetAmount = amount;
    objective->minimalLevel = level;
    scene->objectives.add(objective);
    return objective;
}

Scene::Scene()
: sp::Scene("MAIN")
{
    auto camera = new sp::Camera(getRoot());
    camera->setOrtographic(sp::Vector2d(60, 40));
    setDefaultCamera(camera);

    sp::Node* node = new sp::Node(camera);
    node->render_data.type = sp::RenderData::Type::Transparent;
    node->render_data.mesh = sp::MeshData::createQuad(sp::Vector2f(2.0, 2.0), sp::Vector2f(0, 1), sp::Vector2f(1, 0));
    node->render_data.shader = sp::Shader::get("water.shader");
    node->render_data.texture = water_texture;
    node->render_data.order = -1;

    sp::gui::Loader loader("gui/hud.gui");
    hud = loader.create("HUD");
    hud->getWidgetWithID("RESET")->setEventCallback([](sp::Variant v)
    {
        sp::Scene::get("MAIN").destroy();
        new Scene();
    });

    {
        float w = 100;
        float h = 70;
        auto box = new sp::Node(getRoot());
        sp::collision::Box2D shape1(2, h + 2, w *-0.5-1, 0);
        sp::collision::Box2D shape2(2, h + 2, w * 0.5+1, 0);
        sp::collision::Box2D shape3(w + 2, 2, 0, h * -0.5-1);
        sp::collision::Box2D shape4(w + 2, 2, 0, h *  0.5+1);
        shape1.setFilterCategory(1);
        shape2.setFilterCategory(1);
        shape3.setFilterCategory(1);
        shape4.setFilterCategory(1);
        sp::collision::Compound2D shape({&shape1, &shape2, &shape3, &shape4});
        shape.type = sp::collision::Shape::Type::Static;
        box->setCollisionShape(shape);

        box->render_data.type = sp::RenderData::Type::Normal;
        box->render_data.mesh = sp::MeshData::createQuad(sp::Vector2f(w, h));
        box->render_data.shader = sp::Shader::get("internal:color.shader");
        box->render_data.color = sp::Color(0.15, 0.15, 0.15);
        box->render_data.order = -2;
    }

    fluidTypes = std::array<FluidType, FluidType::count>();
    fluidMixes = std::array<std::array<FluidMix, FluidType::count>, FluidType::count>();

    sp::P<sp::script::Environment> script = new sp::script::Environment();
    script->setGlobal("fluidColor", luaFluidColor);
    script->setGlobal("fluidGravity", luaFluidGravity);
    script->setGlobal("fluidChaos", luaFluidChaos);
    script->setGlobal("fluidEnergyChaos", luaFluidEnergyChaos);
    script->setGlobal("fluidEnergyTransform", luaFluidEnergyTransform);
    script->setGlobal("fluidVelocityDamping", luaFluidVelocityDamping);
    script->setGlobal("fluidMixRate", luaFluidMixRate);
    script->setGlobal("fluidEnergyMixRate", luaFluidEnergyMixRate);
    script->setGlobal("fluidMixesAs", luaFluidMixesAs);
    script->setGlobal("fluidEnergyMixesAs", luaFluidEnergyMixesAs);
    script->setGlobal("bottle", luaBottle);
    script->setGlobal("fire", luaFire);
    script->setGlobal("instructions", luaInstructions);
    script->setGlobal("objective", luaObjective);
    script->load("levels/" + sp::string(level_number) + ".lua");
}

Scene::~Scene()
{
    hud.destroy();
}

void Scene::onUpdate(float delta)
{
    if (escape_key.getUp())
    {
        delete this;
        openLevelSelection();
        return;
    }

    if (pointer_press.get())
    {
        hud->getWidgetWithID("INSTRUCTIONS")->hide();
    }

    bool done = true;
    for(auto objective : objectives)
    {
        if (!objective->isCompleted)
            done = false;
    }
    hud->getWidgetWithID("DONE")->setVisible(done);
}

bool Scene::onPointerDown(sp::io::Pointer::Button button, sp::Ray3d ray, int id)
{
    sp::P<sp::Node> pull_object = nullptr;
    for(sp::P<Bottle> bottle : getRoot()->getChildren())
    {
        if (bottle && bottle->hitTest(sp::Vector2f(ray.start.x, ray.start.y)))
        {
            pull_object = bottle;
        }
    }

    if (pull_object)
    {
        auto local = pull_object->getGlobalTransform().inverse() * sp::Vector2f(ray.start.x, ray.start.y);
        pointer_pull_data[id].object = pull_object;
        pointer_pull_data[id].position = sp::Vector2d(local.x, local.y);
        pointer_pull_data[id].target = sp::Vector2d(ray.start.x, ray.start.y);
        return true;
    }
    return false;
}

void Scene::onPointerDrag(sp::Ray3d ray, int id)
{
    pointer_pull_data[id].target = sp::Vector2d(ray.start.x, ray.start.y);
}

void Scene::onPointerUp(sp::Ray3d ray, int id)
{
    pointer_pull_data.erase(id);
}

void Scene::onFixedUpdate()
{
    for(auto it : pointer_pull_data)
    {
        if (!it.second.object)
            continue;

        auto point = it.second.object->getGlobalPoint2D(it.second.position);

        auto force = (it.second.target - point) * 10.0;
        if (force.length() > 30.0)
            force = force.normalized() * 30.0;
        auto diff = point - it.second.object->getMovementCenter();
        auto torque = diff.x * force.y - diff.y * force.x;
        force = force * std::abs(diff.normalized().dot(force.normalized()));
        it.second.object->setLinearVelocity(it.second.object->getLinearVelocity2D() + force);
        it.second.object->setAngularVelocity(it.second.object->getAngularVelocity2D() + torque);
    }
}
