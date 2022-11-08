#pragma once

#include "../../../Common/d3dApp.h"
#include "../../../Common/UploadBuffer.h"
#include <DirectXPackedVector.h>
#include <DirectXMath.h>

using namespace DirectX;
using namespace PackedVector;
using Microsoft::WRL::ComPtr;

//vertex format
struct Vertex
{
	XMFLOAT3 POSITION;
	XMFLOAT4 COLOR;
};

//struct for constant buffer
//this CB holds WVP matrix
//constant buffer will be updated by the CPU regularly
//shaders can read constant buffer value
struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = MathHelper::Identity4x4();
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

	//box geometry data and needed resources
	void BuildResources4GeometryData();

	//build resources needed for a constant buffer that hold 1 ObjectConstants
	void BuildResources4ConstantBuffer();

private:
	//constant buffer descriptor heap
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	//constant buffer upload helper
	std::unique_ptr<UploadBuffer<ObjectConstants>> mCBUploadHelper = nullptr;

	//Geometry wrapper class for geometry data
	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;
};

