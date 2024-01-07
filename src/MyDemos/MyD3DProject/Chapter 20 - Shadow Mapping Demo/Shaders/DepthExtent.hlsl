//=============================================================================
// Generate edge map from depth map. The edge map is represented
//in min/max form, where the min and max of the far pixels are stored for edges
//This filter is describe in the book ShaderX5 Advanced Rendering Techniques section 4.4
//=============================================================================

Texture2D gInput            : register(t0);//input a shadow map
RWTexture2D<float4> gOutput : register(u0);//write depth extent filtered result


[numthreads(16, 16, 1)]
void DepthExtentCS(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // Sample the pixels in the neighborhood of this pixel.
	/*float4 c[3][3];
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			int2 xy = dispatchThreadID.xy + int2(-1 + j, -1 + i);
			c[i][j] = gInput[xy]; 
		}
	}*/

	// For each color channel, estimate partial x derivative using Sobel scheme.
	//float4 Gx = -1.0f*c[0][0] - 2.0f*c[1][0] - 1.0f*c[2][0] + 1.0f*c[0][2] + 2.0f*c[1][2] + 1.0f*c[2][2];

	// For each color channel, estimate partial y derivative using Sobel scheme.
	//float4 Gy = -1.0f*c[2][0] - 2.0f*c[2][1] - 1.0f*c[2][1] + 1.0f*c[0][0] + 2.0f*c[0][1] + 1.0f*c[0][2];

	// Gradient is (Gx, Gy).  For each color channel, compute magnitude to get maximum rate of change.
	//float4 mag = sqrt(Gx*Gx + Gy*Gy);

	// Make edges black, and nonedges white.
	//mag = 1.0f - saturate(CalcLuminance(mag.rgb));

	float2 neighborOffset[4] =
	{ 
		{1.0, 0.0},//right
		{-1.0, 0.0},//left
		{0.0, 1.0},//top
		{0.0, -1.0}//bottom
	};
	
	float centerTapDepth;//depth of center pixel of the filter box  
	float currTapDepth;
	float depthDiff = 0;
	float maxDepthDiff = 0;
	float furthestTapDepth;
	float furthestTapDepthMin;
	float furthestTapDepthMax;
	
	float4 outColor = { 0, 0, 0, 1 };
	
	float4 blockerColor = {0,1,0,1};//blocker is green
	float4 receiverColor = {1,0,0,1};//receiver is red
	float4 notsureColor = {0,0,1,1 };//not sure is blue
	//sample depth from shadow map
		centerTapDepth = gInput[dispatchThreadID.xy].r;
	
	furthestTapDepthMin = 1;
	furthestTapDepthMax = 0;
	
	bool edgeDetected = false;
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			int2 tapLocation = dispatchThreadID.xy + int2(-1 + j, -1 + i);
			currTapDepth = gInput[tapLocation].r;
			depthDiff = abs(centerTapDepth - currTapDepth); //depth difference between current tap and center tap
			maxDepthDiff = max(depthDiff, maxDepthDiff);
		
		//if the difference between the current and center tap are significant enough to comprise an edge
		//keep trach of min and max tap values that comprise the furthest texels that make up the edge.
		//Only the furthest texels of the edge are stored in the edge map seen as the shadow boundary that 
		//require extra filtering is only projected onto the far depth of the edge
			if (depthDiff > 0.01f)//edge detected
			{
				furthestTapDepth = max(currTapDepth, centerTapDepth);
				furthestTapDepthMin = min(furthestTapDepthMin, furthestTapDepth);
				furthestTapDepthMax = max(furthestTapDepthMax, furthestTapDepth);
				edgeDetected = true;
				if (currTapDepth < centerTapDepth)//center is receiver
				{
					gOutput[tapLocation] = blockerColor;
					gOutput[dispatchThreadID.xy] = receiverColor;
				}
				else //center is blocker
				{
					gOutput[tapLocation] = receiverColor;
					gOutput[dispatchThreadID.xy] = blockerColor;
				}
			}
			else
			{
				gOutput[tapLocation] = notsureColor;
			}
		}
	}
	
	/*if (edgeDetected)
	{
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				int2 tapLocation = dispatchThreadID.xy + int2(-1 + j, -1 + i);
				currTapDepth = gInput[tapLocation].r;
				depthDiff = abs(centerTapDepth - currTapDepth); //depth difference between current tap and center tap
				if (currTapDepth <= centerTapDepth) //closer to light than edge -> is blocker
				{
					
				}
				else if ()// receiver
				{
				}
			}
		}
	}*/
	
	/*for (int i = 0; i < 4; ++i)
	{
		currTapDepth = gInput[dispatchThreadID.xy + neighborOffset[i]].r;
		depthDiff = abs(centerTapDepth - currTapDepth); //depth difference between current tap and center tap
		maxDepthDiff = max(depthDiff, maxDepthDiff);
		
		//if the difference between the current and center tap are significant enough to comprise an edge
		//keep trach of min and max tap values that comprise the furthest texels that make up the edge.
		//Only the furthest texels of the edge are stored in the edge map seen as the shadow boundary that 
		//require extra filtering is only projected onto the far depth of the edge
		if (depthDiff > 0.1f)
		{
			furthestTapDepth = max(currTapDepth, centerTapDepth);
			furthestTapDepthMin = min(furthestTapDepthMin, furthestTapDepth);
			furthestTapDepthMax = max(furthestTapDepthMax, furthestTapDepth);
		}
	}*/
	
	//outColor.r = furthestTapDepthMin;
	//outColor.r = furthestTapDepthMax;
	//outColor.g = furthestTapDepthMax;
	
	//gOutput[dispatchThreadID.xy] = outColor;
}
