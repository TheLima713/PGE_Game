#define OLC_PGE_APPLICATION
#include "wbUILib.h"
#include <string>
#include <vector>

olc::vi2d nSz = { 10,6 };
olc::vi2d scale = { 2,2 };
olc::vi2d tileSize = scale * olc::vi2d{ 16,16 };
const int nSprites = 1;
std::string tileSprites[2] = { "GrassTiles.png","GrassWalls.png" };
float fTime = 0;

int nCurrRoomState = 0;
float fLoading = 0.0f;
enum newRoomStates {
	initialization,
	playerJumpIn,
	screenFadeOut,
	newRoomGeneration,
	screenFadeIn,
	playerJumpOut,
	roomLoaded
};

float AngToRad(float f) {
	return (3.1415 * f / 180);
}

class Hitbox;

class Texture {
private:
	olc::vf2d size;
	olc::vf2d scale;
	olc::Sprite* sprite;
	olc::Decal* decal;
	std::array<olc::vf2d, 4> decalPts;
	/*
	[3] = 0,0
	[2] = 0,1
	[1] = 1,1
	[0] = 1,0
	*/
public:
	Texture() {};
	Texture(olc::vf2d pos, olc::vf2d sc, olc::vf2d sz) {
		size = sz;
		scale = sc;
		for (int d = 0; d < 4; d++)
		{
			decalPts[d] = pos + scale * olc::vf2d{ (d / 2) * size.x,((d + 1) / 2 == 1) * size.y };
		}
	}
	void load(std::string sn) {
		sprite = new olc::Sprite(sn);
		decal = new olc::Decal(sprite);
	}
	void update(olc::vf2d pos, int flip) {
		for (int d = 0; d < 4; d++)
		{
			if (!flip) {
				decalPts[d] = pos + scale * olc::vf2d{ (d / 2) * size.x,((d + 1) / 2 == 1) * size.y };
			}
			else {
				decalPts[d] = pos + scale * olc::vf2d{ (d / 2) * size.x,((d + 1) / 2 == 1) * size.y };
			}
		}
	}
	void offset(olc::vf2d offset) {
		for (int d = 0; d < 4; d++)
		{
			decalPts[d] += offset;
		}
	}
	void transform(int type, olc::vf2d offset, int flip) {
		std::array<olc::vf2d, 4> flipPts;
		switch (type) {
		case 0://horizontal
			for (int d = 0; d < 4; d++)
			{
				if (!flip) {
					flipPts[d] = decalPts[3 - d] + scale * olc::vf2d{ (d / 2) ? offset.x : -offset.x,0.0f };
				}
				else {
					flipPts[d] = decalPts[d] + scale * olc::vf2d{ (d / 2) ? -offset.x : offset.x,0.0f };
				}
			}
			break;
		case 1://vertical
			for (int d = 0; d < 4; d++)
			{
				if (!flip) {
					flipPts[d] = decalPts[2 * (d / 2) + !(d % 2)] + scale * olc::vf2d{ 0.0f,((d + 1) / 2 == 1) ? offset.y : -offset.y };
				}
				else {
					flipPts[d] = decalPts[d] + scale * olc::vf2d{ 0.0f,((d + 1) / 2 == 1) ? -offset.y : offset.y };
				}
			}
			break;
		case 2:// both
			for (int d = 0; d < 4; d++)
			{
				if (!flip) {
					flipPts[d] = decalPts[(2 + d) % 4] + scale * olc::vf2d{ (d / 2) ? offset.x : -offset.x,((d + 1) / 2 == 1) ? offset.y : -offset.y };
				}
				else {
					flipPts[d] = decalPts[d] + scale * olc::vf2d{ (d / 2) ? -offset.x : offset.x,((d + 1) / 2 == 1) ? -offset.y : offset.y };
				}
			}
			break;
		case 3:// "/" diagonal
			break;
		}
		decalPts = flipPts;
	}
	void rotate(float a, int flip) {
		olc::vf2d center;
		olc::vf2d radiusIG;
		float radius;
		switch (flip) {
		case 0:
			center = decalPts[3] + (0.5f * (decalPts[1] - decalPts[3]));
			radiusIG = (decalPts[1] - decalPts[3]) / 2;
			radius = sqrt(radiusIG.x * radiusIG.x + radiusIG.y * radiusIG.y);
			for (int d = 0; d < 4; d++)
			{
				float ang = AngToRad(a - 90 * d);
				decalPts[d] = center + scale * olc::vf2d{ ((d + 1) / 2 == 1) ? radius * cos(ang) : radius * cos(ang),(d / 2) ? radius * sin(ang) : radius * sin(ang) };
			}
			break;
		case 1:
			center = decalPts[3] + (0.5f * (decalPts[1] - decalPts[3]));
			radiusIG = (decalPts[3] - decalPts[1]) / 2;
			radius = sqrt(radiusIG.x * radiusIG.x + radiusIG.y * radiusIG.y);
			for (int d = 0; d < 4; d++)
			{
				float ang = AngToRad(a + 90 * d);
				decalPts[d] = center + scale * olc::vf2d{ ((d + 1) / 2 == 1) ? radius * cos(ang) : radius * cos(ang),(d / 2) ? radius * sin(ang) : radius * sin(ang) };
			}
			break;
		}
	}
	void carrouselWIP(float a) {
		for (int d = 0; d < 4; d++)
		{
			float ang = AngToRad(45 + 90 * (d / 2)) + a;
			decalPts[d] += scale * olc::vf2d{ ((d + 1) / 2 == 1) ? size.x * sin(ang) : size.x * sin(ang),(d / 2) ? size.y * cos(ang) : size.y * cos(ang) };
		}
	}
	void wobbleWIP(float ang) {
		for (int d = 0; d < 4; d++)
		{
			decalPts[d] += scale * olc::vf2d{ ((d + 1) / 2 == 1) ? sin(ang) : sin(ang),(d / 2) ? cos(ang) : cos(ang) };
		}
	}
	void distortWIP(float ang) {
		for (int d = 0; d < 4; d++)
		{
			decalPts[d] += scale * olc::vf2d{ ((d + 1) / 2 == 1) ? -sin(ang) : sin(ang),(d / 2) ? -cos(ang) : cos(ang) };
		}
	}
	void hover(olc::vf2d offset) {
		for (int d = 0; d < 4; d++)
		{
			decalPts[d] += scale * olc::vf2d{ (d / 2) * offset.x,((d + 1) / 2 == 1) * offset.y };
		}
	}
	void wave(olc::vf2d offset) {
		for (int d = 0; d < 4; d++)
		{
			decalPts[d] += scale * olc::vf2d{ (d / 2) ? offset.x : -offset.x,((d + 1) / 2 == 1) ? offset.y : -offset.y };
		}
	}
	void bob(olc::vf2d offset) {
		for (int d = 0; d < 4; d++)
		{
			decalPts[d] += scale * olc::vf2d{ (d / 2) ? offset.x : -offset.x,((d + 1) / 2 == 1) ? 0 : offset.y };
		}
	}
	void draw(olc::vi2d state, olc::PixelGameEngine* pge, int flip) {
		pge->DrawPartialWarpedDecal(decal, decalPts, size * state, size);
		//pge->FillCircle(olc::vi2d(decalPts[0]), 2, olc::RED);
		//pge->FillCircle(olc::vi2d(decalPts[1]), 2, olc::YELLOW);
		//pge->FillCircle(olc::vi2d(decalPts[2]), 2, olc::GREEN);
		//pge->FillCircle(olc::vi2d(decalPts[3]), 2, olc::BLUE);
	}
	void drawString(olc::vf2d offset, olc::vf2d scl, std::string sn, olc::Pixel p, olc::PixelGameEngine* pge) {
		pge->DrawStringDecal(decalPts[0] + offset, sn, p, scl);
	}
};
class Item {
private:
	olc::vf2d scale;
	olc::vf2d sheetPos;
	std::string description;
public:
	int quantity;
	std::string name;
	olc::vf2d pos;
	olc::vf2d size;
	Texture texture;
	Item() {};
	Item(std::string n, std::string d, olc::vf2d p, olc::vf2d sp, int qt, olc::vf2d sz, olc::vf2d scl) {
		name = n;
		description = d;
		pos = p;
		size = sz;
		scale = scl;
		sheetPos = sp;
		quantity = qt;
		texture = Texture(pos, scale, size);
	}
	void merge(Item item) {
		quantity += item.quantity;
		pos = (pos + item.pos) / 2;
	}
	void loadTexture(std::string sn) {
		texture.load(sn);
	}
	void draw(olc::PixelGameEngine* pge) {
		texture.draw(sheetPos, pge, 0);
		texture.drawString(scale * size, scale, std::to_string(quantity), olc::WHITE, pge);
	}
};
class RoomItemHandler {
public:
	std::vector<Item> items;
	RoomItemHandler() {};
	void clump() {
		for (int i = 0; i < items.size(); i++)
		{
			for (int j = i; j < items.size(); j++)
			{
				if (i != j) {
					if (items[j].pos.y > (items[i].pos.y - items[i].size.y) && items[j].pos.y < (items[i].pos.y + items[i].size.y)) {
						if (items[j].pos.x > (items[i].pos.x - items[i].size.x) && items[j].pos.x < (items[i].pos.x + items[i].size.x)) {
							if (items[i].name == items[j].name) {
								items[i].merge(items[j]);
								items.erase(items.begin() + j);
							}
						}
					}
				}
			}
		}
	}
	void add(olc::vf2d p, Item i, std::string sn) {
		items.resize(items.size() + 1);
		items[items.size() - 1] = i;
		items[items.size() - 1].loadTexture(sn);
		if (items.size() > 1000) {
			items.erase(items.begin());
		}
	}
	void update(olc::vi2d camPos, float time) {
		clump();

		for (int i = 0; i < items.size(); i++)
		{
			items[i].texture.update(items[i].pos, 0);
			items[i].texture.offset(camPos);
			items[i].texture.transform(2, { sin(time / 10),sin(time / 10) }, 1);
		}
	}
	void draw(olc::PixelGameEngine* pge) {
		for (int i = 0; i < items.size(); i++)
		{
			items[i].draw(pge);
		}
	}
};
class Tool;
class ItemSlot {

};
class Inventory;
class Tile {
private:
	olc::Sprite* sprite;
	olc::PixelGameEngine* pge;
public:
	olc::vi2d minitile[4];
	olc::vi2d type;
	Tile() {
		for (int n = 0; n < 4; n++)
		{
			minitile[n] = { 0,0 };
		}
	}
	Tile(olc::vi2d mt[4], olc::vi2d t, std::string sn, olc::PixelGameEngine* pg) {
		for (int n = 0; n < 4; n++)
		{
			minitile[n] = mt[n];
		}
		type = t;
		sprite = new olc::Sprite(sn);
		pge = pg;
	}
	void draw(olc::vi2d pos, int scale) {
		for (int n = 0; n < 4; n++)
		{
			pge->DrawPartialSprite(olc::vi2d{ pos.x + scale * 8 * (n % 2), pos.y + scale * 8 * (n / 2) }, sprite, { 8 * minitile[n].x,0 }, { 8,8 }, scale, minitile[n].y);
		}
	}
};
class Gate {
private:
	Tile tile;
public:
	olc::vi2d pos;
	int rot;
	int index;
	Gate() {
		pos = { 0,0 };
	}
	Gate(olc::vi2d p, int i, int r, Tile t) {
		pos = p;
		rot = r;
		index = i;
		tile = t;
	}
	void draw(olc::vi2d p, int s) {
		tile.draw(p, s);
	}
};
class Room {
public:
	olc::vi2d size;/*size of room in tiles*/
	std::vector<std::vector<Tile>> tiles;
	std::vector<Gate> gates;
	RoomItemHandler itemHandler;
	olc::PixelGameEngine* pge;
	Room() {
		size = { 0,0 };
	}
	Room(olc::vi2d sz, olc::PixelGameEngine* pg) {
		size = sz;
		pge = pg;
	}
	std::vector<Gate> newGates(olc::vi2d size, int n) {
		std::vector<Gate> tempGates;
		tempGates.resize(n);
		for (int g = 0; g < tempGates.size(); g++)
		{
			int rndWall = std::rand() % 4;
			int rndXY = 0;
			olc::vi2d pos;
			olc::vi2d mts[4][4] = {
				{{3,0},{3,1},{2,2},{2,3}},
				{{3,0},{1,1},{3,2},{1,3}},
				{{2,0},{2,1},{3,2},{3,3}},
				{{1,0},{3,1},{1,2},{3,3}}
			};
			int index = 0;
			switch (rndWall) {
			case 0:
				rndXY = std::rand() % size.x;
				pos = { rndXY,0 };
				index = 0;
				break;
			case 1:
				rndXY = std::rand() % size.y;
				pos = { 0,rndXY };
				index = 1;
				break;
			case 2:
				rndXY = std::rand() % size.x;
				pos = { rndXY,size.y - 1 };
				index = 2;
				break;
			case 3:
				rndXY = std::rand() % size.y;
				pos = { size.x - 1,rndXY };
				index = 3;
				break;
			}
			tempGates[g] = Gate(pos, -1, index, Tile(mts[index], { 0,2 }, tileSprites[1], pge));
		}
		return tempGates;
	}
	std::vector<std::vector<Tile>> newTileGrid(olc::vi2d size, int type, std::string sn, olc::PixelGameEngine* pg) {
		pge = pg;
		std::vector<std::vector<Tile>> tempTiles;
		tempTiles.resize(size.y);
		for (int y = 0; y < size.y; y++)
		{
			tempTiles[y].resize(size.x);
			for (int x = 0; x < size.x; x++)
			{
				olc::vi2d mts[4];
				int rnd = std::rand() % 2;
				for (int t = 0; t < 4; t++)
				{
					mts[t] = { rnd,t };
				}
				tempTiles[y][x] = Tile(mts, { type,rnd }, tileSprites[type], pg);
			}
		}
		return tempTiles;
	}
	int gateUsed(olc::vi2d pos) {
		for (int g = 0; g < gates.size(); g++)
		{
			for (int y = tileSize.y * gates[g].pos.y; y <= tileSize.y * (gates[g].pos.y + 1); y++)
			{
				if (y > pos.y && y <= pos.y + tileSize.y) {
					for (int x = tileSize.x * gates[g].pos.x; x <= tileSize.x * (gates[g].pos.x + 1); x++)
					{
						if (x > pos.x && x <= pos.x + tileSize.x) {
							return g;
						}
					}
				}
			}
		}
	}
	void updateTiles() {
		for (int y = 0; y < size.y; y++)
		{
			for (int x = 0; x < size.x; x++)
			{
				Tile nTile = tiles[y][x];
				if (nTile.type.y == 1) {
					for (int n = 0; n < 4; n++)
					{
						olc::vi2d mpos = { -(x > 0 && !(n % 2)) + (x < (size.x - 1) && n % 2),-(y > 0 && !(n / 2)) + (y < (size.y - 1) && n / 2) };
						Tile horizTile = tiles[y][x + mpos.x];
						Tile vertTile = tiles[y + mpos.y][x];
						Tile diagTile = tiles[y + mpos.y][x + mpos.x];
						int tiles = 1 * (horizTile.type != nTile.type) + 2 * (vertTile.type != nTile.type) + 4 * (diagTile.type != nTile.type);
						switch (tiles) {
						case 0:
							nTile.minitile[n] = { 1,std::rand() % 4 };
							break;
						case 1:
							nTile.minitile[n] = { 2,(n % 2) + 2 * (std::rand() % 2) };
							break;
						case 2:
							nTile.minitile[n] = { 3,2 * (n / 2) + std::rand() % 2 };
							break;
						case 3:
							nTile.minitile[n] = { 4,n };
							break;
						case 4:
							nTile.minitile[n] = { 5,n };
							break;
						case 5:
							nTile.minitile[n] = { 2,(n % 2) + 2 * (std::rand() % 2) };
							break;
						case 6:
							nTile.minitile[n] = { 3,2 * (n / 2) + std::rand() % 2 };
							break;
						case 7:
							nTile.minitile[n] = { 4,n };
							break;
						}
					}
				}
				else {
					for (int n = 0; n < 4; n++)
					{
						nTile.minitile[n].y = std::rand() % 4;
					}
				}
				tiles[y][x] = nTile;
			}
		}
	}
	void draw(olc::vi2d camPos, olc::vi2d camSize, int scale) {
		for (int y = 0 - (camPos.y / tileSize.y > 0); y <= camSize.y - (((camPos.y / tileSize.y) + camSize.y) >= size.y); y++)
		{
			for (int x = 0 - (camPos.x / tileSize.x > 0); x <= camSize.x - (((camPos.x / tileSize.x) + camSize.x) >= size.x); x++)
			{
				olc::vi2d dpos = (camPos / tileSize) + olc::vi2d{ x,y };
				olc::vi2d tpos = (tileSize * olc::vi2d{ x,y }) - olc::vi2d{ camPos.x % (tileSize.x), camPos.y % (tileSize.y) };
				tiles[dpos.y][dpos.x].draw(tpos, scale);
			}
		}
		for (int g = 0; g < gates.size(); g++)
		{
			if (gates[g].pos.y >= (camPos.y / tileSize.y) && gates[g].pos.y <= ((camPos.y / tileSize.y) + camSize.y)) {
				if (gates[g].pos.x >= (camPos.x / tileSize.x) && gates[g].pos.x <= ((camPos.x / tileSize.x) + camSize.x)) {
					olc::vi2d tpos = tileSize * (gates[g].pos - (camPos / tileSize)) - olc::vi2d{ camPos.x % (tileSize.x), camPos.y % (tileSize.y) };
					gates[g].draw(tpos, 2);
				}
			}
		}
	}
};
class Map {
public:
	int index = 0;
	std::vector<Room> rooms;
	olc::PixelGameEngine* pge;
	Map() {
		index = 0;
	}
	Map(int i, std::vector<Room> r, olc::PixelGameEngine* pg) {
		index = i;
		rooms.resize(r.size());
		for (int n = 0; n < r.size(); n++)
		{
			rooms[n] = r[n];
		}
		pge = pg;
	}
	Room newRoom(olc::vi2d size, olc::PixelGameEngine* pg) {
		rooms.resize(rooms.size() + 1);
		int i = rooms.size() - 1;
		rooms[rooms.size() - 1] = Room(size, pg);
		rooms[i].tiles = rooms[i].newTileGrid(size, 0, tileSprites[0], pg);
		rooms[i].gates = rooms[i].newGates(size, 3);
		rooms[i].updateTiles();
		rooms[i].itemHandler = RoomItemHandler();
		return rooms[rooms.size() - 1];
	}
	void changeRoom(olc::vf2d& pos) {
		int g = rooms[index].gateUsed(pos);
		int gateIndex = rooms[index].gates[g].index;
		int prevIndex = index;
		//if returning to previous room
		if (gateIndex >= 0) {
			index = gateIndex;
			for (int g = 0; g < rooms[index].gates.size(); g++)
			{
				if (rooms[index].gates[g].index == prevIndex) {
					pos = olc::vf2d(tileSize * rooms[index].gates[g].pos) + olc::vf2d{ 1.0f,1.0f };
				}
			}
		}
		//else, new room
		else {
			rooms[index].gates[g].index = rooms.size();
			olc::vi2d size = { std::rand() % 10 + 10,std::rand() % 10 + 10 };
			newRoom(size, pge);
			index = rooms.size() - 1;
			rooms[index].gates[0].index = prevIndex;
			pos = olc::vf2d(tileSize * rooms[index].gates[0].pos) + olc::vf2d{ 1.0f,1.0f };
		}
	}
};
class Cursor {
private:
	olc::vf2d pos1;/*cursor position in screen*/
	olc::vf2d pos2;/*storage of cursor position in screen for selection*/
	olc::vf2d size;/*size of sprite*/
	olc::vf2d scale;
	olc::vf2d mouseSens;
	bool bHidden = false;
	olc::vi2d state;/*x controls the animation frame, y controls cursor state(idle, right clicking, left clicking, scrolling...)*/
	olc::PixelGameEngine* pge;
public:
	Texture texture[4];
	Cursor(olc::vf2d p1, olc::vf2d sz, olc::vi2d st, olc::vf2d sc, olc::vf2d ms, olc::PixelGameEngine* pg) {
		pos1 = p1;
		size = sz;
		pos2 = pos1 + size;
		state = st;
		scale = sc;
		mouseSens = ms;
		for (int t = 0; t < 4; t++)
		{
			texture[t] = Texture(olc::vf2d{ (t / 2) ? pos2.x : pos1.x,((t + 1) / 2 == 0) ? pos2.y : pos1.y }, scale * olc::vf2d{ 0.25f,0.25f }, size);
		}
		pge = pg;
	};
	void loadAll(std::string sn) {
		for (int t = 0; t < 4; t++)
		{
			texture[t].load(sn);
		}
	}
	void update(float elapsed, float time, olc::vi2d camPos) {
		if (!bHidden) {
		}
		if (pge->GetMouse(0).bPressed) {
			//run animation
		}
		else if (pge->GetMouse(0).bHeld) {
			//start updating pos2 instead of pos1
			pos2 = olc::vf2d{ 16,16 } *olc::vf2d((pge->GetMousePos() + olc::vi2d{ size / 2 }) / (scale * size / mouseSens));//update position
			for (int t = 0; t < 4; t++)
			{
				texture[t].update(scale * olc::vf2d{ (t / 2) ? pos2.x : pos1.x,((t + 1) / 2 == 1) ? pos2.y : pos1.y }, 0);
				texture[t].offset(-olc::vf2d{ float(camPos.x % int(scale.x * size.x)),float(camPos.y % int(scale.y * size.y)) });
				texture[t].transform(0, size, t / 2);
				texture[t].transform(1, size, ((t + 1) / 2 == 1));
			}
		}
		else if (pge->GetMouse(1).bPressed || pge->GetMouse(1).bHeld) {
			//idk lol
		}
		else {
			pos1 = olc::vf2d{ 16,16 } *olc::vf2d((pge->GetMousePos() + olc::vi2d{ size / 2 }) / (scale * size));//update position
			for (int t = 0; t < 4; t++)
			{
				texture[t].update(scale * olc::vf2d{ (t / 2) ? pos2.x : pos1.x,((t + 1) / 2 == 1) ? pos2.y : pos1.y }, 0);
				texture[t].offset(-olc::vf2d{ float(camPos.x % int(scale.x * size.x)),float(camPos.y % int(scale.y * size.y)) });
				texture[t].transform(0, size, t / 2);
				texture[t].transform(1, size, ((t + 1) / 2 == 1));
			}
			pos2 = pos1 + size - 0.25f * size;
			state.x = 0;
			float wave = 2 * sin(time / 16);
			for (int t = 0; t < 4; t++)
			{
				texture[t].offset(olc::vf2d{ (t / 2) ? wave : -wave,((t + 1) / 2 == 1) ? wave : -wave });
			}
		}
	}
	void draw() {
		if (!bHidden) {
			for (int t = 0; t < 4; t++)
			{
				texture[t].draw(state, pge, 0);
			}
		}
	}
};
class Player {
private:
	olc::vi2d state;/*composed of frame and direction of animation*/
	olc::vi2d dir;/*direction player is facing, x or y is 0 while the other is 1 or -1 */
	olc::vf2d scale;
	enum anims {
		idle,
		walk,
		turn
	};
	int nAnim = idle;
	float fAnimFrame = 1.0f;
	olc::PixelGameEngine* pge;
	//Hitbox* hitbox;/*implement*/
	//Inventory* inventory;/*implement*/
public:
	int nFlip = 0;
	int nPrevFlip = 0;
	olc::vf2d pos;/*player position in-game*/
	olc::vf2d vel;
	olc::vf2d size;/*size of sprite for h*/
	Texture texture[2];
	bool bLock = false;

