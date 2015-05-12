#pragma once
#include <winerror.h>
#include <fbxsdk.h>
#include <vector>
#include "GraphicsStructs.h"
// Reference http://www.walkerb.net/blog/dx-4/
// Reference http://www.gamedev.net/topic/653502-useful-things-you-might-want-to-know-about-fbxsdk/
// Reference http://docs.autodesk.com/FBX/2014/ENU/FBX-SDK-Documentation/index.html?url=cpp_ref/_u_v_sample_2main_8cxx-example.html,topicNumber=cpp_ref__u_v_sample_2main_8cxx_example_html8f1d53ae-3c78-4711-bae3-747bfdc5bb81

using namespace std;

FbxManager* FBXM = nullptr;
class FBXLoader
{
public:
	HRESULT LoadFXB(vector<VERTEX>& Vertecies);
	void ReadNormal(FbxMesh* pMesh, int ControlPointIndex, int VertexIndex, XMFLOAT3& pNormal);
	void ReadUV(FbxMesh* pMesh, int ControlPointIndex, int VertexIndex, XMFLOAT2& pUV);
};

// Read fbx file and load mesh
// Takes output parameter and filling it with a vector of vertecies
HRESULT FBXLoader::LoadFXB(vector<VERTEX>& Vertecies)
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
	bool bSuccess = pImporter->Initialize("ocean.fbx", -1, FBXM->GetIOSettings());
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
					int PolygonIndex = pMesh->GetPolygonVertexIndex(CurrPoly);
					int TextureIndex = pMesh->GetTextureUVIndex(CurrPoly, vertIndex);
					// get the positions of the fbx verts and fill our struct
					VERTEX tempVert;
					tempVert.Position.x = (float)pVerts[ControlPointIndex].mData[0];
					tempVert.Position.y = (float)pVerts[ControlPointIndex].mData[1];
					tempVert.Position.z = (float)pVerts[ControlPointIndex].mData[2];
					tempVert.Position.w = 1;
					tempVert.Color = XMFLOAT4(1, 1, 1, 1);

					XMFLOAT2 pUV;
					XMFLOAT3 pNormal;

					ReadUV(pMesh, CurrPoly, vertIndex, pUV);
					ReadNormal(pMesh, PolygonIndex, vertIndex, pNormal);
					tempVert.TextureCoord.x = pUV.x;
					tempVert.TextureCoord.y = 1.0f - pUV.y;
					tempVert.Normal.x = pNormal.x;
					tempVert.Normal.y = pNormal.y;
					tempVert.Normal.z = pNormal.z;

					Vertecies.push_back(tempVert);
				}
			}
		}
	}

	return S_OK;
}

void FBXLoader::ReadUV(FbxMesh* pMesh, int ControlPointIndex, int VertexIndex, XMFLOAT2& pUV)
{
	int UVIndex = pMesh->GetTextureUVIndex(ControlPointIndex, VertexIndex);
	FbxStringList lUVSetNameList;
	pMesh->GetUVSetNames(lUVSetNameList);

	for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
	{
		// Getting set name at current index
		const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
		// Getting uvelement related to the set name
		const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);
		// Is the element null
		if (!lUVElement)
			continue;
		
		FbxVector2 lUVValue = lUVElement->GetDirectArray().GetAt(UVIndex);

		pUV.x = (float)lUVValue.mData[0];
		pUV.y = (float)lUVValue.mData[1];
	}

}

void FBXLoader::ReadNormal(FbxMesh* pMesh, int ControlPointIndex, int VertexIndex, XMFLOAT3& pNormals)
{
	FbxGeometryElementNormal* Normal = pMesh->GetElementNormal(0);
	//int index = Normal->GetIndexArray().GetAt(0);
	pNormals.x = (float)Normal->GetDirectArray().GetAt(ControlPointIndex).mData[0];
	pNormals.y = (float)Normal->GetDirectArray().GetAt(ControlPointIndex).mData[1];
	pNormals.z = (float)Normal->GetDirectArray().GetAt(ControlPointIndex).mData[2];
}