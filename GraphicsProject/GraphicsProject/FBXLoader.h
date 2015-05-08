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
	HRESULT LoadFXB(vector<VERTEX>& Vertecies);
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
	bool bSuccess = pImporter->Initialize("Ocean.fbx", -1, FBXM->GetIOSettings());
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
					FbxVector2 pTexts;
					bool hi;
					pMesh->GetPolygonVertexUV(CurrPoly, vertIndex, NULL, pTexts, hi);
					int TextureIndex = pMesh->GetTextureUVIndex(CurrPoly, vertIndex);
					// get the positions of the fbx verts and fill our struct
					VERTEX tempVert;
					tempVert.Position.x = (float)pVerts[ControlPointIndex].mData[0];
					tempVert.Position.y = (float)pVerts[ControlPointIndex].mData[1];
					tempVert.Position.z = (float)pVerts[ControlPointIndex].mData[2];
					tempVert.Position.w = 1;
					tempVert.Color = XMFLOAT4(1, 1, 1, 1);
					tempVert.TextureCoord.x = (float)pTexts.mData[0];
					tempVert.TextureCoord.y = (float)pTexts.mData[1];

					//tempVert.Normal = (float)pVerts;
					Vertecies.push_back(tempVert);
				}
			}
#pragma region getting texture data

			FbxStringList lUVSetNameList;
			pMesh->GetUVSetNames(lUVSetNameList);

			//iterating over all uv sets
			for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
			{
				//get lUVSetIndex-th uv set
				const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
				const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

				if (!lUVElement)
					continue;

				// only support mapping mode eByPolygonVertex and eByControlPoint
				if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
					lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
					break;

				//index array, where holds the index referenced to the uv data
				const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
				const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

				//iterating through the data by polygon
				const int lPolyCount = pMesh->GetPolygonCount();

				if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
				{
					for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
					{
						// build the max index array that we need to pass into MakePoly
						const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
						for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
						{
							FbxVector2 lUVValue;

							//get the index of the current vertex in control points array
							int lPolyVertIndex = pMesh->GetPolygonVertex(lPolyIndex, lVertIndex);

							//the UV index depends on the reference mode
							int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

							lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

							//User TODO:
							//Print out the value of UV(lUVValue) or log it to a file
							Vertecies[lVertIndex].TextureCoord.x = (float)lUVValue.mData[0];
							Vertecies[lVertIndex].TextureCoord.y = (float)lUVValue.mData[1];
						}
					}
				}
				else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					int lPolyIndexCounter = 0;
					for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
					{
						// build the max index array that we need to pass into MakePoly
						const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
						for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
						{
							if (lPolyIndexCounter < lIndexCount)
							{
								FbxVector2 lUVValue;

								//the UV index depends on the reference mode
								int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

								lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

								//User TODO:
								//Print out the value of UV(lUVValue) or log it to a file
								Vertecies[lVertIndex].TextureCoord.x = (float)lUVValue.mData[0];
								Vertecies[lVertIndex].TextureCoord.y = (float)lUVValue.mData[1];

								lPolyIndexCounter++;
							}
						}
					}
				}
			}
		}
	}

#pragma endregion
	return S_OK;
}