#include "libtcod.hpp"

class Actor;
class Map;

class Engine {
public :
   TCODList<Actor *> actors;
   Actor *player;
   Map *map;
   int fovRadius;

   Engine();
   ~Engine();
   void update();
   void render();
   bool computeFov;
};

extern Engine engine;