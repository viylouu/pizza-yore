#include <tail.hpp>

class Game : public tail::Program {
public:
    struct assetdata {
        struct {
            tail::Texture* player;

            void load() {
                player = new tail::Texture("data/sprites/player.png");
            }
            
            void unload() {
                delete player;
            }
        } tex;
    } data;

    struct {
        void load_scene_generics(tail::Node* scene, assetdata& data) {
            tail::Node* cam = scene->add_child(new tail::Node());
            cam->name = "camera";
            tail::Camera* cam_cam = (tail::Camera*)cam->add_component(new tail::Camera(480,270));
            cam_cam->is_master = true;

            v4 sample{0,0,16,32};

            tail::Node* pbod = scene->add_child(new tail::Node());
            pbod->name = "player body";
            tail::Renderer2d* pbod_r2d = (tail::Renderer2d*)pbod->add_component(new tail::Renderer2d());
            pbod_r2d->typedata = tail::Renderer2d::Tex{ 
                .tex = data.tex.player, 
                .tint = v4{1},
                .sample = sample
            };
            pbod_r2d->cams.push_back(cam_cam);

            pbod->scale = v3{sample.z,sample.w,1};
        }
    } state;

    void init(tail::Settings* sets) {
        sets->bgcolor = v3{.2,.4,.3};

        data.tex.load();
        state.load_scene_generics(scene, data);
    }

    void exit() {
        data.tex.unload();
    }
};

tail::Program* create_game() {
    return new Game();
}
