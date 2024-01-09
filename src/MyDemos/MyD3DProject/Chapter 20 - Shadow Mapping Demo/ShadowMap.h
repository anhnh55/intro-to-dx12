//***************************************************************************************
// ShadowMap.h by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

#pragma once

#include "../../../Common/d3dUtil.h"

class ShadowMap
{
public:
	ShadowMap(ID3D12Device* device,
		UINT width, UINT height);
		
	ShadowMap(const ShadowMap& rhs)=delete;
	ShadowMap& operator=(const ShadowMap& rhs)=delete;
	~ShadowMap()=default;

    UINT Width()const;
    UINT Height()const;
	DXGI_FORMAT Format();
	ID3D12Resource* Resource();
	CD3DX12_GPU_DESCRIPTOR_HANDLE Srv()const;
	CD3DX12_CPU_DESCRIPTOR_HANDLE Dsv()const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE DsvGpu()const;
	CD3DX12_GPU_DESCRIPTOR_HANDLE SrvStencil()const;
	D3D12_VIEWPORT Viewport()const;
	D3D12_RECT ScissorRect()const;

	void BuildDescriptors(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv,
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDsv);

	void BuildDescriptorsStencil(
		CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuSrv,
		CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuSrv);

	void OnResize(UINT newWidth, UINT newHeight);

private:
	void BuildDescriptors();
	void BuildResource();

private:

	ID3D12Device* md3dDevice = nullptr;

	D3D12_VIEWPORT mViewport;
	D3D12_RECT mScissorRect;

	UINT mWidth = 0;
	UINT mHeight = 0;
	DXGI_FORMAT mFormat = DXGI_FORMAT_R24G8_TYPELESS;

	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuStencilSrv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuStencilSrv;
	CD3DX12_CPU_DESCRIPTOR_HANDLE mhCpuDsv;
	CD3DX12_GPU_DESCRIPTOR_HANDLE mhGpuDsv;
	
	Microsoft::WRL::ComPtr<ID3D12Resource> mShadowMap = nullptr;
};

 