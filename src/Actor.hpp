#include "libtcod.hpp"

class Actor{
public :
    int x, y;       //position
    int ch;         //ascii code
    TCODColor color; //color

    Actor (int x, int y, int ch, const TCODColor &col);
    void render() const;
};
