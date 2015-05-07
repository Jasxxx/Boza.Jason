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

	// Camera Matricies
	XMFLOAT4X4 VIEWMATRIX;
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
public:
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();

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
	flag = D3D11_CREATE_DEVICE_SINGLETHREADED;

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

	HRESULT ok = pDevice->CreateTexture2D(&descDepth, NULL, &pDepthStencil);

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

#pragma endregion

#pragma region Load Buffers
	vector<VERTEX> verts;
	FBX.LoadFXB(&verts);

	D3D11_SUBRESOURCE_DATA data;
	ZeroMemory(&data, sizeof(data));
	data.pSysMem = &verts;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC vBuffer;
	ZeroMemory(&vBuffer, sizeof(vBuffer));
	vBuffer.Usage = D3D11_USAGE_DYNAMIC;
	vBuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vBuffer.ByteWidth = sizeof(VERTEX) * verts.size() * 3;

	pDevice->CreateBuffer(&vBuffer, &data, &VertexBufferMap);
#pragma endregion

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

	float darkBlue[4] = {0.0f,0.0f,0.0f,1.0f};
	pDeviceContext->ClearRenderTargetView(pBackBuffer, darkBlue);

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

