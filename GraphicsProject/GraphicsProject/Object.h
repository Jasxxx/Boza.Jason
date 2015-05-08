#pragma once
#include <DirectXMath.h>
#include <vector>
#include "GraphicsStructs.h"

using namespace std;
using namespace DirectX;

class Object
{
private:
	XMFLOAT3 mPosition;
	vector<VERTEX>* mGeometry;
	vector<unsigned int> mIndecies;
	string mTextureName;
	XMFLOAT4X4 WorldMatrix;

public:
	Object();
	~Object();

	// Mutators 
	void SetPosition(XMFLOAT3 _pos){ mPosition = _pos; }
	void SetGeometry(vector<VERTEX>* _vec){ mGeometry = _vec; }
	void SetTextureName(string _name){ mTextureName = _name; }
	void SetWorldMatrix(XMFLOAT4X4 _matrix){ WorldMatrix = _matrix; }

	// Accessors
	XMFLOAT3 GetPosition(){ return mPosition; }
	vector<VERTEX>* GetGeometry(){ return mGeometry; }
	string GetTextureName(){ return mTextureName; }
	XMFLOAT4X4 SetWorldMatrix(){ return WorldMatrix; }
};

