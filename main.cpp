#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <cinttypes>

const int DEFAULT_SIZE = 16;
const int DEFAULT_TILES_PER_MINE = 12;
const int TILE_SIZE = 32;
const float MINE_SCALAR = 0.75f;
const float FLAG_SCALAR = 0.75f;
const float TEXT_SCALAR = 0.75f;
int w;
int h;
int minen;

class Tile{
	public:
	Tile()
	: data(0),near(0)
	{
	}
	void setMine(bool val){
		if(val)data |= 0b00000001;
		else data &= 0b11111110;
	}
	void switchMine(){
		data ^= 0b00000001;
	}
	bool getMine(){
		return data&0b00000001;
	}
	void setFlag(bool val){
		if(val)data |= 0b00000010;
		else data &= 0b11111101;
	}
	void switchFlag(){
		data ^= 0b00000010;
	}
	bool getFlag(){
		return data&0b00000010;
	}
	void setShow(bool val){
		if(val)data |= 0b00000100;
		else data &= 0b11111011;
	}
	void switchShow(){
		data ^= 0b00000100;
	}
	bool getShow(){
		return data&0b00000100;
	}
	void setNear(uint8_t val){
		near = val;
	}
	void incrementNear(){
		++near;
	}
	uint8_t getNear(){
		return near;
	}
	private:
	uint8_t data;
	uint8_t near;
};

Tile ** field;

void initField(){
	field = new Tile*[h];
	std::vector<std::pair<int,int> > bombs;
	for(int i=0; i<h; ++i){
		field[i]=new Tile[w];
		for(int j=0; j<w; ++j){
			bombs.push_back(std::make_pair(i,j));
		}
	}
	random_shuffle(bombs.begin(),bombs.end());
	for(int i=0; i<minen; ++i){
		field[bombs[i].first][bombs[i].second].switchMine();
		for(int y=std::max(0,bombs[i].first-1); y<=std::min(h-1,bombs[i].first+1);++y){
			for(int x=std::max(0,bombs[i].second-1); x<=std::min(w-1,bombs[i].second+1);++x){
				field[y][x].incrementNear();
			}
		}
	}
}

void free(){
	for(int i=0; i<h; ++i){
		delete field[i];
	}
	delete field;
}

void show(int x, int y){
	if(x<0 || x>=w || y<0 || y>=h || field[y][x].getShow())return;
	field[y][x].switchShow();
	if(field[y][x].getNear()==0){
		for(int yi=y-1; yi<=y+1; ++yi){
			for(int xi=x-1; xi<=x+1; ++xi){
				show(xi,yi);
			}
		}
	}
}

int main(int argc, char ** args){
	w=(argc<2?DEFAULT_SIZE : std::stoi(args[1]));
	h=(argc<3?DEFAULT_SIZE : std::stoi(args[2]));
	minen=(argc<4?(w*h)/DEFAULT_TILES_PER_MINE : std::stoi(args[3]));
	
	//font and text loading
	sf::Font font;
	if(!font.loadFromFile("res/font.ttf")){
		std::cerr<<"Could not load font"<<std::endl;
		return 1;
	}
	sf::Text text;
	text.setFont(font);
	text.setString("0");
	text.setCharacterSize(TILE_SIZE*TEXT_SCALAR);
	sf::FloatRect rect = text.getLocalBounds();
	text.setOrigin(rect.left+rect.width/2.0f,rect.top+rect.height/2.0f);
	
	//textures and sprites loading
	sf::Texture tile_hide_texture;
	if(!tile_hide_texture.loadFromFile("res/tile_hide.png")){
		std::cerr<<"Couldn't load tile_hide.png"<<std::endl;
		return 1;
	}
	sf::Sprite tile_hide_sprite(tile_hide_texture);
	tile_hide_sprite.setScale((float)TILE_SIZE/tile_hide_texture.getSize().x, (float)TILE_SIZE/tile_hide_texture.getSize().y);
	
	sf::Texture tile_show_texture;
	if(!tile_show_texture.loadFromFile("res/tile_show.png")){
		std::cerr<<"Couldn't load tile_show.png"<<std::endl;
		return 1;
	}
	sf::Sprite tile_show_sprite(tile_show_texture);
	tile_show_sprite.setScale((float)TILE_SIZE/tile_show_texture.getSize().x, (float)TILE_SIZE/tile_show_texture.getSize().y);

	sf::Texture mine_texture;
	if(!mine_texture.loadFromFile("res/mine.png")){
		std::cerr<<"Couldn't load mine.png"<<std::endl;
		return 1;
	}
	sf::Sprite mine_sprite(mine_texture);
	mine_sprite.setScale(MINE_SCALAR*(float)TILE_SIZE/mine_texture.getSize().x, MINE_SCALAR*(float)TILE_SIZE/mine_texture.getSize().y);
	rect = mine_sprite.getLocalBounds();
	mine_sprite.setOrigin(rect.left+rect.width/2.0f,rect.top+rect.height/2.0f);
	
	sf::Texture flag_texture;
	if(!flag_texture.loadFromFile("res/flag.png")){
		std::cerr<<"Couldn't load flag.png"<<std::endl;
		return 1;
	}
	sf::Sprite flag_sprite(flag_texture);
	flag_sprite.setScale(MINE_SCALAR*(float)TILE_SIZE/flag_texture.getSize().x, MINE_SCALAR*(float)TILE_SIZE/flag_texture.getSize().y);
	rect = flag_sprite.getLocalBounds();
	flag_sprite.setOrigin(rect.left+rect.width/2.0f,rect.top+rect.height/2.0f);
	
	//window creation
	sf::RenderWindow window(sf::VideoMode(w*TILE_SIZE, h*TILE_SIZE),"Minesweeper");
	
	//minefield initialization
	initField();
	
	//game loop
	while(window.isOpen()){
		//events
		sf::Event event;
		while(window.pollEvent(event)){
			if(event.type==sf::Event::Closed){
				window.close();
			} if(event.type==sf::Event::MouseButtonPressed){
				if(event.mouseButton.button==sf::Mouse::Left){
					show(event.mouseButton.x/TILE_SIZE,event.mouseButton.y/TILE_SIZE);
				} else if(event.mouseButton.button==sf::Mouse::Right){
					field[event.mouseButton.y/TILE_SIZE][event.mouseButton.x/TILE_SIZE].switchFlag();
				}
			}
		}
		
		//draw
		window.clear();
		for(int y=0; y<h; ++y){
			for(int x=0; x<w; ++x){
				if(field[y][x].getShow()){
					tile_show_sprite.setPosition(x*TILE_SIZE,y*TILE_SIZE);
					window.draw(tile_show_sprite);
					if(field[y][x].getMine()){
						mine_sprite.setPosition(x*TILE_SIZE+TILE_SIZE/2.0f,y*TILE_SIZE+TILE_SIZE/2.0f);
						window.draw(mine_sprite);
					} else if(field[y][x].getNear()){
						text.setString(std::to_string(field[y][x].getNear()));
						text.setPosition(x*TILE_SIZE+TILE_SIZE/2.0f,y*TILE_SIZE+TILE_SIZE/2.0f);
						window.draw(text);
					}
				} else {
					tile_hide_sprite.setPosition(x*TILE_SIZE, y*TILE_SIZE);
					window.draw(tile_hide_sprite);
					if(field[y][x].getFlag()){
						flag_sprite.setPosition(x*TILE_SIZE+TILE_SIZE/2.0f,y*TILE_SIZE+TILE_SIZE/2.0f);
						window.draw(flag_sprite);
					}
				}
			}
		}
		window.display();
	}
	free();
	return 0;
}
