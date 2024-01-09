//***************************************************************************************
// ShadowDebug.hlsl by Frank Luna (C) 2015 All Rights Reserved.
//***************************************************************************************

// Include common HLSL code.
#include "Common.hlsl"

struct VertexIn
{
	float3 PosL    : POSITION;
	float2 TexC    : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float2 TexC    : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout = (VertexOut)0.0f;

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	//vout.PosH = posW.xyz;
	vout.PosH = posW;
    // Already in homogeneous clip space.
    //vout.PosH = float4(vin.PosL, 1.0f);
	
	vout.TexC = vin.TexC;
	
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	//float4 sample = gDebugTex.Sample(gsamLinearWrap, pin.TexC);
	uint2 sample = gDebugTex[pin.TexC];
	/*if (sample.g < 1)
		sample.g = 0;
	else
		sample.g = 1;*/
	return float4(sample.ggg, 1.0f);
	//return float4(gDebugTex.Sample(gsamLinearClamp, pin.TexC).www, 1.0f);
}


