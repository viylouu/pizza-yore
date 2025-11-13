#include <tail.hpp>
#include <wraps/input.hpp>

class Player : public tail::Component {
public:
    tail::Node* handL;
    tail::Node* handR;

    tail::Camera* cam;

public:
    Player* add_to(tail::Node* parent) {
        return (Player*)parent->add_component((Component*)this);
    }

    void update(f32 dt) {
        UNUSED(dt);

        handL->pos = cam->mouse_to_this(tail::get_mouse_pos());
    }
};

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

            tail::Node* player = (new tail::Node())->add_to(scene);
            player->name = "player";
            Player* player_p = (new Player())->add_to(player);

            tail::Node* pbod = (new tail::Node())->add_to(player);
            pbod->name = "player body";
            tail::Renderer2d* pbod_r2d = (new tail::Renderer2d())->add_to(pbod);
            pbod_r2d->typedata = tail::Renderer2d::Tex{ 
                .tex = data.tex.player, 
                .tint = v4{1},
                .sample = pbod_sample
            };
            pbod_r2d->cams.push_back(cam);
            pbod->scale = v3{pbod_sample.z,pbod_sample.w,1};

            tail::Node* phandL = (new tail::Node())->add_to(player);
            phandL->name = "player hand (L)";
            tail::Renderer2d* phandL_r2d = (new tail::Renderer2d())->add_to(phandL);
            phandL_r2d->typedata = tail::Renderer2d::Tex{
                .tex = data.tex.player,
                .tint = v4{1},
                .sample = phand_sample
            };
            phandL_r2d->cams.push_back(cam);
            phandL->scale = v3{phand_sample.z,phand_sample.w,1};

            tail::Node* phandR = (new tail::Node())->add_to(player);
            phandR->name = "player hand (R)";
            tail::Renderer2d* phandR_r2d = (new tail::Renderer2d())->add_to(phandR);
            phandR_r2d->typedata = tail::Renderer2d::Tex{
                .tex = data.tex.player,
                .tint = v4{1},
                .sample = phand_sample
            };
            phandR_r2d->cams.push_back(cam);
            phandR->scale = v3{phand_sample.z,phand_sample.w,1};

            player_p->handL = phandL;
            player_p->handR = phandR;
            player_p->cam = cam;
        }

        void load_scene_generics(tail::Node* scene, assetdata& data) {
            tail::Node* cam = scene->add_child(new tail::Node());
            cam->name = "camera";
            tail::Camera* cam_cam = (new tail::Camera(480,270))->add_to(cam);
            cam_cam->is_master = true;
            cam_cam->bgcolor = v3{.2,.4,.3};

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
