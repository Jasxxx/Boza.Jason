// D3D LAB 1a "Line Land".
// Author: L.Norri CD DRX, FullSail University

// Introduction:
// Welcome to the first lab of the Direct3D Graphics Programming class.
// This is the ONLY guided lab in this course! 
// Future labs will demand the habits & foundation you develop right now!  
// It is CRITICAL that you follow each and every step. ESPECIALLY THE READING!!!

// TO BEGIN: Open the word document that acompanies this lab and start from the top.

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

#pragma comment(lib, "d3d11.lib")

using namespace DirectX;
using namespace std;

#define BACKBUFFER_WIDTH	500	
#define BACKBUFFER_HEIGHT	500

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

	// Texture class for loading and unloading of textures
	TextureManager TM;

	// Direct 3D interface declarations
	IDXGISwapChain* pSwapChain;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	ID3D11ShaderResourceView* SRV;
	ID3D11SamplerState* pTextureSamplerState;
	ID3D11RenderTargetView* pBackBuffer;
	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11InputLayout* pInputLayout;
	ID3D11Texture2D* pDepthStencil;
	ID3D11DepthStencilView* pDSV;
	ID3D11RasterizerState* pDefaultRasterState;
	ID3D11BlendState* pBlendState;

	D3D11_VIEWPORT MAIN_VIEWPORT;

	// Buffers
	ID3D11Buffer* pConstantBuffer;
	ID3D11Buffer* pIndexBuffer;
	ID3D11Buffer* VertexBufferMap;
	// Declaration of Time object for time related use
	XTime Time;

	struct SEND_TO_VRAM
	{
		XMFLOAT4X4 WORLDMATRIX;
		XMFLOAT4X4 VIEWMATRIX;
		XMFLOAT4X4 PROJECTIONMATRIX;
	};

	SEND_TO_VRAM toShader;
public:
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	float rotation = 0;
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
    ZeroMemory( &wndClass, sizeof( wndClass ) );
    wndClass.cbSize         = sizeof( WNDCLASSEX );             
    wndClass.lpfnWndProc    = appWndProc;						
    wndClass.lpszClassName  = L"DirectXApplication";            
	wndClass.hInstance      = application;		               
    wndClass.hCursor        = LoadCursor( NULL, IDC_ARROW );    
    wndClass.hbrBackground  = ( HBRUSH )( COLOR_WINDOWFRAME ); 
	//wndClass.hIcon			= LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_FSICON));
    RegisterClassEx( &wndClass );

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(	L"DirectXApplication", L"Lab 1a Line Land",	WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME|WS_MAXIMIZEBOX), 
							CW_USEDEFAULT, CW_USEDEFAULT, window_size.right-window_size.left, window_size.bottom-window_size.top,					
							NULL, NULL,	application, this );												

    ShowWindow( window, SW_SHOW );
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
	flag = D3D11_CREATE_DEVICE_DEBUG ;

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
	
	FBX.LoadFXB(&verts);

	VERTEX Star[12];

	Star[0].Position = XMFLOAT4(0, 0, 0.2f, 1);
	Star[0].Color = XMFLOAT4(0, 1, 0, 1);
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
	}
	Star[11].Position = XMFLOAT4(0, 0, -0.2f, 1);
	Star[11].Color = XMFLOAT4(0, 1, 0, 1);

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
	data.pSysMem = &Star;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC vBuffer;
	ZeroMemory(&vBuffer, sizeof(vBuffer));
	vBuffer.Usage = D3D11_USAGE_IMMUTABLE;
	vBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBuffer.CPUAccessFlags = NULL;
	vBuffer.ByteWidth = sizeof(VERTEX) * ARRAYSIZE(Star);

	pDevice->CreateBuffer(&vBuffer, &data, &VertexBufferMap);

	// Constant Buffer
	D3D11_BUFFER_DESC cBufferData;
	ZeroMemory(&cBufferData, sizeof(cBufferData));
	cBufferData.ByteWidth = sizeof(SEND_TO_VRAM);
	// TODO: PART 3 STEP 4b
	cBufferData.Usage = D3D11_USAGE_DYNAMIC;
	cBufferData.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferData.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA cData;
	ZeroMemory(&cData, sizeof(cData));
	cData.pSysMem = &toShader;
	pDevice->CreateBuffer(&cBufferData, &cData, &pConstantBuffer);
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
#pragma endregion

#pragma region Layouts
	D3D11_INPUT_ELEMENT_DESC vLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		/*{ "TEXTURE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMALS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }*/
	};

	int elements = sizeof(vLayout) / sizeof(vLayout[0]);
	// TODO: PART 2 STEP 8b
	pDevice->CreateInputLayout(vLayout,
		elements,
		Trivial_VS,
		sizeof(Trivial_VS),
		&pInputLayout);
#pragma endregion

