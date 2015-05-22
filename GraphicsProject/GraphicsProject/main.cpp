// Graphics 2 Demo Project.
// Author: Jason Boza

// Reference http://www.rastertek.com/dx11tut10.html
// Reference http://www.braynzarsoft.net/index.php?p=D3D11SIMPLELIGHT#still
// Reference http://www.braynzarsoft.net/index.php?p=D3D11CUBEMAP
// Reference http://en.wikipedia.org/wiki/Sine_wave

//************************************************************
//************ INCLUDES & DEFINES ****************************
//************************************************************
#pragma once
#include <map>
#include<ctime>
#include "XTime.h"
#include "TextureManager.h"
#include <d3d11.h>
#include <string>
#include <DirectXMath.h>
#include "FBXLoader.h"
#include "General_VS.csh"
#include "General_PS.csh"
#include "Trivial_VS.csh"
#include "Trivial_PS.csh"
#include "Sky_VS.csh"
#include "Sky_PS.csh"
#include "DDSTextureLoader.h"

#pragma comment(lib, "d3d11.lib")

using namespace DirectX;
using namespace std;

#define BACKBUFFER_WIDTH	1280	
#define BACKBUFFER_HEIGHT	720

//************************************************************
//************ SIMPLE WINDOWS APP CLASS **********************
//************************************************************

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;

	// FBX file loader interface
	FBXLoader FBX;
	vector<VERTEX> verts;
	// Camera Matricies
	XMFLOAT4X4 VIEWMATRIX;
	XMFLOAT4X4 WORLDMATRIX;
	XMFLOAT4X4 PROJECTIONMATRIX;
	// Sky Matrix
	XMMATRIX SKYMATRIX;
	XMMATRIX Scale;

	// Texture class for loading and unloading of textures
	TextureManager TM;
	ID3D11Resource* pTexture;

	// Direct 3D interface declarations
	IDXGISwapChain* pSwapChain;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	ID3D11RenderTargetView* pBackBuffer;
	ID3D11InputLayout* pInputLayout;
	ID3D11Texture2D* pDepthStencil;
	ID3D11DepthStencilView* pDSV;
	ID3D11BlendState* pBlendState;
	ID3D11DepthStencilState* pDepthStateLessEqual;

	// Shader Resource
	ID3D11ShaderResourceView* SRV;
	ID3D11ShaderResourceView* SkySRV;
	ID3D11UnorderedAccessView* pWaveView;

	// Raster States
	ID3D11RasterizerState* pDefaultRasterState;
	ID3D11RasterizerState* pNoCullRasterState;

	// Sampler States
	ID3D11SamplerState* pTextureSamplerState;

	// Shaders
	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11PixelShader* pNonTexturedPixelShader;
	ID3D11VertexShader* pSkyVertexShader;
	ID3D11PixelShader* pSkyPixelShader;

	D3D11_VIEWPORT MAIN_VIEWPORT;

	// Buffers
	ID3D11Buffer* pConstantBuffer;
	ID3D11Buffer* pConstantLightBuffer;
	ID3D11Buffer* pIndexBuffer;
	map<string, ID3D11Buffer*> VertexBufferMap;
	ID3D11Buffer* pSphereIndexBuffer;
	ID3D11Buffer* pSphereVertBuffer;
	ID3D11Buffer* pConstantInstanceBuffer;
	int NumSphereFaces;
	ID3D11Buffer* pTesselationBuffer;
	ID3D11Buffer* pWaveBuffer;

	// instance data variables
	int OceanCount = 1;
	int instanceMul = 1;

	// Declaration of Time object for time related use
	XTime Time;

	// Camera Declarations
	XMMATRIX Transform;
	float RotateX = 0.0f;
	float RotateY = 0.0f;

	// Wave Variables
	/*float fPeak = 0.0f;
	float fAverage = 0.0f;
	vector<float> OscillationVec;*/
	float Average;

	struct SEND_TO_VRAM
	{
		XMFLOAT4X4 WORLDMATRIX;
		XMFLOAT4X4 VIEWMATRIX;
		XMFLOAT4X4 PROJECTIONMATRIX;
	};

	struct InstancedData
	{
		XMFLOAT3 position;
	};

	struct Light
	{
		XMFLOAT3 dir;
		float specularPower;
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 specular;
		XMFLOAT3 CameraPosition;
		float padding;
		XMFLOAT3 position;
		float range;
		XMFLOAT3 attenuation;
		float pad2;
	};

	struct  Wave
	{
		float Oscillation;
	};

	SEND_TO_VRAM toShader;
	Light lightToShader;
