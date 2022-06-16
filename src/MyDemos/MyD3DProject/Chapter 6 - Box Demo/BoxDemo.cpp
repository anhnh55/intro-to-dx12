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

	//Initialize app specific resources
	//input layouts
	VertexDesc = new D3D12_INPUT_ELEMENT_DESC[2]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
	};


	//box vertices
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

	UINT64 vByteSize = vertices.size()*sizeof(Vertex);
	UINT64 iByteSize = indices.size()*sizeof(uint16_t);

	//vertex buffer step 1: create buffer
	md3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
	                                    &CD3DX12_RESOURCE_DESC::Buffer(vByteSize),
	                                    D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(vertexBufferGPU.GetAddressOf()));

	//vertex buffer step 2: create upload heap
	// In order to copy CPU memory data (vertex buffer) into our default buffer, we need
	// to create an intermediate upload heap.
	//Upload heap is required to created with D3D12_RESOURCE_STATE_GENERIC_READ
	md3dDevice->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vByteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(vertexBufferUploaderGPU.GetAddressOf()));

	// vertex buffer step 3: Describe the data we want to copy into the default buffer.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = vertices.data();
	subResourceData.RowPitch = vByteSize;
	subResourceData.SlicePitch = vByteSize;

	//vertex buffer step 4: copy CPU memory into upload heap
	//4.1: transition buffer to copy destination state
	mCommandList->ResourceBarrier(1,
								&CD3DX12_RESOURCE_BARRIER::Transition(vertexBufferGPU.Get(),
								D3D12_RESOURCE_STATE_COMMON,
								D3D12_RESOURCE_STATE_COPY_DEST));
	//4.2 update upload buffer with data
	UpdateSubresources<1>(mCommandList.Get(), vertexBufferGPU.Get(), vertexBufferUploaderGPU.Get(), 0, 0, 1, &subResourceData);

	//4.3 change vertex buffer back to normal state
	// Note: uploadBuffer has to be kept alive after that above function
	// calls because the command list has not been executed yet that
	// performs the actual copy.
	// The caller can Release the uploadBuffer after it knows the copy
	// has been executed.
	mCommandList->ResourceBarrier(1,
								&CD3DX12_RESOURCE_BARRIER::Transition(vertexBufferGPU.Get(),
								D3D12_RESOURCE_STATE_COPY_DEST,
								D3D12_RESOURCE_STATE_GENERIC_READ));

	//vertex buffer step 5: bind vertex buffer to the pipeline
	//5.1: create a view to vertex buffer
	D3D12_VERTEX_BUFFER_VIEW vBV{};
	vBV.BufferLocation = vertexBufferGPU->GetGPUVirtualAddress();
	vBV.SizeInBytes = vByteSize;
	vBV.StrideInBytes = sizeof(Vertex);
	//5.2: bind vertex buffer to the pipeline using the view we just created
	//D3D12_VERTEX_BUFFER_VIEW vertexBuffers[1] = { vBV };
	//NumViews: The number of vertex buffers we are binding to the input slots.If the
	//start slot has index k and we are binding n buffers, then we are binding buffers to
	//input slots Ik, Ik + 1, E Ik + n - 1.
	D3D12_VERTEX_BUFFER_VIEW vertexBuffers[1] = { vBV }; // this step is meant to be in draw phase
	mCommandList->IASetVertexBuffers(0, 1, vertexBuffers);
	//then drawing code ID3D12CommandList::DrawInstanced, ID3D12GraphicsCommandList::IASetPrimitiveTopology  e.t.c...

	//Index buffer: similar to vertex buffer
	//this time we use utility function in d3dUtil.h but the steps are same as for vertex buffer
	indexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(), mCommandList.Get(), indices.data(), iByteSize, indexBufferUploaderGPU);
	D3D12_INDEX_BUFFER_VIEW iBV{};
	iBV.BufferLocation = indexBufferGPU->GetGPUVirtualAddress();
	iBV.SizeInBytes = iByteSize;
	iBV.Format = DXGI_FORMAT_R16_UINT;
	mCommandList->IASetIndexBuffer(&iBV);

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
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished
	// execution on the GPU.
	ThrowIfFailed(mDirectCmdListAlloc->Reset());
	// A command list can be reset after it has been added to the
	// command queue via ExecuteCommandList. Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));// Indicate a state transition on the resource usage.

	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Set the viewport and scissor rect. This needs to be reset
	// whenever the command list is reset.
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(
		CurrentBackBufferView(),
		Colors::DarkOliveGreen, 0, nullptr);

	mCommandList->ClearDepthStencilView(
		DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH |
		D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Wait until frame commands are complete. This waiting is
	// inefficient and is done for simplicity. Later we will show how to
	// organize our rendering code so we do not have to wait per frame.
	FlushCommandQueue();
}