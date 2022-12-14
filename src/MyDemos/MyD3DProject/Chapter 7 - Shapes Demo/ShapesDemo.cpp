#include "ShapesDemo.h"
#include "d3d12.h"
//number of FrameResources instances
//so CPU might be at most 3 frame ahead of GPU
const int gNumFrameResources = 3;
using namespace Microsoft::WRL;

ShapesDemo::ShapesDemo(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	mMainWndCaption = L"Box Demo";
}

ShapesDemo::~ShapesDemo()
{

}

bool ShapesDemo::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	BuildShapeGeometry();
	BuildInputLayout();
	CompileShaders();
	BuildResources4ConstantBuffer();
	BuildRootSignature();
	BuildPSO();

	//in the build functions above we use some command list call so 
	// now we have to execute the commands queue.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete
	//using Fence to synchronize between CPU and GPU
	//this is a simple solution and not recommended for optimization because
	//CPU has to be idle while waiting for GPU
	FlushCommandQueue();

	return true;
}

void ShapesDemo::OnResize()
{
	//resize swapchain buffer and depth/stencil buffer
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	//Builds a left-handed perspective projection matrix based on a field of view.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void ShapesDemo::Update(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
	//Todo: why we have this x,y,z calculation?
	//Answer: x = rsin(phi)cos(theta)https://en.wikipedia.org/wiki/File:3D_Spherical_2.svg
	//        z = rsin(phi)sin(theta) because z point into screen
	//	      y = rcos(phi) because y point up
	//	r ≥ 0,
	//	0° < phi < 180°(π rad), 
	//	0° ≤ theta < 360°(2π rad).
	//we use the mouse to moving the camera around in a spherical coord
	//sinf, cosf: just sin and cos in float
	//float x = mRadius * sinf(mPhi) * cosf(mTheta);
	//float z = mRadius * sinf(mPhi) * sinf(mTheta);
	//float y = mRadius * cosf(mPhi);

	////then make the camera look at point 0,0,0
	//// Build the view matrix from 3 perpendicular vectors
	//XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	//XMVECTOR target = XMVectorZero();
	//XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	//XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	//XMStoreFloat4x4(&mView, view);

	//XMMATRIX world = XMLoadFloat4x4(&mWorld);
	//XMMATRIX proj = XMLoadFloat4x4(&mProj);
	////compute world view projection matrix
	//XMMATRIX worldViewProj = world * view * proj;

	//// Update the constant buffer with the latest worldViewProj matrix.
	//ObjectConstants objConstants;
	////Todo: why transpose the matrix?
 //   //https://stackoverflow.com/questions/32037617/why-is-this-transpose-required-in-my-worldviewproj-matrix
	////answer: CPU: row-major matrix, HLSL: column major matrix ==> has to transpose
	//XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	//mCBUploadHelper->CopyData(0, objConstants);

	/*OnKeyboardInput(gt);
	UpdateCamera(gt);*/

	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	//Todo CH7: how can CPU being ahead of GPU if it has to wait for GPU to finish current frame resource every update?
	//answer: because when mCurrFrameResource->Fence = 0, GPU has not processed this frame so we can skip the wait and add draw commands to the frame resource (CPU being ahead of GPU)
	//when mCurrFrameResource->Fence != 0 and GPU fence < frame fence, CPU has to wait GPU process current frame resource (CPU can only be ahead of GPU for a fixed number of frame resource)
	//when mCurrFrameResource->Fence != 0 and GPU fence >= frame fence, GPU has already processed this frame so CPU can update draw commands for this frame resource (CPU being ahead of GPU)
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	//UpdateObjectCBs(gt);
	//UpdateMainPassCB(gt);
}

void ShapesDemo::Draw(const GameTimer& gt)
{
	/*==============set the setting that will be used===============*/
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	//assign CommandListAllocator and PipelineStateObject to the command list
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), mPSO.Get()));

	//set viewport and scissor rectangle
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	//current back buffer change to render target state
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	// Set all back buffer to LightSteelBlue
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	/*===================================*/

	/*========specify the resources (data) that will be used to draw========*/
	// Specify the back buffers and depth stencil buffer
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	//specify constant buffer heap
	ID3D12DescriptorHeap* descriptorHeaps[] = { mCbvHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	//specify root signature
	//Todo: why have to set Root signature again when PSO already specify it???
	//Answer: https://stackoverflow.com/questions/38535725/what-is-the-point-of-d3d12s-setgraphicsrootsignature
	//It is some kind of CPU optimization
	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	////specify vertex, index buffer
	//mCommandList->IASetVertexBuffers(0, 1, &mBoxGeo->VertexBufferView());
	//mCommandList->IASetIndexBuffer(&mBoxGeo->IndexBufferView());

	////primitive topology type, in this case is triangle list
	//mCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	////specify root descriptor table
	//mCommandList->SetGraphicsRootDescriptorTable(0, mCbvHeap->GetGPUDescriptorHandleForHeapStart());

	///*=================draw the scene===============*/
	////draw object using indices
	//mCommandList->DrawIndexedInstanced(
	//	mBoxGeo->DrawArgs["box"].IndexCount,
	//	1, 0, 0, 0);

	// Indicate a state transition on the resource usage.
	//bring back buffer to screen
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	//close command list so command queue can execute it
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	/*==============================*/


	/*================post-draw===============*/
	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	//FlushCommandQueue();
	/*================================*/
	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;
	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void ShapesDemo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	//window specific code, capture mouse in this app window even if cursor goes outside of it
	//this is for when we hold left mouse to rotate the box
	SetCapture(mhMainWnd);
}

void ShapesDemo::OnMouseUp(WPARAM btnState, int x, int y)
{
	//release mouse capture when mouse is released
	ReleaseCapture();
}