public:
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	float rotation = 0;
	float lightChange = 0.0f;
	float change = 0.001f;
	void Input();
	void UpdateCamera();
	void SetLight();
	void CreateSphere(int LatLines, int LongLines);
	void UpdateSky();
	void InitializeWave(vector<Wave>& wave);
	//void WaveMotion(ID3D11Buffer** buffer);
};

//************************************************************
//************ CREATION OF OBJECTS & RESOURCES ***************
//************************************************************

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	// ****************** BEGIN WARNING ***********************// 
	// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY! 
	application = hinst;
	appWndProc = proc;
	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"Lab 1a Line Land", WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX),
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);
	//********************* END WARNING ************************//

#pragma region D3D initialization

#pragma region SwapChain
	// Swap Chain 
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));
	scd.BufferCount = 1; // the amount of back buffers we only need one
	scd.BufferDesc.Width = BACKBUFFER_WIDTH;
	scd.BufferDesc.Height = BACKBUFFER_HEIGHT;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // using 32 bit color
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = window;
	scd.SampleDesc.Count = 1;
	scd.Windowed = true;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	D3D11_CREATE_DEVICE_FLAG flag = (D3D11_CREATE_DEVICE_FLAG)0;
	flag = D3D11_CREATE_DEVICE_DEBUG;

#if _DEBUG
	flag = D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		flag,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&pSwapChain,
		&pDevice,
		NULL,
		&pDeviceContext);

#pragma endregion

#pragma region BackBuffer, Depth Scencil, and Viewport(s)
	// BackBuffer
	ID3D11Texture2D* BackBuffer;
	pSwapChain->GetBuffer(0, _uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);

	pDevice->CreateRenderTargetView(BackBuffer, nullptr, &pBackBuffer);
	BackBuffer->Release();

	// Depth Stencil
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = scd.BufferDesc.Width;
	descDepth.Height = scd.BufferDesc.Height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	pDevice->CreateTexture2D(&descDepth, NULL, &pDepthStencil);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	pDevice->CreateDepthStencilView(pDepthStencil, &descDSV, &pDSV);

	// Skybox Depth Stencil
	D3D11_DEPTH_STENCIL_DESC DepthStateDesc;
	ZeroMemory(&DepthStateDesc, sizeof(DepthStateDesc));
	DepthStateDesc.DepthEnable = true;
	DepthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStateDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	pDevice->CreateDepthStencilState(&DepthStateDesc, &pDepthStateLessEqual);

	// Viewport(s)
	ZeroMemory(&MAIN_VIEWPORT, sizeof(D3D11_VIEWPORT));

	MAIN_VIEWPORT.TopLeftX = 0;
	MAIN_VIEWPORT.TopLeftY = 0;
	MAIN_VIEWPORT.Width = BACKBUFFER_WIDTH;
	MAIN_VIEWPORT.Height = BACKBUFFER_HEIGHT;
	MAIN_VIEWPORT.MinDepth = 0;
	MAIN_VIEWPORT.MaxDepth = 1;

#pragma endregion

