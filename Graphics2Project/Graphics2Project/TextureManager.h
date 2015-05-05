#pragma once
#include <map>

using namespace std;

struct ID3D11Texture2D;

class TextureManager
{
	// Map of all textures
	map<string, ID3D11Texture2D*> pTextures;
public:
	TextureManager();
	~TextureManager();
	void AddTexture();
};

