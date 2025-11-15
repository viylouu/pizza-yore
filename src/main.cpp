#include <tail.hpp>
#include <wraps/input.hpp>
#include <cmath>

class Player : public tail::Component {
public:
    tail::Node* handL;
    tail::Node* handR;

    v3 handLRest = v3{-8,0,0};
    v3 handRRest = v3{8,0,0};

    f32 restOffAmt = .02f;
    f32 grabOffAmt = .24f;

    v3 handLVel = v3{0,0,0};
    v3 handRVel = v3{0,0,0};

    tail::Camera* cam;
    f32 camoff = .03f;

    f32 maxDist = 120;

public:
    void init() {
        name = "Player Component";

        vars.push_back(tail::Variable{
                &handLRest, sizeof(v3),
                tail::VarType::VECTOR_3, 
                "left hand rest pos", NULL
            });

        vars.push_back(tail::Variable{
                &handRRest, sizeof(v3),
                tail::VarType::VECTOR_3, 
                "right hand rest pos", NULL
            });

        vars.push_back(tail::Variable{
                &maxDist, sizeof(f32),
                tail::VarType::FLOAT,
                "maximum mouse distance", NULL
            });

        vars.push_back(tail::Variable{
                &camoff, sizeof(f32),
                tail::VarType::FLOAT,
                "camera offset (by mouse)", NULL
            });

        vars.push_back(tail::Variable{
                &restOffAmt, sizeof(f32),
                tail::VarType::FLOAT, 
                "hand resting offset (by mouse)", NULL
            });

        vars.push_back(tail::Variable{
                &grabOffAmt, sizeof(f32),
                tail::VarType::FLOAT, 
                "hand grabbing offset (by mouse)", NULL
            });
    }

public:
    Player* add_to(tail::Node* parent) {
        return (Player*)parent->add_component((Component*)this);
    }

    void update(f32 dt) {
        if (dt > .2f) return;

        bool lgrab = get_mouse(tail::Mouse::LEFT);
        bool rgrab = get_mouse(tail::Mouse::RIGHT);

        v3 cammouse = cam->mouse_to_this() - node->pos;

        f32 mag = std::sqrt(cammouse.x*cammouse.x + cammouse.y*cammouse.y);
        v3 normed = cammouse / mag;
        mag = std::fmin(mag, maxDist);

        cammouse = normed * mag + node->pos;

        { // hand movement
            v3 handLTarg = cammouse - handLRest;
            v3 handRTarg = cammouse - handRRest;

            if (lgrab) handLTarg *= grabOffAmt;
            else handLTarg *= restOffAmt;
            if (rgrab) handRTarg *= grabOffAmt;
            else handRTarg *= restOffAmt;

            handLTarg += handLRest;
            handRTarg += handRRest;

            const f32 d = 12, k = 256;

            v3 x, a;
            x = handL->pos - handLTarg;
            a = (x * -k) - (handLVel * d);
            handLVel += a * dt;
            handL->pos += handLVel * dt;

            x = handR->pos - handRTarg;
            a = (x * -k) - (handRVel * d);
            handRVel += a * dt;
            handR->pos += handRVel * dt;

            cam->node->pos = node->pos + cammouse * camoff;
        }
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
