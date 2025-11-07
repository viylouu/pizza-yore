#include <tail.hpp>

class Game : public tail::Program {
public:
    void init(tail::Settings* sets) {
        sets->bgcolor = v3{.2,.4,.3};

        tail::Node* n = scene->add_child(new tail::Node());
        tail::Renderer2d* c = (tail::Renderer2d*)n->add_component(new tail::Renderer2d());
        c->typedata = tail::Renderer2d::Rect{ .col = v4{1,0,0,1} };

        n->scale = v3{64,64,1};
    }
};

tail::Program* create_game() {
    return new Game();
}