#pragma region Buffers

	FBX.LoadFXB(verts);
	verts.shrink_to_fit();

	CreateSphere(10, 10);

	VERTEX Star[12];

	Star[0].Position = XMFLOAT4(0, 0, 0.2f, 1);
	Star[0].Color = XMFLOAT4(0, 1, 0, 1);
	Star[0].TextureCoord = XMFLOAT2(0.0f, 0.0f);
	for (int i = 1; i < 11; i++)
	{
		float x;
		float y;
		if (i % 2 == 0)
		{
			x = (float)cos(((i - 1) * 36)* (3.14159 / 180)) / 2;
			y = (float)sin(((i - 1) * 36)* (3.14159 / 180)) / 2;
		}
		else
		{
			x = (float)cos(((i - 1) * 36)* (3.14159 / 180));
			y = (float)sin(((i - 1) * 36)* (3.14159 / 180));
		}
		Star[i].Position = XMFLOAT4(x, y, 0, 1);
		Star[i].Color = XMFLOAT4(1, 0, 0, 1);
		Star[i].TextureCoord = XMFLOAT2(0.0f, 0.0f);
	}
	Star[11].Position = XMFLOAT4(0, 0, -0.2f, 1);
	Star[11].Color = XMFLOAT4(0, 1, 0, 1);
	Star[11].TextureCoord = XMFLOAT2(0.0f, 0.0f);

	unsigned int StarIndecies[60] =
	{
		0, 1, 2, 0, 2, 3, 0, 3, 4,
		0, 4, 5, 0, 5, 6, 0, 6, 7,
		0, 7, 8, 0, 8, 9, 0, 9, 10,
		0, 10, 1,
		11, 2, 1, 11, 3, 2, 11, 4, 3,
		11, 5, 4, 11, 6, 5, 11, 7, 6,
		11, 8, 7, 11, 9, 8, 11, 10, 9,
		11, 1, 10
	};

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = StarIndecies;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC iBuffer;
	ZeroMemory(&iBuffer, sizeof(iBuffer));
	iBuffer.Usage = D3D11_USAGE_IMMUTABLE;
	iBuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
	iBuffer.CPUAccessFlags = NULL;
	iBuffer.ByteWidth = sizeof(unsigned int) * ARRAYSIZE(StarIndecies);

	pDevice->CreateBuffer(&iBuffer, &indexData, &pIndexBuffer);

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = &verts[0];
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC vBuffer;
	ZeroMemory(&vBuffer, sizeof(vBuffer));
	vBuffer.Usage = D3D11_USAGE_DYNAMIC;
	vBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vBuffer.ByteWidth = sizeof(VERTEX) * verts.size();

	pDevice->CreateBuffer(&vBuffer, &data, &VertexBufferMap["ocean"]);

	D3D11_SUBRESOURCE_DATA data2;
	ZeroMemory(&data2, sizeof(data2));
	data2.pSysMem = &Star;
	data2.SysMemPitch = 0;
	data2.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC v2Buffer;
	ZeroMemory(&v2Buffer, sizeof(v2Buffer));
	v2Buffer.Usage = D3D11_USAGE_IMMUTABLE;
	v2Buffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	v2Buffer.CPUAccessFlags = NULL;
	v2Buffer.ByteWidth = sizeof(VERTEX) * ARRAYSIZE(Star);

	pDevice->CreateBuffer(&v2Buffer, &data2, &VertexBufferMap["star"]);

	// Constant Buffer
	D3D11_BUFFER_DESC cBufferData;
	ZeroMemory(&cBufferData, sizeof(cBufferData));
	cBufferData.ByteWidth = sizeof(SEND_TO_VRAM);
	cBufferData.Usage = D3D11_USAGE_DYNAMIC;
	cBufferData.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferData.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA cData;
	ZeroMemory(&cData, sizeof(cData));
	cData.pSysMem = &toShader;
	pDevice->CreateBuffer(&cBufferData, &cData, &pConstantBuffer);

	float s = sizeof(Light);
	//ZeroMemory(&pConstantLightBuffer, sizeof(pConstantLightBuffer));
	D3D11_BUFFER_DESC cLightBuffer;
	ZeroMemory(&cLightBuffer, sizeof(cLightBuffer));
	cLightBuffer.ByteWidth = sizeof(Light);
	cLightBuffer.Usage = D3D11_USAGE_DYNAMIC;
	cLightBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cLightBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA cLightData;
	ZeroMemory(&cLightData, sizeof(cLightData));
	cLightData.pSysMem = &lightToShader;
	pDevice->CreateBuffer(&cLightBuffer, &cLightData, &pConstantLightBuffer);

	// instance buffer
	//OceanCount = (OceanCount - 1);
	vector<InstancedData> instVec(OceanCount);
	XMVECTOR tempPos;
	tempPos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMStoreFloat3(&instVec[0].position, tempPos);


	instanceMul = 1;
	int placeCounter = 0;
	int testVar = 8;
	for (int i = 1; i < OceanCount; i++)
	{
		int temp = i - 1;
		temp = temp % 8;
		float tempValue = 4.0f * instanceMul;
		switch (temp)
		{
		case 0:
		{
			tempPos = XMVectorSet(-tempValue, 0.0f, -tempValue, 0.0f);
			break;
		}
		case 1:
		{
			tempPos = XMVectorSet(0.0f, 0.0f, -tempValue, 0.0f);
			break;
		}
		case 2:
		{
			tempPos = XMVectorSet(tempValue, 0.0f, -tempValue, 0.0f);
			break;
		}
		case 3:
		{
			tempPos = XMVectorSet(-tempValue, 0.0f, 0.0f, 0.0f);
			break;
		}
		case 4:
		{
			tempPos = XMVectorSet(tempValue, 0.0f, 0.0f, 0.0f);
			break;
		}
		case 5:
		{
			tempPos = XMVectorSet(-tempValue, 0.0f, tempValue, 0.0f);
			break;
		}
		case 6:
		{
			tempPos = XMVectorSet(0.0f, 0.0f, tempValue, 0.0f);
			break;
		}
		case 7:
		{
			tempPos = XMVectorSet(tempValue, 0.0f, tempValue, 0.0f);
			break;
		}
		default:
			break;
		}

		XMStoreFloat3(&instVec[i].position, tempPos);

		placeCounter++;
		if (placeCounter == 8 * instanceMul)
		{
			instanceMul++;
			placeCounter = 0;
		}
	}

	D3D11_BUFFER_DESC instanceBufferDesc;
	ZeroMemory(&instanceBufferDesc, sizeof(instanceBufferDesc));
	instanceBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	instanceBufferDesc.ByteWidth = sizeof(instVec) * OceanCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = 0;
	instanceBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA instanceData;
	ZeroMemory(&instanceData, sizeof(instanceData));
	instanceData.pSysMem = &instVec[0];
	pDevice->CreateBuffer(&instanceBufferDesc, &instanceData, &pConstantInstanceBuffer);

