/*
1. Create the ID3D12Device using the D3D12CreateDevice function.
2. Create an ID3D12Fence object and query descriptors' sizes.
3. Check 4X MSAA quality level support.
4. Create the command queue, command list allocator, and main command list.
5. Describe and create the swap chain.
6. Create the descriptor heaps the application requires.
7. Resize the back buffer and create a render target view to the back buffer.
8. Create the depth / stencil buffer and its associated depth / stencil view.
9. Set the viewport and scissor rectangles.*/

#include <cassert>
#include <d3d12.h>
#include <d3dx12.h> //this is a helper header downloaded at https://github.com/microsoft/DirectX-Headers/blob/main/include/directx/d3dx12.h
#include <dxgi1_4.h>
#include <iostream>
#include <wrl/client.h>

using namespace  Microsoft::WRL;

//#pragma comment(lib,"d3dcompiler.lib")
//#pragma comment(lib, "D3D12.lib")
//#pragma comment(lib, "dxgi.lib")

ComPtr<IDXGIFactory4> mdxgiFactory;
ComPtr<ID3D12Device> md3dDevice;
ComPtr<ID3D12Fence> mFence;
ComPtr<IDXGISwapChain> mSwapChain;
ComPtr<ID3D12Resource> mDepthStencilBuffer;
UINT mRtvDescriptorSize;

UINT mDsvDescriptorSize;

UINT mCbvSrvUavDescriptorSize;

DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;;
DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
UINT m4xMsaaQuality;
HWND      mhMainWnd = nullptr;

int mClientWidth = 800;
int mClientHeight = 600;
const int SwapChainBufferCount = 2;
int mCurrBackBuffer = 0; //to keep track of index of current back buffer

int main()
{
	//1. Create the ID3D12Device using the D3D12CreateDevice function.
	// Enable the D3D12 debug layer.
	// Debug layer enable extra debugging information
#if defined(DEBUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)); // IID_PPV_ARGS: retrieve an interface pointer
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
	
	//2. Create Fence and query Descriptor size (the size of the handle increment for the given type of descriptor heap, including any necessary padding)
	md3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence));
	mRtvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//3. Check 4X MSAA Quality Support
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	md3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
									&msQualityLevels,
									sizeof(msQualityLevels));
	m4xMsaaQuality = msQualityLevels.NumQualityLevels;
	assert(m4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

	//4. Create Command Queue and Command List
	//Command Queue is a container for Command Lists
	//Command List is a container for Command Allocator
	ComPtr<ID3D12CommandQueue> mCommandQueue;
	ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;
	ComPtr<ID3D12GraphicsCommandList> mCommandList;
	//describe command queue
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	md3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue));
	md3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mDirectCmdListAlloc));
	md3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mDirectCmdListAlloc.Get(), nullptr, IID_PPV_ARGS(&mCommandList));
	//close after created
	mCommandList->Close();
	
	//5. Create Swap Chain
	mSwapChain.Reset();
	//describe swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	//describe buffer
	swapChainDesc.BufferDesc.Width = mClientWidth;
	swapChainDesc.BufferDesc.Height = mClientHeight;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = mBackBufferFormat;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//describe sample
	swapChainDesc.SampleDesc.Count = 4;
	swapChainDesc.SampleDesc.Quality = m4xMsaaQuality - 1;  //valid range is between zero and one less than quality level
	//buffer usage
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SwapChainBufferCount;
	swapChainDesc.OutputWindow = mhMainWnd;
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // D3D12 apps must use DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL or DXGI_SWAP_EFFECT_FLIP_DISCARD.
	mdxgiFactory->CreateSwapChain(mCommandQueue.Get(), &swapChainDesc, &mSwapChain); // For Direct3D 12 first param is a pointer to a direct command queue

	//6. Create descriptor heaps
	ComPtr<ID3D12DescriptorHeap> mRtvHeap;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap;
	//describe rtv heap
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;  //number of render target view (RTV) descriptor is the same with number of swap chain buffer
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;//For single-adapter operation, set this to zero.
	md3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&mRtvHeap));
	//describe dsv heap
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1; //normally we need only one depth/stencil buffer
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	md3dDevice->CreateDescriptorHeap(&dsvHeapDesc,IID_PPV_ARGS(&mDsvHeap));

	// to access the descriptors of corresponding heap : see function CurrentBackBufferView()

	//7. Create RTV for back buffer
	ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount]; //an array for swapchain buffer resources
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart()); //a handle to rtv descriptor in the heap
	if (mSwapChain != nullptr)
	{
		for (int i = 0; i < SwapChainBufferCount; ++i)
		{
			//get ith buffer in the swapchain
			mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i]));

			//Create RTV
			md3dDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle); // D3D12_RENDER_TARGET_VIEW_DESC can be nullptr because resource is typed (not typeless)

			//offset handle to next descriptor in the heap
			rtvHeapHandle.Offset(1, mRtvDescriptorSize);
		}
	}

	//8. Create Depth/Stencil Buffer and View
	//describe resource
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = mDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = 4;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	//describe clear value
	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	//create resource for depth buffer
	CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);//default type is for GPU only
	md3dDevice->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON, //resource is in default state when created
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())
	);
	//create descriptor for depth buffer
	md3dDevice->CreateDepthStencilView(mDepthStencilBuffer.Get(), nullptr, mDsvHeap->GetCPUDescriptorHandleForHeapStart());
	//transition resource to be used as a depth buffer
	CD3DX12_RESOURCE_BARRIER resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		mDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE);

	mCommandList->ResourceBarrier(1,
			&resourceBarrier);

	//9. Set the Viewport
	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(mClientWidth);//static_cast is faster than dynamic_cast but not safe (no run-time type checking)
	vp.Height = static_cast<float>(mClientHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;//The MinDepth and MaxDepth members are used to transform the depth interval[0, 1] to the depth interval[MinDepth, MaxDepth].
	mCommandList->RSSetViewports(1, &vp);

	//10. we can set scissor rectangle: pixel outside of the rectangle are culled, for example:
	/*mScissorRect = { 0, 0, mClientWidth / 2, mClientHeight / 2
	};
	mCommandList->RSSetScissorRects(1, &mScissorRect);*/
}

/*
D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const
{
	// CD3DX12 constructor to offset to the RTV of the current back buffer.
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			mRtvHeap->GetCPUDescriptorHandleForHeapStart(), //handle start
			mCurrBackBuffer, // index to offset
			mRtvDescriptorSize); // byte size of descriptor
}

D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const
{
return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
}
*/

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
