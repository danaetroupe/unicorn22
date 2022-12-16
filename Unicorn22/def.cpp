
#include "header.h"
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

// Implementation File

/* PLAYER */
Player::~Player()
{
	cout << "Player Destroyed" << endl;
}


/* TEXTURES */
Texture::Texture()
{
	lTexture = NULL;
	txWidth = 0;
	txHeight = 0;
	x = 0;
	y = 0;
}

Texture::~Texture() 
{ 
	//free(); 
}


bool Texture::loadFromFile(string path)
{
	bool success = true; 

	free();
	SDL_Texture* newTexture = NULL;

	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		cout << "Unable to load image " << path.c_str() << "! SDL_image Error : " << IMG_GetError() << endl;
		bool success = false;
	}
	else
	{
		newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			cout << "Unable to create texture from " << path.c_str() << "! SDL Error : " << SDL_GetError() << endl;
			bool success = false;
		}
		else
		{
			txWidth = loadedSurface->w;
			txHeight = loadedSurface->h;
		}

		SDL_FreeSurface(loadedSurface);
	}

	lTexture = newTexture;
	return success;

}

void Texture::free() {
	if (lTexture != NULL)
	{
		SDL_DestroyTexture(lTexture);
		lTexture = NULL;
		txWidth = 0;
		txHeight = 0;
	}
}

void Texture::render(int x, int y, SDL_Rect* frame = NULL) 
{
	SDL_Rect renderQuad = { x, y, txWidth, txHeight };
	if (frame != NULL)
	{
		renderQuad.w = frame->w;
		renderQuad.h = frame->h;
	}
	SDL_RenderCopy(gRenderer, lTexture, frame, &renderQuad);
}

int Texture::getWidth() { return txWidth; }
int Texture::getHeight() { return txHeight; }

void Texture::setCoords(int x, int y) 
{
	this->x = x;
	this->y = y;
}

int Texture::getX() { return x; }
int Texture::getY() { return y; }

/* SPRITES */

Sprite::~Sprite()
{
	//rows = 0;
	//columns = 0;
	//frames = 0;
}

void Animation::calculateValues()
{
	vector<SDL_Rect> frames(nun.getFrames());

	int i = 0;
	int xx = 0;
	int yy = 0;
	int ww = sprite.getTxWidth() / sprite.getColumns();
	int hh = sprite.getTxHeight() / sprite.getRows();
	while (i < sprite.getFrames())
	{
		frames[i].w = ww;
		frames[i].h = hh;
		frames[i].x = xx * ww;
		frames[i].y = yy * hh;

		if (xx < sprite.getColumns() - 1) { xx++; }
		else {
			xx = 0;
			yy++;
		}
		i++;
	}
	this->frames = frames;
}


void Texture::changeCoords(int dX, int dY)
{
	this->x += dX;
	this->y += dY;
}

Animation::~Animation()
{
	//reset();
}

void Animation::reset()
{
	currentFrame = 0;
}

void Animation::animate()
{
	if ((currentFrame / frameTime) < min || (currentFrame / frameTime) >= max)
	{
		currentFrame = min * frameTime;
	}

	if(isAnimating)
	{
		SDL_Rect* currentClip = &frames[currentFrame / frameTime];
		sprite.render(sprite.getX(), sprite.getY(), currentClip);
		currentFrame++;
	}
}

void Animation::changeState(bool isAnimating) { this->isAnimating = isAnimating; }

/* GAME */

/*int Game::StartGame(int level)
{
	cout << "StartGame doesn't do anything yet." << endl;
	this->currentLevel = level;
	return level;
}*/