#pragma region Tesselation Buffer

	//D3D11_BUFFER_DESC TessBufferData;
	//ZeroMemory(&TessBufferData, sizeof(TessBufferData));
	//TessBufferData.ByteWidth = sizeof(SEND_TO_VRAM);
	//TessBufferData.Usage = D3D11_USAGE_DYNAMIC;
	//TessBufferData.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	//TessBufferData.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	//D3D11_SUBRESOURCE_DATA TessData;
	//ZeroMemory(&TessData, sizeof(TessData));
	//TessData.pSysMem = &toShader;
	//pDevice->CreateBuffer(&TessBufferData, &TessData, &pTesselationBuffer);

#pragma endregion

#pragma region Compute Buffer

	vector<Wave> toCompute;
	InitializeWave(toCompute);
	
	D3D11_BUFFER_DESC ComputeDesc;
	ZeroMemory(&ComputeDesc, sizeof(ComputeDesc));
	ComputeDesc.Usage = D3D11_USAGE_DEFAULT;
	ComputeDesc.ByteWidth = sizeof(Wave) * toCompute.size();
	ComputeDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;

	D3D11_SUBRESOURCE_DATA ComputeData;
	ZeroMemory(&ComputeData, sizeof(ComputeData));
	ComputeData.pSysMem = &toCompute[0];

	pDevice->CreateBuffer(&ComputeDesc, &ComputeData, &pWaveBuffer);

	//pDevice->CreateUnorderedAccessView(&pWaveBuffer,)

#pragma endregion

#pragma endregion

#pragma region Shaders
	pDevice->CreateVertexShader(Trivial_VS,
		sizeof(Trivial_VS),
		NULL,
		&pVertexShader);
	pDevice->CreatePixelShader(Trivial_PS,
		sizeof(Trivial_PS),
		NULL,
		&pPixelShader);
	pDevice->CreateVertexShader(Sky_VS,
		sizeof(Sky_VS),
		NULL,
		&pSkyVertexShader);
	pDevice->CreatePixelShader(Sky_PS,
		sizeof(Sky_PS),
		NULL,
		&pSkyPixelShader);
	pDevice->CreatePixelShader(General_PS,
		sizeof(General_PS),
		NULL,
		&pNonTexturedPixelShader);
#pragma endregion

#pragma region Layouts
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	};

	int elements = sizeof(vLayout) / sizeof(vLayout[0]);
	pDevice->CreateInputLayout(vLayout,
		elements,
		Trivial_VS,
		sizeof(Trivial_VS),
		&pInputLayout);
#pragma endregion

#pragma region Matricies
	XMMATRIX temp;

	temp = XMMatrixIdentity();
	XMStoreFloat4x4(&WORLDMATRIX, temp);

	XMFLOAT4 tempVec = XMFLOAT4(0, 1.0f, -5.0f, 1);
	XMVECTOR pos = XMLoadFloat4(&tempVec);

	tempVec = XMFLOAT4(0, 0, 1, 1);
	XMVECTOR focus = XMLoadFloat4(&tempVec);

	tempVec = XMFLOAT4(0, 1, 0, 1);
	XMVECTOR upDir = XMLoadFloat4(&tempVec);

	temp = XMMatrixLookToLH(pos, focus, upDir);
	XMStoreFloat4x4(&VIEWMATRIX, temp);

	temp = XMMatrixPerspectiveFovLH(65 * (XM_PI / 180), (float)(BACKBUFFER_WIDTH / (float)BACKBUFFER_HEIGHT), 0.1f, 100.0f);
	XMStoreFloat4x4(&PROJECTIONMATRIX, temp);

	// Camera use 
	Transform = XMMatrixIdentity();
#pragma endregion

