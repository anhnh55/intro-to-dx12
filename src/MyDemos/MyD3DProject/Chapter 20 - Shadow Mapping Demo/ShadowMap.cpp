//***************************************************************************************
// ShadowMap.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "ShadowMap.h"
 
ShadowMap::ShadowMap(ID3D12Device* device, UINT width, UINT height)
{
	md3dDevice = device;

	mWidth = width;
	mHeight = height;

	mViewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	mScissorRect = { 0, 0, (int)width, (int)height };

	BuildResource();
}

UINT ShadowMap::Width()const
{
    return mWidth;
}

UINT ShadowMap::Height()const
{
    return mHeight;
}

DXGI_FORMAT ShadowMap::Format()
{
	return mFormat;
}

ID3D12Resource*  ShadowMap::Resource()
{
	return mShadowMap.Get();
}

CD3DX12_GPU_DESCRIPTOR_HANDLE ShadowMap::Srv()const
{
	return mhGpuSrv;
}

CD3DX12_CPU_DESCRIPTOR_HANDLE ShadowMap::Dsv()const
{
	return mhCpuDsv;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE ShadowMap::DsvGpu() const
{
	return mhGpuDsv;
}

D3D12_VIEWPORT ShadowMap::Viewport()const
{
	return mViewport;
}

D3D12_RECT ShadowMap::ScissorRect()const
{
	return mScissorRect;
}

void ShadowMap::BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
	                             CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
	                             CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv,
	                             CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDsv)
{
	// Save references to the descriptors. 
	mhCpuSrv = hCpuSrv;
	mhGpuSrv = hGpuSrv;
    mhCpuDsv = hCpuDsv;
    mhGpuDsv = hGpuDsv;

	//  Create the descriptors
	BuildDescriptors();
}

void ShadowMap::OnResize(UINT newWidth, UINT newHeight)
{
	if((mWidth != newWidth) || (mHeight != newHeight))
	{
		mWidth = newWidth;
		mHeight = newHeight;

		BuildResource();

		// New resource, so we need new descriptors to that resource.
		BuildDescriptors();
	}
}

D3D12_SHADER_RESOURCE_VIEW_DESC ShadowMap::SrvDesc()
{
	return msrvDesc;
}

void ShadowMap::BuildDescriptors()
{
    // Create SRV to resource so we can sample the shadow map in a shader program.
	msrvDesc = {};
	msrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    msrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; 
	msrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	msrvDesc.Texture2D.MostDetailedMip = 0;
	msrvDesc.Texture2D.MipLevels = 1;
	//msrvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
    msrvDesc.Texture2D.PlaneSlice = 0;
    md3dDevice->CreateShaderResourceView(mShadowMap.Get(), &msrvDesc, mhCpuSrv);

	// Create DSV to resource so we can render to the shadow map.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc; 
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.Texture2D.MipSlice = 0;
	md3dDevice->CreateDepthStencilView(mShadowMap.Get(), &dsvDesc, mhCpuDsv);
}

void ShadowMap::BuildResource()
{
	D3D12_RESOURCE_DESC texDesc;
	ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texDesc.Alignment = 0;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.DepthOrArraySize = 1;
	texDesc.MipLevels = 1;
	texDesc.Format = mFormat;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;

	ThrowIfFailed(md3dDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&texDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
		&optClear,
		IID_PPV_ARGS(&mShadowMap)));
}