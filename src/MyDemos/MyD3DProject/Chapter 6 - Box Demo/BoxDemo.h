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
	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	//box geometry data and needed resources
	void BuildResources4GeometryData();

	//build resources needed for a constant buffer that hold 1 ObjectConstants
	void BuildResources4ConstantBuffer();

	//build root signature
	void BuildRootSignature();

	//compile VS and FS
	void CompileShaders();

	//build Pipeline State Object (PSO)
	void BuildPSO();
private:
	//constant buffer descriptor heap
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	//constant buffer upload helper
	std::unique_ptr<UploadBuffer<ObjectConstants>> mCBUploadHelper = nullptr;

	//Geometry wrapper class for geometry data
	std::unique_ptr<MeshGeometry> mBoxGeo = nullptr;

	//root signature 
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	//VS byte code
	ComPtr<ID3DBlob> mvsByteCode = nullptr;

	//FS byte code
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	//vertex input layout corresponding to struct Vertex
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	//Pipeline State Object
	ComPtr<ID3D12PipelineState> mPSO = nullptr;

	//world view projection matrices
	XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	//float mTheta = 1.5f * XM_PI; //3PI/2
	//float mPhi = XM_PIDIV4;
	//float mRadius = 6.0f;

	float mTheta = -XM_PIDIV2;
	//Todo why it doesn't show anything if mPhi is initialized 0.0f
	//answer: https://stackoverflow.com/questions/7394600/y-orbit-tumbles-from-top-to-bottom-as-mousey-changes-in-processing-ide
	//because in this code camera up vector is fixed at 0,1,0 so at the pole the view matrix is not correct
	//that is why Phi is restricted in range 0 < Phi < 180
	float mPhi = XM_PIDIV2;
	float mRadius = 6.0f;

	//last frame mouse position
	POINT mLastMousePos;
};

