#pragma once

#include "../../../Common/d3dApp.h"
#include "../../../Common/UploadBuffer.h"
#include "../../../Common/GeometryGenerator.h"
#include <DirectXPackedVector.h>
#include <DirectXMath.h>

#include "FrameResource.h"

using namespace DirectX;
using namespace PackedVector;
using Microsoft::WRL::ComPtr;

struct Data
{
	XMFLOAT3 vec; //3D vector
};

class Ex1 : public D3DApp
{
public:
	Ex1(HINSTANCE hInstance);
	~Ex1();
	
	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);

	//build root signature
	void BuildRootSignature();

	//compile VS and FS
	void CompileShaders();

	//build Pipeline State Object (PSO)
	void BuildPSOs();

	//Frameresource to optimize CPU and GPU sync
	void BuildFrameResources();

	//Build data buffers
	void BuildDataBuffers();

	void RunComputeShader();
private:
	//root signature 
	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	//Heap for SRV
	ComPtr<ID3D12DescriptorHeap> mCbvSrvUavDescriptorHeap = nullptr;
	UINT mCbvSrvDescriptorSize = 0;

	//VS byte code
	ComPtr<ID3DBlob> mvsByteCode = nullptr;

	//FS byte code
	ComPtr<ID3DBlob> mpsByteCode = nullptr;

	//vertex input layout corresponding to struct Vertex
	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	//Pipeline State Object
	ComPtr<ID3D12PipelineState> mPSO = nullptr;

	PassConstants mMainPassCB;
	//world view projection matrices
	XMFLOAT4X4 mWorld = MathHelper::Identity4x4();
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	////Camera related
	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	//in this demo, we use a directional light that will move around the unit sphere
	float mSunTheta = 1.25f * XM_PI;
	float mSunPhi = XM_PIDIV4;

	/*FrameResource related*/
	//list of FrameResource
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;

	//index and pointer to track current FrameResource
	int mCurrFrameResourceIndex = 0;
	FrameResource* mCurrFrameResource = nullptr;

	UINT mPassCbvOffset;
	bool mIsWireframe = false;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;

	//compute shader related
	const int NumDataElements = 64;// input 64 3D vector
	ComPtr<ID3D12Resource> mInputBuffer = nullptr;
	ComPtr<ID3D12Resource> mInputUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> mOutputBuffer = nullptr; //for GPU
	ComPtr<ID3D12Resource> mReadBackBuffer = nullptr; //to copy from GPU to CPU
};

