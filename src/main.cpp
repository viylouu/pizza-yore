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
        void load_scene_player(tail::Node* scene, assetdata& data, tail::Camera* cam) {
            v4 pbod_sample{0,0,16,32};
            v4 phand_sample{16,0,16,16};

            tail::Node* player = scene->add_child(new tail::Node());
            player->name = "player";

            tail::Node* pbod = player->add_child(new tail::Node());
            pbod->name = "player body";
            tail::Renderer2d* pbod_r2d = (tail::Renderer2d*)pbod->add_component(new tail::Renderer2d());
            pbod_r2d->typedata = tail::Renderer2d::Tex{ 
                .tex = data.tex.player, 
                .tint = v4{1},
                .sample = pbod_sample
            };
            pbod_r2d->cams.push_back(cam);
            pbod->scale = v3{pbod_sample.z,pbod_sample.w,1};

            tail::Node* phandL = player->add_child(new tail::Node());
            phandL->name = "player hand (L)";
            tail::Renderer2d* phandL_r2d = (tail::Renderer2d*)phandL->add_component(new tail::Renderer2d());
            phandL_r2d->typedata = tail::Renderer2d::Tex{
                .tex = data.tex.player,
                .tint = v4{1},
                .sample = phand_sample
            };
            phandL_r2d->cams.push_back(cam);
            phandL->scale = v3{phand_sample.z,phand_sample.w,1};
        }

        void load_scene_generics(tail::Node* scene, assetdata& data) {
            tail::Node* cam = scene->add_child(new tail::Node());
            cam->name = "camera";
            tail::Camera* cam_cam = (tail::Camera*)cam->add_component(new tail::Camera(480,270));
            cam_cam->is_master = true;

            load_scene_player(scene, data, cam_cam);
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
