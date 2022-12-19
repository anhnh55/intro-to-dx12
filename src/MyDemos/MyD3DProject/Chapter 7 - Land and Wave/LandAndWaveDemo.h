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

	//Build geometry data
	void BuildShapeGeometry();

	//initialize vertex input layout
	void BuildInputLayout();

	//build resources needed for a constant buffer that hold 1 ObjectConstants
	void BuildResources4ConstantBuffers();

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
	//constant buffer descriptor heap
	ComPtr<ID3D12DescriptorHeap> mCbvHeap = nullptr;

	//constant buffer upload helper
	std::unique_ptr<UploadBuffer<ObjectConstants>> mCBUploadHelper = nullptr;

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
	//XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	//float mTheta = -XM_PIDIV2;
	////Todo why it doesn't show anything if mPhi is initialized 0.0f
	////answer: https://stackoverflow.com/questions/7394600/y-orbit-tumbles-from-top-to-bottom-as-mousey-changes-in-processing-ide
	////because in this code camera up vector is fixed at 0,1,0 so at the pole the view matrix is not correct
	////that is why Phi is restricted in range 0 < Phi < 180
	//float mPhi = XM_PIDIV2;
	//float mRadius = 6.0f;
	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	float mTheta = 1.5f * XM_PI;
	float mPhi = 0.2f * XM_PI;
	float mRadius = 15.0f;

	//last frame mouse position
	POINT mLastMousePos;

	/*FrameResource related*/
	//list of FrameResource
	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;
	// Render items divided by PSO.
	std::vector<RenderItem*> mOpaqueRitems;
	std::vector<RenderItem*> mTransparentRitems;
	//index and pointer to track current FrameResource
	int mCurrFrameResourceIndex = 0;
	FrameResource* mCurrFrameResource = nullptr;

	UINT mPassCbvOffset;
	bool mIsWireframe = false;
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> mPSOs;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;
};

