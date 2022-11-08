#include "BoxDemo.h"
#include "d3d12.h"

using namespace Microsoft::WRL;
BoxDemo::BoxDemo(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"Box Demo";
}

BoxDemo::~BoxDemo()
{

}

bool BoxDemo::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	BuildResources4ConstantBuffer();
	BuildResources4GeometryData();
	BuildRootSignature();
	CompileShaders();
	BuildPSO();

	//in the build functions above we use some command list call so 
	// now we have to execute the commands queue.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete
	//using Fence to synchronize between CPU and GPU
	//this is a simple solution but CPU has to be idle while waiting for GPU
	FlushCommandQueue();

	return true;
}

void BoxDemo::OnResize()
{
	D3DApp::OnResize();
}

void BoxDemo::Update(const GameTimer& gt)
{

}

void BoxDemo::Draw(const GameTimer& gt)
{
	//// Reuse the memory associated with command recording.
	//// We can only reset when the associated command lists have finished
	//// execution on the GPU.
	//ThrowIfFailed(mDirectCmdListAlloc->Reset());
	//// A command list can be reset after it has been added to the
	//// command queue via ExecuteCommandList. Reusing the command list reuses memory.
	//ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));// Indicate a state transition on the resource usage.

	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
	//	CurrentBackBuffer(),
	//	D3D12_RESOURCE_STATE_PRESENT,
	//	D3D12_RESOURCE_STATE_RENDER_TARGET));

	//// Set the viewport and scissor rect. This needs to be reset
	//// whenever the command list is reset.
	//mCommandList->RSSetViewports(1, &mScreenViewport);
	//mCommandList->RSSetScissorRects(1, &mScissorRect);

	//// Clear the back buffer and depth buffer.
	//mCommandList->ClearRenderTargetView(
	//	CurrentBackBufferView(),
	//	Colors::DarkOliveGreen, 0, nullptr);

	//mCommandList->ClearDepthStencilView(
	//	DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH |
	//	D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	//// Specify the buffers we are going to render to.
	//mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	//// Indicate a state transition on the resource usage.
	//mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
	//	CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
	//	D3D12_RESOURCE_STATE_PRESENT));

	//// Done recording commands.
	//ThrowIfFailed(mCommandList->Close());

	//// Add the command list to the queue for execution.
	//ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	//mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	//// swap the back and front buffers
	//ThrowIfFailed(mSwapChain->Present(0, 0));
	//mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	//// Wait until frame commands are complete. This waiting is
	//// inefficient and is done for simplicity. Later we will show how to
	//// organize our rendering code so we do not have to wait per frame.
	//FlushCommandQueue();
}

void BoxDemo::BuildResources4GeometryData()
{
	//input layout later will be included in Pipeline State Object
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	//vertex data
	std::array<Vertex, 8> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green) }),
		Vertex({ XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan) }),
		Vertex({ XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta) })
	};

	//indice data
	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	//size of vertex data in byte
	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	//size of indice data in byte
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	//geometry helper that hold resources needed for the box data
	mBoxGeo = std::make_unique<MeshGeometry>();
	mBoxGeo->Name = "boxGeo";

	// copy data for future CPU access??
	ThrowIfFailed(D3DCreateBlob(vbByteSize, &mBoxGeo->VertexBufferCPU));
	//CopyMemory is a macro of memcpy
	CopyMemory(mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);
	ThrowIfFailed(D3DCreateBlob(ibByteSize, &mBoxGeo->IndexBufferCPU));
	CopyMemory(mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	//create resources on GPU needed to hold the vertex and indice data
	mBoxGeo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, mBoxGeo->VertexBufferUploader);
	mBoxGeo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, mBoxGeo->IndexBufferUploader);

	//vertex and index data format
	mBoxGeo->VertexByteStride = sizeof(Vertex);
	mBoxGeo->VertexBufferByteSize = vbByteSize;
	mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;// 16-bit (2 byte) unsigned-integer format.
	mBoxGeo->IndexBufferByteSize = ibByteSize;

	//submesh data in case we concatenate multiple object data into 1 buffer
	// but in this demo we draw just 1 object
	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	mBoxGeo->DrawArgs["box"] = submesh;
}

void BoxDemo::BuildResources4ConstantBuffer()
{
	//first we need a heap that can hold cb view 
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; //shader can read value from CB
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&mCbvHeap)));

	//then we need to create a resource that will contain constant buffer data of 1 ObjectConstants
	mCBUploadHelper = std::make_unique<UploadBuffer<ObjectConstants>>(md3dDevice.Get(), 1, true);

	/* ==then we need to describe that resourceand add that description to the heap==*/
	//UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = mCBUploadHelper->Resource()->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer, in this case is 0 because the buffer contains just 1 object
	int cbIndex = 0;
	cbAddress += UINT64(cbIndex * mCBUploadHelper->GetElementByteSize());

	//fill in constant buffer view desc and bind it to desc heap
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = mCBUploadHelper->GetElementByteSize();
	md3dDevice->CreateConstantBufferView(
		&cbvDesc,
		mCbvHeap->GetCPUDescriptorHandleForHeapStart());
	/*==*/
}

void BoxDemo::BuildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.

	// Root parameter can be a table, root descriptor or root constants.
	//in this box demo, shader require only one constant buffer so we need just 1 root parameter
	//of type D3D12_DESCRIPTOR_RANGE_TYPE_CBV
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	//the third parameter 0 mean register(b0) in shader code below:
	/*cbuffer cbPerObject : register(b0)
	{
		float4x4 gWorldViewProj;
	};*/
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	//D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT indicate to use mInputLayout??
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	//mRootSignature will be used later when we create Pipeline State Object
	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void BoxDemo::CompileShaders()
{
	//in this demo we use run-time  compiling method
	//another method is offline-compiling using FXC tool
	//if offline-compiling is used, the compiled file has to be loaded using d3dUtil::LoadBinary
	mvsByteCode = d3dUtil::CompileShader(L"..\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"..\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
}

void BoxDemo::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	//quick way to initialize all struct members to zero
	//because the struct D3D12_GRAPHICS_PIPELINE_STATE_DESC doesn't have default constructor
	//don't know if this is lazy or good practice???
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	//config input layout
	psoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	//config root signature
	psoDesc.pRootSignature = mRootSignature.Get();

	//config VS
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mvsByteCode->GetBufferPointer()),
		mvsByteCode->GetBufferSize()
	};

	//config FS
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mpsByteCode->GetBufferPointer()),
		mpsByteCode->GetBufferSize()
	};

	//basically default settings
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = mBackBufferFormat;
	psoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = mDepthStencilFormat;

	//create PSO
	//PSO will be use when we draw object
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&mPSO)));
}