#pragma region RasterStates

	D3D11_RASTERIZER_DESC DefaultRasterDesc;
	ZeroMemory(&DefaultRasterDesc, sizeof(DefaultRasterDesc));
	DefaultRasterDesc.AntialiasedLineEnable = FALSE;
	DefaultRasterDesc.CullMode = D3D11_CULL_BACK;
	DefaultRasterDesc.FillMode = D3D11_FILL_SOLID;
	pDevice->CreateRasterizerState(&DefaultRasterDesc, &pDefaultRasterState);

	D3D11_RASTERIZER_DESC NoCullRasterDesc;
	ZeroMemory(&NoCullRasterDesc, sizeof(NoCullRasterDesc));
	NoCullRasterDesc.AntialiasedLineEnable = FALSE;
	NoCullRasterDesc.CullMode = D3D11_CULL_NONE;
	NoCullRasterDesc.FillMode = D3D11_FILL_SOLID;
	pDevice->CreateRasterizerState(&NoCullRasterDesc, &pNoCullRasterState);

#pragma endregion

#pragma region Textures
	CreateDDSTextureFromFile(pDevice, L"txt_001_diff.dds", NULL, &SRV);
	CreateDDSTextureFromFile(pDevice, L"SkyBox.dds", NULL, &SkySRV);

	// D3D11_RESOURCE_MISC_TEXTURECUBE;


	D3D11_SAMPLER_DESC sData;
	ZeroMemory(&sData, sizeof(sData));
	sData.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sData.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sData.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sData.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sData.MaxLOD = D3D11_FLOAT32_MAX;
	sData.MinLOD = -D3D11_FLOAT32_MAX;

	pDevice->CreateSamplerState(&sData, &pTextureSamplerState);
#pragma endregion

#pragma endregion

	Time.Restart();
	//InitializeWave();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	Time.Signal();
	SetLight();
	Input();
	UpdateSky();

	pDeviceContext->OMSetRenderTargets(1, &pBackBuffer, pDSV);
	pDeviceContext->RSSetViewports(1, &MAIN_VIEWPORT);

	float darkBlue[4] = { 0.0f, 0.0f, 0.50f, 1.0f };
	pDeviceContext->ClearRenderTargetView(pBackBuffer, darkBlue);
	pDeviceContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	ID3D11Buffer* vertInstBuffers[2] = { VertexBufferMap["ocean"], pConstantInstanceBuffer };
	unsigned int strideArr[2] = { sizeof(VERTEX), sizeof(InstancedData) };
	unsigned int offsetArr[2] = { 0, 0 };
	//WaveMotion(&vertInstBuffers[0]);

	unsigned int stride = sizeof(VERTEX);
	unsigned int offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 2, vertInstBuffers, strideArr, offsetArr);
	pDeviceContext->IASetInputLayout(pInputLayout);
	pDeviceContext->VSSetShader(pVertexShader, NULL, 0);
	pDeviceContext->PSSetShader(pPixelShader, NULL, 0);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->RSSetState(pDefaultRasterState);
	pDeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pDeviceContext->PSSetShaderResources(0, 1, &SRV);
	pDeviceContext->PSSetSamplers(0, 1, &pTextureSamplerState);
	pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	pDeviceContext->PSSetConstantBuffers(0, 1, &pConstantLightBuffer);

	D3D11_MAPPED_SUBRESOURCE data;
	ZeroMemory(&data, sizeof(data));

	pDeviceContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
	//rotation += 1.0f *(float)Time.Delta();
	XMMATRIX temp = XMLoadFloat4x4(&WORLDMATRIX);
	//temp = XMMatrixRotationY(rotation);
	XMFLOAT4X4 newMatrix;
	XMStoreFloat4x4(&newMatrix, temp);
	toShader.WORLDMATRIX = newMatrix;

	temp = XMLoadFloat4x4(&VIEWMATRIX);
	XMMatrixInverse(NULL, temp);
	XMStoreFloat4x4(&newMatrix, temp);
	toShader.VIEWMATRIX = VIEWMATRIX;

	toShader.PROJECTIONMATRIX = PROJECTIONMATRIX;

	memcpy(data.pData, &toShader, sizeof(toShader));
	pDeviceContext->Unmap(pConstantBuffer, 0);

