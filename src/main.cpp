#include <tail.hpp>

class Game : public tail::Program {
public:
    void init(tail::Settings* sets) {
        sets->bgcolor = v3{.2,.4,.3};

        tail::Node* cam = scene->add_child(new tail::Node());
        cam->name = "camera";
        tail::Camera* cam_cam = (tail::Camera*)cam->add_component(new tail::Camera(480,270));
        cam_cam->is_master = true;

        v4 sample{0,0,16,32};

        tail::Node* pbod = scene->add_child(new tail::Node());
        pbod->name = "player body";
        tail::Renderer2d* pbod_r2d = (tail::Renderer2d*)pbod->add_component(new tail::Renderer2d());
        pbod_r2d->typedata = tail::Renderer2d::Tex{ 
            .tex = new tail::Texture("data/sprites/player.png"), 
            .tint = v4{1},
            .sample = sample
        };
        pbod_r2d->cams.push_back(cam_cam);

        pbod->scale = v3{sample.z,sample.w,1};
    }
};

tail::Program* create_game() {
    return new Game();
}
