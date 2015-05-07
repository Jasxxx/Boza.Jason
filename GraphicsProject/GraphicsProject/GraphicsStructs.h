#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#pragma comment(lib, "d3d11.lib")

using namespace DirectX;

struct VERTEX
{
	XMFLOAT4 Position;
	XMFLOAT4 Color;
	XMFLOAT2 TextureCoord;
	XMFLOAT2 Normal;
	XMFLOAT4 padding;
};