bool Game::init()
{
	bool success = true;

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		cout << "SDL could not initialize! SDL Error:" << SDL_GetError() << endl;
		success = false;
	}
	else
	{
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			cout << "Warning: Linear texture filtering not enabled!" << endl;
		}

		gWindow = SDL_CreateWindow("Danae Troupe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			cout << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
			success = false;
		}

		gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (gRenderer == NULL)
		{
			cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
			success = false;
		}
		else
		{
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

			int imgFlags = IMG_INIT_PNG;
			if (!(IMG_Init(imgFlags) & imgFlags))
			{
				cout << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << endl;
				success = false;
			}
		}
	}

	tmx::Map basemap;
	if (!basemap.load("assets/MapFile.tmx"))
	{
		cout << "Map could not be loaded." << endl;
		success = false;
	}
	else
	{
		::basemap.setTileSize(basemap.getTileSize());
		::basemap.setTileCount(basemap.getTileCount());

		const auto& tilesets = basemap.getTilesets();
		for (const auto& set : tilesets)
		{
			::basemap.addTileSet(set);
		}

		const auto& layers = basemap.getLayers();
		for (const auto& layer : layers)
		{
			if (layer->getType() == tmx::Layer::Type::Object)
			{
				const auto& objectLayer = layer->getLayerAs<tmx::ObjectGroup>();
				const auto& objects = objectLayer.getObjects();
				if (objectLayer.getName() == "BoundingBox")
				{
					int i = 0;
					for (const auto& object : objects)
					{
						::basemap.addBoundingBox(object.getAABB());

					}
				}
				else if (objectLayer.getName() == "GameExit")
				{
					int i = 0;
					for (const auto& object : objects)
					{
						::basemap.setGameExit(object.getAABB());
						i += 1;
					}
					if (i > 1) { cout << "Error: Game Exit contains " << i << " instances." << endl; success = false; }

				}
				else { cout << objectLayer.getName() << ": is an unrecognized object layer." << endl; success = false; }

			}
			else if (layer->getType() == tmx::Layer::Type::Tile)
			{
				const auto& tileLayer = layer->getLayerAs<tmx::TileLayer>();
				::basemap.addTileLayer(tileLayer);
			}

		}
	}

	return success;
}

bool Game::loadMedia()
{
	bool success = true;

	//Load sprite sheet texture
	if (!nun.loadFromFile("assets/unicorn.png"))
	{
		cout << "Failed to load animated unicorn." << endl;
		success = false;
	}
	else
	{
		animNun.calculateValues();
	}
	if (!basemap.loadFromFile("assets/MapFile.png"))
	{
		cout << "Failed to load Map File" << endl;
		success = false;
	}

	return success;
}

void Game::close()
{
	nun.free();

	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	IMG_Quit();
	SDL_Quit();
}

bool Game::checkCollision()
{
	Sprite& sprite = player.getSprite();
	Animation& anim = player.getAnimation();
	vector<SDL_Rect> frames = anim.getFrames();
	
	bool collision = false;
	vector<tmx::FloatRect> boxes = map.getBoundingBoxes();
	SDL_FRect box1{ static_cast<float>(sprite.getX() + map.getX()), static_cast<float>(sprite.getY() + map.getY()),static_cast<float>(frames[0].w), static_cast<float>(frames[0].h) };
	
	for (tmx::FloatRect box : boxes)
	{
		SDL_FRect box2 = { box.left, box.top, box.width, box.height };
	
		if (SDL_HasIntersectionF(&box1, &box2) == SDL_TRUE) { collision = true; break; }
	}
	return collision;
}

void Game::changeUntil(int x, int y)
{
	Sprite& sprite = player.getSprite();

	sprite.changeCoords(x, y);
	map.changeCoords(x, y);
	if (checkCollision()) { changeUntil(x, y); }
	else { render(); }
}

void Game::camera()
{
	int mapX = map.getWidth();
	int mapY = map.getHeight();
	int xMax = mapX - SCREEN_WIDTH;
	int yMax = mapY - SCREEN_HEIGHT;
	Sprite sprite = player.getSprite();

	int currentMapX = map.getX();
	int currentMapY = map.getY();
	int currentSpriteX = sprite.getX();
	int currentSpriteY = sprite.getY();
	Animation& anim = player.getAnimation();
	const auto frames = anim.getFrames();
	int spriteXRight = currentSpriteX + frames[0].w;
	int spriteYBottom = currentSpriteY + frames[0].h;

	if (currentMapX >= xMax)
	{
		map.setHLock(true);
		player.setHLock(false);
	}
	else if (currentMapX < xMax)
	{
		map.setHLock(false);
		player.setHLock(true);
	}
	else if (currentMapX <= 0)
	{
		map.setHLock(true);
		player.setHLock(false);
	}
	else
	{
		cout << "Something went wrong on mapX." << endl;
	}
	if (xMax == 0) { player.setHLock(false); map.setHLock(true); }

	if (currentMapY >= yMax)
	{
		map.setVLock(true);
		player.setVLock(false);
	}
	else if (currentMapY < yMax && currentMapY > 0)
	{
		map.setVLock(false);
		player.setVLock(true);
	}
	else if (currentMapY <= 0)
	{
		map.setVLock(true);
		player.setVLock(false);
	}
	else
	{
		cout << "Something went wrong on mapY." << endl;
	}
	if (yMax == 0) { player.setVLock(false); map.setVLock(true); }

	if (currentSpriteX < 0 || spriteXRight > SCREEN_WIDTH) { player.setHLock(true); }
	if (currentSpriteY < 0 || spriteYBottom > SCREEN_HEIGHT) { player.setVLock(true); }
}

