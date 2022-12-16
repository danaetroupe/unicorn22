
#pragma once
#include <iostream>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <vector>
#include <tmxlite/Map.hpp>
#include <tmxlite/Layer.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ObjectGroup.hpp>
#include <tmxlite/Types.hpp>
#include <tmxlite/Object.hpp>
#include <tmxlite/Property.hpp>
using namespace std;

// Header File

/* CLASSES */

class Tools
{
protected:
	bool active;
public:
	bool isActive() { return active; }
};

class Lock
{
protected: 
	bool vLock = true;
	bool hLock = true;
public:
	void setLocks(bool vlock, bool hlock) { vLock = vlock; hLock = hlock; }
	void setVLock(bool vlock) { vLock = vlock; }
	void setHLock(bool hlock) { hLock = hlock; }
	bool getVLock() { return vLock; }
	bool getHLock() { return hLock; }
};


class Texture
{
public: 
	Texture();
	~Texture();

	bool loadFromFile(string path);
	void free();
	void render(int x, int y, SDL_Rect* frame);

	int getWidth();
	int getHeight();
	void setCoords(int x, int y);
	int getX();
	int getY();
	void changeCoords(int dX, int dY);
protected:
	SDL_Texture* lTexture;
	int txWidth, txHeight;
	int x = 0;
	int y = 0;
	string path;
};

class Sprite : public Texture
{
private: 
	int rows, columns, frames;
	const int defaultMoveRate, defaultSpeed;
	int speed, moveRate;
	int gameX = x;
	int gameY = y;
public: 
	Sprite(int columns, int rows, int speed = 8, int moveRate = 1) : columns(columns), rows(rows), frames(rows* columns), speed(speed), moveRate(moveRate), defaultMoveRate(moveRate), defaultSpeed(speed) {};
	~Sprite();
	int getTxWidth() { return txWidth; }
	int getTxHeight() { return txHeight; }
	int getColumns() { return columns; }
	int getRows() { return rows;  }
	int getFrames() { return frames; }
	int getSpeed() { return speed; }
	int getMoveRate() { return moveRate; }
	int getDefaultMoveRate() { return defaultMoveRate; }
	void changeMoveRate(int rate) { moveRate = rate; }
	void resetDefaults() { moveRate = defaultMoveRate; speed = defaultSpeed; }
	void setGameCoords(int dX, int dY) { gameX += dX; gameY += dY; }
};

class Animation
{
private:
	vector<SDL_Rect> frames;
	int currentFrame;
	bool isAnimating;
	int frameTime;
	Sprite& sprite;
	int min, max;
public:
	Animation(Sprite& sprite, bool isAnimating = false) : frameTime(sprite.getSpeed()), sprite(sprite), isAnimating(isAnimating) { reset(); };
	~Animation();
	void animate();
	void reset();
	void calculateValues();
	void changeState(bool isAnimating);
	void changeFrameTime(int time) { if (time != 0) { frameTime = time; } }
	Sprite getSprite() { return sprite; }
	vector<SDL_Rect> getFrames() { return frames; }
	void setMinMax(int min, int max) { this->min = min; this->max = max; }
	int getMin() { return min; }
	int getMax() { return max; }
};

class Player : public virtual Tools, public virtual Lock
{
private:
	Sprite& sprite;
	Animation& anim;
public:
	Player(Sprite& sprite, Animation& anim) : sprite(sprite), anim(anim) {};
	~Player();
	Sprite& getSprite() { return sprite; }
	Animation& getAnimation() { return anim; }

};

class Map : public Texture, public virtual Lock
{
private:
	vector<tmx::FloatRect> boundingBoxes;
	tmx::FloatRect gameExit;
	vector <tmx::Tileset> tileSets;
	vector <tmx::TileLayer> tileLayers;
	vector <SDL_Texture*> textures;
	int ROWS = 0;
	int COLUMNS = 0;
	bool mapLockV = false;
	bool mapLockH = false;

public:
	Map() {};
	~Map() {};
	void addBoundingBox(tmx::FloatRect box) { boundingBoxes.push_back(box); }
	void setGameExit(tmx::FloatRect exit) { gameExit = exit; }
	void addTileSet(tmx::Tileset set) { tileSets.push_back(set); }
	void addTileLayer(tmx::TileLayer layer) { tileLayers.push_back(layer); }
	vector<tmx::Tileset> getTileSets() { return tileSets; }
	vector<tmx::TileLayer> getTileLayers() { return tileLayers; }
	void addTexture(SDL_Texture* tex) { textures.push_back(tex); }
	void setTileSize(tmx::Vector2u size) { txWidth = size.x; txHeight = size.y; }
	void setTileCount(tmx::Vector2u count) { ROWS = count.x; COLUMNS = count.y; }
	int getRows() { return ROWS; }
	int getColumns() { return COLUMNS; }
	vector<tmx::FloatRect> getBoundingBoxes() { return boundingBoxes; }
};


class Game : public virtual Tools
{
private:
	int currentLevel;
	Map& map;
	Player& player;
	const Uint8* keystate = SDL_GetKeyboardState(NULL);
	const int SCREEN_WIDTH = 512;
	const int SCREEN_HEIGHT = 512;
public:
	Game(Map& map, Player& player, int currentLevel = 0) : map(map), player(player), currentLevel(currentLevel) {};
	//int StartGame(int level);
	void handleInput();
	void close();
	bool init();
	bool loadMedia();
	void camera();
	void render();
	int getScreenWidth() { return SCREEN_WIDTH; }
	int getScreenHeight() { return SCREEN_HEIGHT; }
	bool checkCollision();
	void changeUntil(int x, int y);
};





SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

Map basemap;

Sprite nun(3, 4);
Sprite& rNun = nun;
Animation animNun(rNun);
Player player1(nun, animNun);
Game mainGame(basemap, player1);


/* ASSET CREDITS: */
// "[LPC] Horses" Artist: bluecarrot16 License: CC-BY 3.0 / GPL 3.0 / GPL 2.0 / OGA-BY 3.0. Please link to opengameart: http://opengameart.org/content/lpc-horses. Reworked by Jordan Irwin (AntumDeluge).*/
// http://opengameart.org/users/hyptosis