#pragma region map light
	pDeviceContext->Map(pConstantLightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
	memcpy(data.pData, &lightToShader, sizeof(lightToShader));
	pDeviceContext->Unmap(pConstantLightBuffer, 0);
#pragma endregion


	pDeviceContext->DrawInstanced(verts.size(), OceanCount, 0, 0);

	pDeviceContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
	//rotation += 1.0f *(float)Time.Delta();

	temp = XMMatrixTranslation(0.0f, 2.0f, 0.0f);
	XMStoreFloat4x4(&newMatrix, temp);
	toShader.WORLDMATRIX = newMatrix;

	temp = XMLoadFloat4x4(&VIEWMATRIX);
	XMMatrixInverse(NULL, temp);
	XMStoreFloat4x4(&newMatrix, temp);
	toShader.VIEWMATRIX = VIEWMATRIX;

	toShader.PROJECTIONMATRIX = PROJECTIONMATRIX;

	memcpy(data.pData, &toShader, sizeof(toShader));
	pDeviceContext->Unmap(pConstantBuffer, 0);

	pDeviceContext->PSSetShader(pNonTexturedPixelShader, NULL, 0);
	pDeviceContext->IASetVertexBuffers(0, 1, &VertexBufferMap["star"], &stride, &offset);
	pDeviceContext->DrawIndexed(60, 0, 0);

	pDeviceContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
	//rotation += 1.0f *(float)Time.Delta();

	temp = SKYMATRIX;
	XMStoreFloat4x4(&newMatrix, temp);
	toShader.WORLDMATRIX = newMatrix;

	temp = XMLoadFloat4x4(&VIEWMATRIX);
	XMMatrixInverse(NULL, temp);
	XMStoreFloat4x4(&newMatrix, temp);
	toShader.VIEWMATRIX = VIEWMATRIX;

	toShader.PROJECTIONMATRIX = PROJECTIONMATRIX;

	memcpy(data.pData, &toShader, sizeof(toShader));
	pDeviceContext->Unmap(pConstantBuffer, 0);

	pDeviceContext->IASetIndexBuffer(pSphereIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pDeviceContext->IASetVertexBuffers(0, 1, &pSphereVertBuffer, &stride, &offset);
	pDeviceContext->RSSetState(pNoCullRasterState);
	pDeviceContext->OMSetDepthStencilState(pDepthStateLessEqual, 0);
	pDeviceContext->PSSetShaderResources(0, 1, &SkySRV);
	pDeviceContext->PSSetShader(pSkyPixelShader, NULL, 0);
	pDeviceContext->VSSetShader(pSkyVertexShader, NULL, 0);
	pDeviceContext->DrawIndexed(NumSphereFaces * 3, 0, 0);

	pDeviceContext->OMSetDepthStencilState(NULL, 0);

	pSwapChain->Present(0, 0);
	return true;
}

//************************************************************
//************ DESTRUCTION ***********************************
//************************************************************

bool DEMO_APP::ShutDown()
{
	pSwapChain->Release();
	pDevice->Release();
	pDeviceContext->Release();
	SRV->Release();
	pTextureSamplerState->Release();
	pBackBuffer->Release();
	pVertexShader->Release();
	pPixelShader->Release();
	pNonTexturedPixelShader->Release();
	pInputLayout->Release();
	pDepthStencil->Release();
	pDSV->Release();
	pIndexBuffer->Release();
	pDefaultRasterState->Release();
	//pBlendState->Release();
	pConstantBuffer->Release();
	pConstantLightBuffer->Release();
	VertexBufferMap["ocean"]->Release();
	VertexBufferMap["star"]->Release();
	pSphereIndexBuffer->Release();
	pSphereVertBuffer->Release();
	pConstantInstanceBuffer->Release();
	pSkyVertexShader->Release();
	pSkyPixelShader->Release();
	SkySRV->Release();
	pNoCullRasterState->Release();
	pDepthStateLessEqual->Release();

	UnregisterClass(L"DirectXApplication", application);
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance, (WNDPROC)WndProc);
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY) : { PostQuitMessage(0); }
						break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
//********************* END WARNING ************************//

void DEMO_APP::Input()
{
	XMFLOAT4X4 temp;
	XMStoreFloat4x4(&temp, Transform);

	// forward
	if (GetAsyncKeyState('W'))
		Transform = XMMatrixTranslation(0.0f, 0.0f, -(float)Time.Delta());
	// back
	if (GetAsyncKeyState('S'))
		Transform = XMMatrixTranslation(0.0f, 0.0f, (float)Time.Delta());
	// left
	if (GetAsyncKeyState('A'))
		Transform = XMMatrixTranslation((float)Time.Delta(), 0.0f, 0.0f);
	// right
	if (GetAsyncKeyState('D'))
		Transform = XMMatrixTranslation(-(float)Time.Delta(), 0.0f, 0.0f);
	// up
	if (GetAsyncKeyState(VK_SPACE))
		Transform = XMMatrixTranslation(0.0f, -(float)Time.Delta(), 0.0f);
	// down
	if (GetAsyncKeyState(VK_SHIFT))
		Transform = XMMatrixTranslation(0.0f, (float)Time.Delta(), 0.0f);


	// look left
	if (GetAsyncKeyState(VK_LEFT))
		RotateY += (float)Time.Delta();
	// look right
	if (GetAsyncKeyState(VK_RIGHT))
		RotateY -= (float)Time.Delta();
	// look up
	if (GetAsyncKeyState(VK_UP))
		RotateX += (float)Time.Delta();
	// look down
	if (GetAsyncKeyState(VK_DOWN))
		RotateX -= (float)Time.Delta();

	/*Transform = XMLoadFloat4x4(&temp);*/
	UpdateCamera();
}