	Player(olc::vf2d p, olc::vf2d v, olc::vf2d sz, olc::vi2d st, olc::vi2d d, olc::vf2d sc, olc::PixelGameEngine* pg) {
		pos = p;
		vel = v;
		size = sz;
		state = st;
		dir = d;
		scale = sc;
		texture[0] = Texture(pos, scale, size);
		texture[1] = Texture(pos, scale, size);
		pge = pg;
	}
	void setAnim(int a, olc::vi2d st) {
		nAnim = a;
		state = st;
	}
	void animJumpIn(float step, olc::vf2d gatePos, olc::vf2d camPos) {
		setAnim(0, { 0,1 });
		pos = tileSize * gatePos;
		texture[0].update(pos, 0);
		texture[0].offset(camPos);
		texture[0].transform(2, olc::vf2d{ 8 * sin(step), 8 * sin(step) }, 0);
		texture[0].rotate(150 + 120 * step, 0);
	}
	void animJumpOut(float step, olc::vf2d camPos) {
		setAnim(0, { 0,0 });
		texture[0].update(pos, 0);
		texture[0].offset(camPos);
		texture[0].transform(2, olc::vf2d{ 7 * cos(step), 7 * cos(step) }, 0);
		texture[0].rotate(100 + 120 * step, 0);
	}
	void update(float elapsed, float time, olc::vi2d camPos, Map& map) {
		Room& room = map.rooms[map.index];
		if (!bLock) {
			nAnim = idle;
			state.x = (state.x + ((int(time) % 15) == 0)) % 4;
			if (pge->GetKey(olc::W).bPressed || pge->GetKey(olc::W).bHeld) {
				pos.y -= vel.y * (elapsed * 100);
				dir = { 0,-1 };
				state.y = 2;
				nAnim = walk;
			}
			else if (pge->GetKey(olc::S).bPressed || pge->GetKey(olc::S).bHeld) {
				pos.y += vel.y * (elapsed * 100);
				dir = { 0,1 };
				state.y = 0;
				nAnim = walk;
			}
			if (pge->GetKey(olc::A).bPressed || pge->GetKey(olc::A).bHeld) {
				pos.x -= vel.x * (elapsed * 100);
				dir = { -1,0 };
				state.y = 1;
				nAnim = walk;
				nPrevFlip = nFlip;
				nFlip = 1;
				fAnimFrame = (nFlip == nPrevFlip) ? fAnimFrame : 0.0f;
			}
			else if (pge->GetKey(olc::D).bPressed || pge->GetKey(olc::D).bHeld) {
				pos.x += vel.x * (elapsed * 100);
				dir = { 1,0 };
				state.y = 1;
				nAnim = walk;
				nPrevFlip = nFlip;
				nFlip = 0;
				fAnimFrame = (nFlip == nPrevFlip) ? fAnimFrame : 0.0f;
			}
			if (pge->GetKey(olc::E).bPressed) {
				nCurrRoomState = playerJumpIn;
			}
			//clamp position
			pos.x = (pos.x < 0.0f) ? 0.0f : (pos.x >= tileSize.x * (room.size.x - 1)) ? float(tileSize.x * (room.size.x - 1.0f)) : pos.x;
			pos.y = (pos.y < 0.0f) ? 0.0f : (pos.y >= tileSize.y * (room.size.y - 1)) ? float(tileSize.y * (room.size.y - 1.0f)) : pos.y;

			texture[1].update(pos, nFlip);
			texture[1].transform(0, olc::vf2d{ 17 * sin(2 * fAnimFrame),17 * sin(2 * fAnimFrame) }, nFlip);
			texture[1].offset(-camPos);
			texture[0].update(pos, nFlip);
			texture[0].transform(0, olc::vf2d{ 17 * sin(2 * fAnimFrame),17 * sin(2 * fAnimFrame) }, nFlip);
			texture[0].offset(-camPos);
			fAnimFrame = (fAnimFrame < 1.0f) ? fAnimFrame + 0.05f : 1.0f;
		}
	}
	void draw(olc::vi2d camPos) {
		texture[(nAnim + 1) / 2].draw(state, pge, nFlip);
		pge->DrawString(0, 0, std::to_string(pos.x) + " " + std::to_string(pos.y));
	}
};
class Camera {
public:
	olc::vf2d pos;
	olc::vi2d size;
	olc::PixelGameEngine* pge;
	bool bLock = false;
	Camera(olc::vf2d p, olc::vi2d sz) {
		pos = p;
		size = sz;
	}
	void update(Player player, Room room) {
		if (!bLock) {
			pos.x =
				(player.pos.x < float(tileSize.x* (size.x / 2)))
				? 0
				: (player.pos.x > float(tileSize.x * (room.size.x - (size.x / 2)) - 1))
				? tileSize.x * (room.size.x - (size.x)) - 1
				: player.pos.x - tileSize.x * (size.x / 2);
			pos.y =
				(player.pos.y < float(tileSize.y* (size.y / 2)))
				? 0
				: (player.pos.y > float(tileSize.y * (room.size.y - (size.y / 2)) - 1))
				? tileSize.y * (room.size.y - (size.y)) - 1
				: player.pos.y - tileSize.y * (size.y / 2);
		}
	}
};

