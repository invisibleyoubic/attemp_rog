#include "libtcod.hpp"
#include "Map.hpp"
#include "Actor.hpp"
#include "Engine.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;

static const int MAX_ROOM_MONSTERS = 3;

class BspListener : public ITCODBspCallback {
private :
    Map &map; // a map to dig
    int roomNum; // room number
    int lastx,lasty; // center of the last room
public :
    BspListener(Map &map) : map(map), roomNum(0) {}
    bool visitNode(TCODBsp *node, void *userData) {
    if ( node->isLeaf() ) {
        int x,y,w,h;
        // dig a room
        TCODRandom *rng=TCODRandom::getInstance();
        w=rng->getInt(ROOM_MIN_SIZE, node->w-2);
        h=rng->getInt(ROOM_MIN_SIZE, node->h-2);
        x=rng->getInt(node->x+1, node->x+node->w-w-1);
        y=rng->getInt(node->y+1, node->y+node->h-h-1);
        map.createRoom(roomNum == 0, x, y, x+w-1, y+h-1);
        if ( roomNum != 0 ) {
            // dig a corridor from last room
            map.dig(lastx,lasty,x+w/2,lasty);
            map.dig(x+w/2,lasty,x+w/2,y+h/2);
        }
        lastx=x+w/2;
        lasty=y+h/2;
        roomNum++;
    }
    return true;
    }
};

Map::Map(int width, int height) : width(width),height(height) {
    tiles=new Tile[width*height];

    map=new TCODMap(width,height);

    TCODBsp bsp(0,0,width,height);
    bsp.splitRecursive(NULL,8,ROOM_MAX_SIZE,ROOM_MAX_SIZE,1.5f,1.5f);
    BspListener listener(*this);
    bsp.traverseInvertedLevelOrder(&listener,NULL);

    //setWall(30,22); replaced
    //setWall(50,22); replaced
}

Map::~Map() {
    delete [] tiles;
    delete map;
}

bool Map::isWall(int x, int y) const {
    return !map->isWalkable(x,y);
}

//void Map::setWall(int x, int y) {
//    tiles[x+y*width].canWalk=false;
//}

bool Map::isExplored(int x, int y) const {
    return tiles[x+y*width].explored;
}

bool Map::isInFov(int x, int y) const {
    if ( map->isInFov(x,y) ) {
        tiles[x+y*width].explored=true;
        return true;
    }
    return false;
}

void Map::computeFov() {
    map->computeFov(engine.player->x,engine.player->y,engine.fovRadius);
}

void Map::render() const {
    static const TCODColor darkWall(0,0,100);
    static const TCODColor darkGround(50,50,150);

    static const TCODColor lightWall(130,110,50);
    static const TCODColor lightGround(200,180,50);

    for (int x=0; x < width; x++) {
        for (int y=0; y < height; y++) {
            if ( isInFov(x,y) ) {
                TCODConsole::root->setCharBackground(x,y,
                isWall(x,y) ? lightWall :lightGround );
            } 
            else if ( isExplored(x,y) ) {
                TCODConsole::root->setCharBackground(x,y,isWall(x,y) ? darkWall : darkGround );
            }
        }    
    }
}

void Map::dig(int x1, int y1, int x2, int y2) {
    if ( x2 < x1 ) {
       int tmp=x2;
       x2=x1;
       x1=tmp;
    }
    if ( y2 < y1 ) {
       int tmp=y2;
       y2=y1;
       y1=tmp;
    }
    for (int tilex=x1; tilex <= x2; tilex++) {
        for (int tiley=y1; tiley <= y2; tiley++) {
            map->setProperties(tilex,tiley,true,true);
        }
    }
}

void Map::createRoom(bool first, int x1, int y1, int x2, int y2) {
    dig (x1,y1,x2,y2);
    if ( first ) {
       // put the player in the first room
       engine.player->x=(x1+x2)/2;
       engine.player->y=(y1+y2)/2;
    } 
    else {
        TCODRandom *rng=TCODRandom::getInstance();
        int nbMonsters=rng->getInt(0,MAX_ROOM_MONSTERS);
        while (nbMonsters > 0) {
            int x=rng->getInt(x1,x2);
            int y=rng->getInt(y1,y2);
            if ( canWalk(x,y) ) {
                addMonster(x,y);
            }
            nbMonsters--;
        }
    }
}

bool Map::canWalk(int x, int y) const {
    if (isWall(x,y)) {
        // this is a wall
        return false;
    }
    for (Actor **iterator=engine.actors.begin();
        iterator!=engine.actors.end();iterator++) {
        Actor *actor=*iterator;
        if ( actor->x == x && actor->y == y ) {
            // there is an actor there. cannot walk
            return false;
        }
    }
    return true;
}

void Map::addMonster(int x, int y) {
    TCODRandom *rng=TCODRandom::getInstance();
    if ( rng->getInt(0,100) < 80 ) {
        // create an orc
        engine.actors.push(new Actor(x,y,'o',"orc",TCODColor::green));
    } 
    else {
        // create a troll
        engine.actors.push(new Actor(x,y,'T',"troll",TCODColor::red));               
    }
}