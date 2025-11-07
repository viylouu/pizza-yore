#include <tail.hpp>

class Game : public tail::Program {
public:
    void init(tail::Settings* sets) {
        sets->bgcolor = v3{.2,.4,.3};
    }
};

tail::Program* create_game() {
    return new Game();
}
