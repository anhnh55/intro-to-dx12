#pragma once

#include "../../../Common/d3dApp.h"
#include "../../../Common/UploadBuffer.h"
#include "../../../Common/GeometryGenerator.h"
#include <DirectXPackedVector.h>
#include <DirectXMath.h>

#include "FrameResource.h"
#include "Waves.h"

using namespace DirectX;
using namespace PackedVector;
using Microsoft::WRL::ComPtr;



// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.
struct RenderItem
{
	RenderItem() = default;

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify object data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	//Todo CH7: seems like this is unnecessary because DrawIndexedInstanced parameters are also saved below
	UINT ObjCBIndex = -1;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
	// Geometry associated with this render-item. Note that multiple
	// render-items can share the same geometry.
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

enum class RenderLayer : int
{
	Opaque = 0,
	Count
};

class LandAndWaveDemo : public D3DApp
{
public:
	LandAndWaveDemo(HINSTANCE hInstance);
	~LandAndWaveDemo();
	
	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& renderItems);
	virtual void Draw(const GameTimer& gt)override;
	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void UpdateWaves(const GameTimer& gt);

	//get height using a sin func
	float GetHillsHeight(float x, float z);

	//Build Geometry for land
	void BuildLandGeometry();

	//Build Geometry for waves
	void BuildWavesGeometryBuffers();

	//initialize vertex input layout
	void BuildInputLayout();

	//build root signature
	void BuildRootSignature();

	//compile VS and FS
	void CompileShaders();

	//build Pipeline State Object (PSO)
	void BuildPSO();

	//Frameresource to optimize CPU and GPU sync
	void BuildFrameResources();

	//update object constant buffer 
	void UpdateObjectCBs(const GameTimer& gt);
	//update frame render pass constant buffer 
	void UpdateMainPassCB(const GameTimer& gt);

	//config to draw each object
	void BuildRenderItems();
private:
	//Geometry wrapper class for geometry data
	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;

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

	////Camera related
	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	float mTheta = 1.5f * XM_PI;
	float mPhi = XM_PIDIV2 - 0.1f;
	float mRadius = 50.0f;

	//last frame mouse position
	POINT mLastMousePos;

	/*FrameResource related*/
	//list of FrameResource
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	// Render items divided by PSO.
	std::vector<RenderItem*> mRitemLayer[(int)RenderLayer::Count];

	//Wave item
	RenderItem* mWavesRitem = nullptr;
	//index and pointer to track current FrameResource
	int mCurrFrameResourceIndex = 0;
	FrameResource* mCurrFrameResource = nullptr;

	UINT mPassCbvOffset;
	bool mIsWireframe = false;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
	

	std::unique_ptr<Waves> mWaves;
};