void DEMO_APP::UpdateCamera()
{
	XMMATRIX RotationMatrixX = XMMatrixRotationX(RotateX);
	XMMATRIX RotationMatrixY = XMMatrixRotationY(RotateY);

	XMMATRIX tempView = XMLoadFloat4x4(&VIEWMATRIX);

	Transform = XMMatrixMultiply(RotationMatrixX, Transform);
	Transform = XMMatrixMultiply(RotationMatrixY, Transform);
	Transform = XMMatrixMultiply(tempView, Transform);


	XMStoreFloat4x4(&VIEWMATRIX, Transform);

	Transform = XMMatrixIdentity();
	RotateX = 0.0f;
	RotateY = 0.0f;
}

void DEMO_APP::SetLight()
{

	if (lightChange >= 1.0f)
	{
		lightChange = 1.0f;
		change = -0.001f;
	}
	if (lightChange <= 0.0f)
	{
		lightChange = 0.0f;
		change = 0.001f;
	}

	lightChange += change;

	lightToShader.dir = XMFLOAT3(0.5f, -1.0f, -0.5f);
	lightToShader.ambient = XMFLOAT4(0.05f, 0.05f, 0.05f, 1.0f);
	lightToShader.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lightToShader.specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	lightToShader.specularPower = 64.0f;

	XMMATRIX tempMatrix = XMLoadFloat4x4(&VIEWMATRIX);
	XMMATRIX tempMatrix2 = XMLoadFloat4x4(&PROJECTIONMATRIX);
	tempMatrix = XMMatrixInverse(NULL, tempMatrix);
	lightToShader.CameraPosition = XMFLOAT3(tempMatrix.r[3].m128_f32[0], tempMatrix.r[3].m128_f32[1], tempMatrix.r[3].m128_f32[2]);
	tempMatrix = XMMatrixMultiply(tempMatrix, tempMatrix2);

	XMFLOAT4X4 newView;
	XMStoreFloat4x4(&newView, tempMatrix);

	lightToShader.padding = 0.0f;
}

