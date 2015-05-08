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
					//FbxVector2 lUVValue;

					////the UV index depends on the reference mode
					//int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

					//lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

					////User TODO:
					////Print out the value of UV(lUVValue) or log it to a file
					//Vertecies[lVertIndex].TextureCoord.x = (float)lUVValue.mData[0];
					//Vertecies[lVertIndex].TextureCoord.y = (float)lUVValue.mData[1];

					//lPolyIndexCounter++;

					// get the index of the polygon
					int ControlPointIndex = pMesh->GetPolygonVertex(CurrPoly, vertIndex);
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

					ReadUV(pMesh, ControlPointIndex, vertIndex, pUV);

					/*tempVert.TextureCoord.x =
					tempVert.TextureCoord.y =*/


					//tempVert.Normal = (float)pVerts;
					Vertecies.push_back(tempVert);
				}
			}
#pragma region getting texture data

			//FbxStringList lUVSetNameList;
			//pMesh->GetUVSetNames(lUVSetNameList);

			////iterating over all uv sets
			//for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
			//{
			//	//get lUVSetIndex-th uv set
			//	const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
			//	const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

			//	if (!lUVElement)
			//		continue;

			//	// only support mapping mode eByPolygonVertex and eByControlPoint
			//	if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
			//		lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
			//		break;

			//	for (int i = 0; i < UVSetName; i++)
			//	{

			//	}

			//	//index array, where holds the index referenced to the uv data
			//	const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
			//	const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

			//	//iterating through the data by polygon
			//	const int lPolyCount = pMesh->GetPolygonCount();

			//	if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
			//	{
			//		for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			//		{
			//			// build the max index array that we need to pass into MakePoly
			//			const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
			//			for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
			//			{
			//				FbxVector2 lUVValue;

			//				//get the index of the current vertex in control points array
			//				int lPolyVertIndex = pMesh->GetPolygonVertex(lPolyIndex, lVertIndex);

			//				//the UV index depends on the reference mode
			//				int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

			//				lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

			//				//User TODO:
			//				//Print out the value of UV(lUVValue) or log it to a file
			//				Vertecies[lVertIndex].TextureCoord.x = (float)lUVValue.mData[0];
			//				Vertecies[lVertIndex].TextureCoord.y = (float)lUVValue.mData[1];
			//			}
			//		}
			//	}
			//	else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
			//	{
			//		int lPolyIndexCounter = 0;
			//		for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			//		{
			//			// build the max index array that we need to pass into MakePoly
			//			const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
			//			for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
			//			{
			//				if (lPolyIndexCounter < lIndexCount)
			//				{
			//					FbxVector2 lUVValue;

			//					//the UV index depends on the reference mode
			//					int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

			//					lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

			//					//User TODO:
			//					//Print out the value of UV(lUVValue) or log it to a file
			//					Vertecies[lVertIndex].TextureCoord.x = (float)lUVValue.mData[0];
			//					Vertecies[lVertIndex].TextureCoord.y = (float)lUVValue.mData[1];

			//					lPolyIndexCounter++;
			//				}
			//			}
			//		}
			//	}
			//}
#pragma endregion
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

		// only support mapping mode eByPolygonVertex and eByControlPoint
		// Current understanding is that games use these mapping modes so for now just accept it
		if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
			lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
			return;

		//index array, where holds the index referenced to the uv data
		const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
		const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

		//iterating through the data by polygon
		const int lPolyCount = pMesh->GetPolygonCount();

	}

	FbxLayerElementUV* vertexUV = pMesh->GetElementUV();
	//void LoadUVInformation(FbxMesh* pMesh)
	//{
	//	//get all UV set names
	//	FbxStringList lUVSetNameList;
	//	pMesh->GetUVSetNames(lUVSetNameList);

	//	//iterating over all uv sets
	//	for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
	//	{
	//		//get lUVSetIndex-th uv set
	//		const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
	//		const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

	//		if (!lUVElement)
	//			continue;

	//		// only support mapping mode eByPolygonVertex and eByControlPoint
	//		if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
	//			lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
	//			return;

	//		//index array, where holds the index referenced to the uv data
	//		const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
	//		const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

	//		//iterating through the data by polygon
	//		const int lPolyCount = pMesh->GetPolygonCount();

	//		if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
	//		{
	//			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
	//			{
	//				// build the max index array that we need to pass into MakePoly
	//				const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
	//				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
	//				{
	//					FbxVector2 lUVValue;

	//					//get the index of the current vertex in control points array
	//					int lPolyVertIndex = pMesh->GetPolygonVertex(lPolyIndex, lVertIndex);

	//					//the UV index depends on the reference mode
	//					int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

	//					lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

	//					//User TODO:
	//					//Print out the value of UV(lUVValue) or log it to a file
	//				}
	//			}
	//		}
	//		else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
	//		{
	//			int lPolyIndexCounter = 0;
	//			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
	//			{
	//				// build the max index array that we need to pass into MakePoly
	//				const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
	//				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
	//				{
	//					if (lPolyIndexCounter < lIndexCount)
	//					{
	//						FbxVector2 lUVValue;

	//						//the UV index depends on the reference mode
	//						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

	//						lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

	//						//User TODO:
	//						//Print out the value of UV(lUVValue) or log it to a file

	//						lPolyIndexCounter++;
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

}

void FBXLoader::ReadNormal(FbxMesh* pMesh, int ControlPointIndex, int VertexIndex, XMFLOAT3& pNormals)
{

}