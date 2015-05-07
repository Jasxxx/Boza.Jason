#pragma once
#include <winerror.h>
#include <fbxsdk.h>
#include <vector>
#include "GraphicsStructs.h"
// Tutorial http://www.walkerb.net/blog/dx-4/

using namespace std;

FbxManager* FBXM = nullptr;
class FBXLoader
{
public:
HRESULT LoadFXB(vector<VERTEX>* Vertecies);
};

// Read fbx file and load mesh
// Takes output parameter and filling it with a vector of vertecies
HRESULT FBXLoader::LoadFXB(vector<VERTEX>* Vertecies)
{
	// If not made yet create FbxManager and set the settings to what we want to load in
	if (FBXM == nullptr)
	{
		FBXM = FbxManager::Create();

		FbxIOSettings* pIOSettings = FbxIOSettings::Create(FBXM, IOSROOT);
		FBXM->SetIOSettings(pIOSettings);
	}

	FbxImporter* pImporter = FbxImporter::Create(FBXM, "");
	FbxScene* pScene = FbxScene::Create(FBXM, "");

	// Open file
	bool bSuccess = pImporter->Initialize("cone.fbx", -1, FBXM->GetIOSettings());
	if (!bSuccess)
		return E_FAIL;

	// Grab scene
	bSuccess = pImporter->Import(pScene);
	if (!bSuccess)
		return E_FAIL;

	pImporter->Destroy();

	// Set toot to the scenes root node
	FbxNode* pRoot = pScene->GetRootNode();

	if (pRoot)
	{
		// Traverse through every item if ignoring none meshes
		for (int i = 0; i < pRoot->GetChildCount(); i++)
		{
			FbxNode* pChildNode = pRoot->GetChild(i);
			if (pChildNode->GetNodeAttribute() == nullptr)
				continue;

			// Enum for what type of attribute we are accessing ie: meshes
			FbxNodeAttribute::EType AttributeType = pChildNode->GetNodeAttribute()->GetAttributeType();

			// Filtering for mesh data
			if (AttributeType != FbxNodeAttribute::eMesh)
				continue;

			// Access mesh data
			FbxMesh* pMesh = (FbxMesh*)pChildNode->GetNodeAttribute();

			// Control Points is the same as positions
			FbxVector4* pVerts = pMesh->GetControlPoints();
			//FbxVector2* pUV = pMesh->GetTextureUV();

			for (int CurrPoly = 0; CurrPoly < pMesh->GetPolygonCount(); CurrPoly++)
			{
				int NumVerts = pMesh->GetPolygonSize(CurrPoly);
				// make sure its a triangle
				assert(NumVerts == 3);

				// check position of vert
				for (int vertIndex = 0; vertIndex < NumVerts; vertIndex++)
				{
					// get the index of the polygon
					int ControlPointIndex = pMesh->GetPolygonVertex(CurrPoly, vertIndex);

					// get the positions of the fbx verts and fill our struct
					VERTEX tempVert;
					tempVert.Position.x = (float)pVerts[ControlPointIndex].mData[0];
					tempVert.Position.y = (float)pVerts[ControlPointIndex].mData[1];
					tempVert.Position.z = (float)pVerts[ControlPointIndex].mData[2];
					tempVert.Position.w = 1;
					//tempVert.Color = XMFLOAT4(1, 1, 1, 1);
					/*tempVert.Normal = XMFLOAT2(0, 0);
					tempVert.TextureCoord = XMFLOAT2(0,0);*/
					Vertecies->push_back(tempVert);
				}
			}
		}
	}
		return S_OK;
}