void ShapesDemo::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;
		/*mTheta += dy;
		mPhi += dx;*/

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.005 unit in the scene.
		float dx = 0.005f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.005f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 4.0f, 8.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
	//std::wstring text = L"***Mouse Pos: ";
	//text += std::to_wstring(mLastMousePos.x);
	//text += L"---";
	//text += std::to_wstring(mLastMousePos.y);
	//text += L"\n";
	//::OutputDebugStringW(text.c_str());
}

void ShapesDemo::BuildInputLayout()
{
	//input layout later will be included in Pipeline State Object
	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void ShapesDemo::BuildResources4ConstantBuffer()
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

	/* ==then we need to describe that resource and add that description to the heap==*/
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
	/*======*/
}

void ShapesDemo::BuildRootSignature()
{
	//we have two constant buffers (object CB and pass CB)
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;	//b0
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE cbvTable1; //b1
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	// Root parameter can be a descriptor table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	// Create root parameter as descriptor table CBVs.
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr,
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

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void ShapesDemo::CompileShaders()
{
	//in this demo we use run-time  compiling method
	//another method is offline-compiling using FXC tool
	//if offline-compiling is used, the compiled file has to be loaded using d3dUtil::LoadBinary
	mvsByteCode = d3dUtil::CompileShader(L"..\\Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	mpsByteCode = d3dUtil::CompileShader(L"..\\Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");
}

void ShapesDemo::BuildPSO()
{
	//Not all rendering states are encapsulated in a PSO.Some states like the viewport and
	//scissor rectangles are specified independently
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;

	//quick way to initialize all struct members to zero
	//because the struct D3D12_GRAPHICS_PIPELINE_STATE_DESC doesn't have default constructor
	//Todo don't know if this is lazy or good practice???
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

void ShapesDemo::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)mAllRitems.size()));
	}
}

void ShapesDemo::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for (auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		//Note: if data is changed NumFramesDirty will be reset to gNumFrameResources somewhere
		if (e->NumFramesDirty > 0)
		{
			//
			XMMATRIX world = XMLoadFloat4x4(&e->World);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void ShapesDemo::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);

	PassConstants mMainPassCB;
	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mEyePos;
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();

	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void ShapesDemo::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.5f, 0.5f, 1.5f, 3);
	GeometryGenerator::MeshData grid = geoGen.CreateGrid(20.0f, 30.0f, 60, 40);
	GeometryGenerator::MeshData sphere = geoGen.CreateSphere(0.5f, 20, 20);
	GeometryGenerator::MeshData cylinder = geoGen.CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20);

	//
	// We are concatenating all the geometry into one big vertex/index buffer.  So
	// define the regions in the buffer each submesh covers.
	//

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	UINT boxVertexOffset = 0;
	UINT gridVertexOffset = (UINT)box.Vertices.size();
	UINT sphereVertexOffset = gridVertexOffset + (UINT)grid.Vertices.size();
	UINT cylinderVertexOffset = sphereVertexOffset + (UINT)sphere.Vertices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	UINT boxIndexOffset = 0;
	UINT gridIndexOffset = (UINT)box.Indices32.size();
	UINT sphereIndexOffset = gridIndexOffset + (UINT)grid.Indices32.size();
	UINT cylinderIndexOffset = sphereIndexOffset + (UINT)sphere.Indices32.size();

	// Define the SubmeshGeometry that cover different 
	// regions of the vertex/index buffers.

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = boxIndexOffset;
	boxSubmesh.BaseVertexLocation = boxVertexOffset;

	SubmeshGeometry gridSubmesh;
	gridSubmesh.IndexCount = (UINT)grid.Indices32.size();
	gridSubmesh.StartIndexLocation = gridIndexOffset;
	gridSubmesh.BaseVertexLocation = gridVertexOffset;

	SubmeshGeometry sphereSubmesh;
	sphereSubmesh.IndexCount = (UINT)sphere.Indices32.size();
	sphereSubmesh.StartIndexLocation = sphereIndexOffset;
	sphereSubmesh.BaseVertexLocation = sphereVertexOffset;

	SubmeshGeometry cylinderSubmesh;
	cylinderSubmesh.IndexCount = (UINT)cylinder.Indices32.size();
	cylinderSubmesh.StartIndexLocation = cylinderIndexOffset;
	cylinderSubmesh.BaseVertexLocation = cylinderVertexOffset;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	auto totalVertexCount =
		box.Vertices.size() +
		grid.Vertices.size() +
		sphere.Vertices.size() +
		cylinder.Vertices.size();

	std::vector<Vertex> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::DarkGreen);
	}

	for (size_t i = 0; i < grid.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = grid.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::ForestGreen);
	}

	for (size_t i = 0; i < sphere.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = sphere.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::Crimson);
	}

	for (size_t i = 0; i < cylinder.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = cylinder.Vertices[i].Position;
		vertices[k].Color = XMFLOAT4(DirectX::Colors::SteelBlue);
	}

	std::vector<std::uint16_t> indices;
	indices.insert(indices.end(), std::begin(box.GetIndices16()), std::end(box.GetIndices16()));
	indices.insert(indices.end(), std::begin(grid.GetIndices16()), std::end(grid.GetIndices16()));
	indices.insert(indices.end(), std::begin(sphere.GetIndices16()), std::end(sphere.GetIndices16()));
	indices.insert(indices.end(), std::begin(cylinder.GetIndices16()), std::end(cylinder.GetIndices16()));

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "shapeGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;
	geo->DrawArgs["grid"] = gridSubmesh;
	geo->DrawArgs["sphere"] = sphereSubmesh;
	geo->DrawArgs["cylinder"] = cylinderSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}