#pragma once
#include <map>
#include <string>

using namespace std;

//struct ID3D11Texture2D;

class TextureManager
{
	// Map of all textures
	//map<string, int> pTextures;
public:
	TextureManager();
	~TextureManager();
	void AddTexture();
};