void Game::handleInput()
{
	Sprite& sprite = player.getSprite();
	Animation& anim = player.getAnimation();
	int moveRate = sprite.getMoveRate();
	
	if (keystate[SDL_SCANCODE_LEFT])
	{
		sprite.changeCoords(-moveRate, 0);
		map.changeCoords(-moveRate, 0);
		if (checkCollision())
		{
			changeUntil(nun.getMoveRate(), 0);
		}
		else
		{
			anim.changeState(true);
			anim.setMinMax(9, 12);
		}
		camera();
		if(map.getHLock())
		{
			map.changeCoords(moveRate, 0);
		}
		if (player.getHLock())
		{
			sprite.changeCoords(moveRate, 0);
		}
		render();
	}
	else if (keystate[SDL_SCANCODE_RIGHT])
	{
		sprite.changeCoords(moveRate, 0);
		map.changeCoords(moveRate, 0);
		if (checkCollision())
		{ 
			changeUntil(-moveRate, 0);
		}
		else
		{
			anim.changeState(true);
			anim.setMinMax(3, 6);
		}
		camera();
		if (map.getHLock())
		{
			map.changeCoords(-moveRate, 0);
		}
		if (player.getHLock())
		{
			sprite.changeCoords(-moveRate, 0);
		}
		render();
	}
	else if (keystate[SDL_SCANCODE_UP])
	{
		sprite.changeCoords(0, -moveRate);
		map.changeCoords(0, -moveRate);
		if (checkCollision())
		{ 
			changeUntil(0, moveRate);
		}
		else 
		{
			anim.changeState(true);
			anim.setMinMax(0, 3);
		}
		camera();
		if (map.getVLock())
		{
			map.changeCoords(0, moveRate);
		}
		if (player.getVLock())
		{
			sprite.changeCoords(0, moveRate);
		}
		render();
	}
	else if (keystate[SDL_SCANCODE_DOWN])
	{
		sprite.changeCoords(0, moveRate);
		map.changeCoords(0, moveRate);
		if (checkCollision()) {
			changeUntil(0, -moveRate);
		}
		else 
		{ 
			anim.changeState(true);
			anim.setMinMax(6, 9);
		}
		camera();
		if (map.getVLock())
		{
			map.changeCoords(0, -moveRate);
		}
		if (player.getVLock())
		{
			sprite.changeCoords(0, -moveRate);
		}
		render();
	}
	else { animNun.changeState(false); }

	if (keystate[SDL_SCANCODE_LSHIFT])
	{
		anim.changeFrameTime(sprite.getSpeed() / 2);
		sprite.changeMoveRate(sprite.getDefaultMoveRate() * 2);
	}
	else
	{
		anim.changeFrameTime(sprite.getSpeed());
		sprite.changeMoveRate(sprite.getDefaultMoveRate());
	}
}

void Game::render()
{
	Animation& pAnim = player.getAnimation();

	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(gRenderer);

	SDL_Rect mapRect = { map.getX(), map.getY(), SCREEN_WIDTH, SCREEN_HEIGHT };
	map.render(0, 0, &mapRect);
	pAnim.animate();
	SDL_RenderPresent(gRenderer);
}

int main(int argc, char *args[])
{
	if (!mainGame.init())
	{
		cout << "Failed to initialize!" << endl;
	}
	else
	{
		if (!mainGame.loadMedia())
		{
			cout << "Failed to load media!" << endl;
		}
		else
		{
			bool quit = false;
			SDL_Event e;

			nun.setCoords(mainGame.getScreenWidth() / 2, mainGame.getScreenHeight() / 2 + 100);
			animNun.changeState(true);
			animNun.setMinMax(9, 10);
			mainGame.render();

			while (!quit)
			{
				while (SDL_PollEvent(&e) != 0)
				{
					if (e.type == SDL_QUIT)
					{
						quit = true;
					}
				}
				mainGame.handleInput();
			}
		}
	}

	mainGame.close();

	return 0;
}
