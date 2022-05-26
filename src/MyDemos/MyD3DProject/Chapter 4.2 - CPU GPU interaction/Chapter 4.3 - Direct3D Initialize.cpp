/*
1. Create the ID3D12Device using the D3D12CreateDevice function.
2. Create an ID3D12Fence object and query descriptor sizes.
3. Check 4X MSAA quality level support.
4. Create the command queue, command list allocator, and main command list.
5. Describe and create the swap chain.
6. Create the descriptor heaps the application requires.
7. Resize the back buffer and create a render target view to the back buffer.
8. Create the depth / stencil buffer and its associated depth / stencil view.
9. Set the viewport and scissor rectangles.*/

#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <wrl/client.h>

using namespace  Microsoft::WRL;

//#pragma comment(lib,"d3dcompiler.lib")
//#pragma comment(lib, "D3D12.lib")
//#pragma comment(lib, "dxgi.lib")

ComPtr<IDXGIFactory4> mdxgiFactory;
ComPtr<ID3D12Device> md3dDevice;

int main()
{
	//1. Create the ID3D12Device using the D3D12CreateDevice function.
	// Enable the D3D12 debug layer.
	// Debug layer enable extra debugging information
#if defined(DEBUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	debugController->EnableDebugLayer();
#endif

	//create a DXGIFactory
	//DXGIFactory to enable WARP (a fallback option if D3D device is failed to create)
	CreateDXGIFactory1(IID_PPV_ARGS(&mdxgiFactory));

	//Create hardware device
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&md3dDevice));

	// Fallback to WARP device.
	// WARP stands for Windows Advanced Rasterization Platform, a software adapter
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		mdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter));
		D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0, //WARP support up to feature level 11 atm
			IID_PPV_ARGS(&md3dDevice));
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