void DEMO_APP::CreateSphere(int LatLines, int LongLines)
{
	int NumSphereVertices = ((LatLines - 2) * LongLines) + 2;
	NumSphereFaces = ((LatLines - 3)*(LongLines)* 2) + (LongLines * 2);

	float sphereYaw = 0.0f;
	float spherePitch = 0.0f;

	std::vector<VERTEX> vertices(NumSphereVertices);

	XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	vertices[0].Position.x = 0.0f;
	vertices[0].Position.y = 0.0f;
	vertices[0].Position.z = 1.0f;

	for (int i = 0; i < LatLines - 2; ++i)
	{
		spherePitch = (float)((i + 1) * (3.14f / (LatLines - 1)));
		XMMATRIX Rotationx = XMMatrixRotationX(spherePitch);
		for (int j = 0; j < LongLines; ++j)
		{
			sphereYaw = float(j * (6.28f / (LongLines)));
			XMMATRIX Rotationy = XMMatrixRotationZ(sphereYaw);
			currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
			currVertPos = XMVector3Normalize(currVertPos);
			vertices[i*LongLines + j + 1].Position.x = XMVectorGetX(currVertPos);
			vertices[i*LongLines + j + 1].Position.y = XMVectorGetY(currVertPos);
			vertices[i*LongLines + j + 1].Position.z = XMVectorGetZ(currVertPos);
		}
	}

	vertices[NumSphereVertices - 1].Position.x = 0.0f;
	vertices[NumSphereVertices - 1].Position.y = 0.0f;
	vertices[NumSphereVertices - 1].Position.z = -1.0f;


	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VERTEX) * NumSphereVertices;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexBufferData;

	ZeroMemory(&vertexBufferData, sizeof(vertexBufferData));
	vertexBufferData.pSysMem = &vertices[0];
	pDevice->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &pSphereVertBuffer);


	std::vector<unsigned int> indices(NumSphereFaces * 3);

	int k = 0;
	for (int l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = 0;
		indices[k + 1] = l + 1;
		indices[k + 2] = l + 2;
		k += 3;
	}

	indices[k] = 0;
	indices[k + 1] = LongLines;
	indices[k + 2] = 1;
	k += 3;

	for (int i = 0; i < LatLines - 3; ++i)
	{
		for (int j = 0; j < LongLines - 1; ++j)
		{
			indices[k] = i*LongLines + j + 1;
			indices[k + 1] = i*LongLines + j + 2;
			indices[k + 2] = (i + 1)*LongLines + j + 1;

			indices[k + 3] = (i + 1)*LongLines + j + 1;
			indices[k + 4] = i*LongLines + j + 2;
			indices[k + 5] = (i + 1)*LongLines + j + 2;

			k += 6; // next quad
		}

		indices[k] = (i*LongLines) + LongLines;
		indices[k + 1] = (i*LongLines) + 1;
		indices[k + 2] = ((i + 1)*LongLines) + LongLines;

		indices[k + 3] = ((i + 1)*LongLines) + LongLines;
		indices[k + 4] = (i*LongLines) + 1;
		indices[k + 5] = ((i + 1)*LongLines) + 1;

		k += 6;
	}

	for (int l = 0; l < LongLines - 1; ++l)
	{
		indices[k] = NumSphereVertices - 1;
		indices[k + 1] = (NumSphereVertices - 1) - (l + 1);
		indices[k + 2] = (NumSphereVertices - 1) - (l + 2);
		k += 3;
	}

	indices[k] = NumSphereVertices - 1;
	indices[k + 1] = (NumSphereVertices - 1) - LongLines;
	indices[k + 2] = NumSphereVertices - 2;

	D3D11_BUFFER_DESC indexBufferDesc;
	ZeroMemory(&indexBufferDesc, sizeof(indexBufferDesc));

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * NumSphereFaces * 3;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA iinitData;

	iinitData.pSysMem = &indices[0];
	pDevice->CreateBuffer(&indexBufferDesc, &iinitData, &pSphereIndexBuffer);
}

void DEMO_APP::UpdateSky()
{
	SKYMATRIX = XMMatrixIdentity();
	Scale = XMMatrixScaling(5.0f, 5.0f, 5.0f);
	XMMATRIX tempMatrix = XMLoadFloat4x4(&VIEWMATRIX);

	tempMatrix = XMMatrixInverse(NULL, tempMatrix);
	XMFLOAT3 camPos = XMFLOAT3(tempMatrix.r[3].m128_f32[0], tempMatrix.r[3].m128_f32[1], tempMatrix.r[3].m128_f32[2]);
	XMVECTOR cameraPosition = XMLoadFloat3(&camPos);

	XMMATRIX translation = XMMatrixTranslation(XMVectorGetX(cameraPosition), XMVectorGetY(cameraPosition), XMVectorGetZ(cameraPosition));
	XMMATRIX rotation = XMMatrixRotationX(-1.5f);
	SKYMATRIX = Scale * translation;
}

void DEMO_APP::InitializeWave(vector<Wave>& wave)
{
	Average = 0.0f;
	wave.resize(verts.size());

	for (unsigned int i = 0; i < verts.size(); i++)
	{
		wave[i].Oscillation = verts[i].Position.y;
	}

	for (unsigned int i = 0; i < wave.size(); i++)
	{
		Average += wave[i].Oscillation;
	}

	Average /= wave.size();

	for (unsigned int i = 0; i < verts.size(); i++)
	{
		if (wave[i].Oscillation >= Average)
		{
			wave[i].Oscillation = 0.00005f;
		}
		else
		{
			wave[i].Oscillation = -0.00005f;
		}
	}
}
//
//void DEMO_APP::WaveMotion(ID3D11Buffer** buffer)
//{
//	for (unsigned int i = 0; i < verts.size(); i++)
//	{
//		verts[i].Position.y += OscillationVec[i];
//		if (verts[i].Position.y >= fAverage + 0.01f || verts[i].Position.y <= fAverage - 0.01f)
//		{
//			OscillationVec[i] = -OscillationVec[i];
//		}
//	}
//
//
//	D3D11_MAPPED_SUBRESOURCE WaveData;
//	ZeroMemory(&WaveData, sizeof(WaveData));
//
//	pDeviceContext->Map(*buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &WaveData);
//
//	memcpy(WaveData.pData, &(verts[0]), sizeof(VERTEX) * verts.size());
//	pDeviceContext->Unmap(*buffer, 0);
//}