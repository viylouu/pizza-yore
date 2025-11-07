#include <tail.hpp>

class Game : public tail::Program {
public:
    void init(tail::Settings* sets) {
        sets->bgcolor = v3{.2,.4,.3};

        v4 sample{0,0,16,32};

        tail::Node* n = scene->add_child(new tail::Node());
        tail::Renderer2d* c = (tail::Renderer2d*)n->add_component(new tail::Renderer2d());
        c->typedata = tail::Renderer2d::Tex{ 
            .tex = new tail::Texture("data/sprites/player.png"), 
            .tint = v4{1},
            .sample = sample
        };

        n->scale = v3{sample.z,sample.w,1};
    }
};

tail::Program* create_game() {
    return new Game();
}