void loadRoomRoutine(float time, Map& map, Camera& camera, Player& player, Cursor& cursor, olc::PixelGameEngine* pge) {
	float step = 0.0f;
	int gateIndex;

	Room& room = map.rooms[map.index];
	RoomItemHandler& items = room.itemHandler;
	switch (nCurrRoomState) {
	case playerJumpIn:
		player.bLock = true;
		step = fLoading;
		//drawing routine
		room.draw(camera.pos, camera.size, 2);
		gateIndex = room.gateUsed(player.pos);
		player.animJumpIn(step, room.gates[gateIndex].pos, -camera.pos);
		player.draw(camera.pos);
		//update state
		fLoading = (fLoading < 1.03f) ? fLoading + 0.03f : 0.0f;
		nCurrRoomState += (fLoading < 0.03f);
		break;
	case screenFadeOut:
		player.bLock = true;
		step = fLoading;
		//drawing routine
		room.draw(camera.pos, camera.size, 2);
		//cursor.draw();
		pge->FillTriangle({ 0,0 }, { 0,int(2 * step * tileSize.y * nSz.y) }, { int(2 * step * tileSize.x * nSz.x),0 }, olc::BLACK);
		//update state
		fLoading = (fLoading < 1.03f) ? fLoading + 0.03f : 0.0f;
		nCurrRoomState += (fLoading < 0.03f);
		break;
	case newRoomGeneration:
		map.changeRoom(player.pos);
		//update state
		nCurrRoomState += 1;
		break;
	case screenFadeIn:
		player.bLock = true;
		step = 1 - fLoading;
		//drawing routine
		room.draw(camera.pos, camera.size, 2);
		pge->FillTriangle({ 0,0 }, { 0,int(2 * step * tileSize.y * nSz.y) }, { int(2 * step * tileSize.x * nSz.x),0 }, olc::BLACK);
		//update state
		fLoading = (fLoading < 1.03f) ? fLoading + 0.03f : 0.0f;
		nCurrRoomState += (fLoading < 0.03f);
		break;
	case playerJumpOut:
		player.bLock = true;
		step = fLoading;
		//drawing routine
		room.draw(camera.pos, camera.size, 2);
		player.animJumpOut(step, -camera.pos);
		player.draw(camera.pos);
		//update state
		fLoading = (fLoading < 1.03f) ? fLoading + 0.03f : 0.0f;
		nCurrRoomState += (fLoading < 0.03f);
		break;
	case roomLoaded:
		if (pge->GetKey(olc::Key::I).bPressed) {
			Item newItem = Item("Log", "Just a log bruh", pge->GetMousePos() + camera.pos, { 0, 0 }, 1, { 16,16 }, { 2,2 });
			items.add(pge->GetMousePos() + camera.pos, newItem, "ItemSheet.png");
		}
		items.update(-camera.pos, time);
		player.bLock = false;
		//drawing routine
		room.draw(camera.pos, camera.size, 2);
		player.draw(camera.pos);
		items.draw(pge);
		cursor.draw();
		break;
	}
}