#pragma endregion

	XMMATRIX temp;

	temp = XMMatrixIdentity();
	XMStoreFloat4x4(&WORLDMATRIX, temp);
	
	XMFLOAT4 tempVec = XMFLOAT4(0,0,-2, 1);
	XMVECTOR pos = XMLoadFloat4(&tempVec);
	
	tempVec = XMFLOAT4(0, 0, 1, 1);
	XMVECTOR focus = XMLoadFloat4(&tempVec);

	tempVec = XMFLOAT4(0, 1, 0, 1);
	XMVECTOR upDir = XMLoadFloat4(&tempVec);


	temp = XMMatrixLookToLH(pos, focus, upDir);
	XMStoreFloat4x4(&VIEWMATRIX, temp);
	
	float Yscale = ((1 / tanf(65 / 2 * (3.14159265f / 180))));
	float Xscale = Yscale * (BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT);
	temp = XMMatrixPerspectiveFovLH(Yscale, Xscale, 0.1f, 100.0f);
	XMStoreFloat4x4(&PROJECTIONMATRIX, temp);

	D3D11_RASTERIZER_DESC DefaultRasterDesc;
	ZeroMemory(&DefaultRasterDesc, sizeof(DefaultRasterDesc));
	DefaultRasterDesc.AntialiasedLineEnable = FALSE;
	DefaultRasterDesc.CullMode = D3D11_CULL_BACK;
	DefaultRasterDesc.FillMode = D3D11_FILL_SOLID;
	pDevice->CreateRasterizerState(&DefaultRasterDesc, &pDefaultRasterState);


	Time.Restart();
}

//************************************************************
//************ EXECUTION *************************************
//************************************************************

bool DEMO_APP::Run()
{
	Time.Signal();
	pDeviceContext->OMSetRenderTargets(1, &pBackBuffer, pDSV);
	pDeviceContext->RSSetViewports(1, &MAIN_VIEWPORT);

	float darkBlue[4] = {0.0f,0.0f,0.50f,1.0f};
	pDeviceContext->ClearRenderTargetView(pBackBuffer, darkBlue);
	pDeviceContext->ClearDepthStencilView(pDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	unsigned int stride = sizeof(VERTEX);
	unsigned int offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &VertexBufferMap, &stride, &offset);
	pDeviceContext->IASetInputLayout(pInputLayout);
	pDeviceContext->VSSetShader(pVertexShader, NULL, 0);
	pDeviceContext->PSSetShader(pPixelShader, NULL, 0);
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pDeviceContext->VSSetConstantBuffers(0, 1, &pConstantBuffer);
	pDeviceContext->RSSetState(pDefaultRasterState);
	pDeviceContext->IASetIndexBuffer(pIndexBuffer,DXGI_FORMAT_R32_UINT,	0);

	D3D11_MAPPED_SUBRESOURCE data;
	ZeroMemory(&data, sizeof(data));

	pDeviceContext->Map(pConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
	rotation += 1.0f *(float)Time.Delta();
	XMMATRIX temp = XMLoadFloat4x4(&WORLDMATRIX);
	temp = XMMatrixRotationY(rotation);
	XMFLOAT4X4 newMatrix;
	XMStoreFloat4x4(&newMatrix, temp);
	toShader.WORLDMATRIX = newMatrix;

	toShader.VIEWMATRIX = VIEWMATRIX;

	toShader.PROJECTIONMATRIX = PROJECTIONMATRIX;

	memcpy(data.pData, &toShader, sizeof(toShader));
	pDeviceContext->Unmap(pConstantBuffer, 0);

	pDeviceContext->DrawIndexed(60, 0, 0);

	pSwapChain->Present(0,0);
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

	pBackBuffer->Release();
	pDepthStencil->Release();
	pDSV->Release();

	UnregisterClass( L"DirectXApplication", application ); 
	return true;
}

//************************************************************
//************ WINDOWS RELATED *******************************
//************************************************************

// ****************** BEGIN WARNING ***********************// 
// WINDOWS CODE, I DON'T TEACH THIS YOU MUST KNOW IT ALREADY!
	
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine,	int nCmdShow );						   
LRESULT CALLBACK WndProc(HWND hWnd,	UINT message, WPARAM wparam, LPARAM lparam );		
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance,(WNDPROC)WndProc);	
    MSG msg; ZeroMemory( &msg, sizeof( msg ) );
    while ( msg.message != WM_QUIT && myApp.Run() )
    {	
	    if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        { 
            TranslateMessage( &msg );
            DispatchMessage( &msg ); 
        }
    }
	myApp.ShutDown(); 
	return 0; 
}
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    if(GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
    switch ( message )
    {
        case ( WM_DESTROY ): { PostQuitMessage( 0 ); }
        break;
    }
    return DefWindowProc( hWnd, message, wParam, lParam );
}
//********************* END WARNING ************************//

