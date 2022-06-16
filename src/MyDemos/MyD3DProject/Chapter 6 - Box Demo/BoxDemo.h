#pragma once

#include "../../../Common/d3dApp.h"
#include <DirectXPackedVector.h>
#include <DirectXMath.h>

using namespace DirectX;
using namespace PackedVector;

struct Vertex
{
	XMFLOAT3 POSITION;
	XMFLOAT4 COLOR;
};

class BoxDemo : public D3DApp
{
public:
	BoxDemo(HINSTANCE hInstance);
	~BoxDemo();
	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	D3D12_INPUT_ELEMENT_DESC* VertexDesc;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferGPU;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferGPU;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBufferUploaderGPU;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBufferUploaderGPU;

};