class Basics : public olc::PixelGameEngine
{
public:
	Basics()
	{
		sAppName = "Basic()";
	}
public:
	Player player = Player(olc::vf2d{ tileSize * nSz / olc::vf2d{ 2, 2 } }, { 1,1 }, { 16.0f, 16.0f }, { 0,0 }, { 0, 0 }, { 2.0f,2.0f }, this);
	Camera camera = Camera({ 0,0 }, nSz);
	Cursor cursor = Cursor({ 0,0 }, { 16,16 }, { 0,0 }, { 2,2 }, { 0.9f,0.9f }, this);
	std::vector<Room> rooms;
	Map map;
	bool OnUserCreate() override
	{
		map.rooms.resize(0);
		map.newRoom({ 20,10 }, this);
		map = Map(0, map.rooms, this);
		player.texture[0].load("Player.png");
		player.texture[1].load("PlayerWalk.png");
		cursor.loadAll("Selection.png");
		SetPixelMode(olc::Pixel::Mode::ALPHA);
		nCurrRoomState = roomLoaded;
		return true;
	}
	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::BLACK);
		fTime = (fTime < 999) ? fTime + 1 : 0;
		//check for transitions before updating player/camera
		player.update(fElapsedTime, fTime, camera.pos, map);
		cursor.update(fElapsedTime, fTime, camera.pos);
		camera.update(player, map.rooms[map.index]);
		loadRoomRoutine(fTime, map, camera, player, cursor, this);
		DrawString(0, 0, std::to_string(map.index), olc::WHITE);
		return true;
	}
};

int main()
{
	ShowCursor(false);
	Basics demo;
	if (demo.Construct(tileSize.x * nSz.x, tileSize.y * nSz.y, 2, 2))
	{
		demo.Start();
	}
	return 0;
}