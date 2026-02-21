#ifndef MARIOEFFECT_H_INCLUDED
#define MARIOEFFECT_H_INCLUDED

const char* EFFECT_STR = R"=====(
float4x4 gProjMat; // The projection matrix (Project geometry from 3D to 2D window)
float4x4 gViewMat; // The view matrix (Our current camera view of the world)
float4x4 gWorldMat; // The world matrix (Current world orientation of our object)
texture gTexture; // The texture to use when drawing with the pixel shader

sampler TextureSampler = sampler_state
{
	Texture = (gTexture); // The texture

	MipFilter = LINEAR;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
};


struct VSIn
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 tex : TEXCOORD;
};

struct VSOut_Textured
{
	float4 pos : POSITION;
	float3 normal : TEXCOORD1;
	float2 tex : TEXCOORD0;
};

struct VSOut_Untextured
{
	float4 pos : POSITION;
	float3 normal : TEXCOORD1;
	float4 color : COLOR;
};

struct PSOut
{
	float4 color : COLOR; // The pixel color to be drawn to the screen
};


VSOut_Textured TexturedVertexShader(VSIn vIn)
{
	VSOut_Textured vOut;

	// Create the worldViewProjection matrix
	float4x4 worldViewProjMat = mul(gWorldMat, mul(gViewMat, gProjMat));

	// Put vertex position in screen space
	vOut.pos = mul(float4(vIn.pos, 1), worldViewProjMat);
	vOut.normal = vIn.normal;
	vOut.tex = vIn.tex;
	return vOut;
}

PSOut TexturedPixelShader(float2 uv : TEXCOORD)
{
	PSOut pOut;

	pOut.color = tex2D(TextureSampler, uv);
	return pOut;
}


VSOut_Untextured UntexturedVertexShader(VSIn vIn)
{
	VSOut_Untextured vOut;

	// Create the worldViewProjection matrix
	float4x4 worldViewProjMat = mul(gWorldMat, mul(gViewMat, gProjMat));

	// Put vertex position in screen space
	vOut.pos = mul(float4(vIn.pos, 1), worldViewProjMat);
	vOut.normal = vIn.normal;
	vOut.color = vIn.color;
	return vOut;
}

PSOut UntexturedPixelShader(float4 color : COLOR)
{
	PSOut pOut;

	pOut.color = color;
	return pOut;
}


technique MarioTechnique
{
	// untextured
	pass First
	{
		Lighting = TRUE; // Turn off lights
		ZEnable = TRUE; // Turn on the Z-buffer
		ZWriteEnable = TRUE; // Turn on the Z-buffer
		AlphaBlendEnable = TRUE; // Turn on alpha blending
		SrcBlend = SRCALPHA; // Set SrcBlend flag for standard alpha blending
		DestBlend = INVSRCALPHA; // Set DestBlend flag for standard alpha blending
		CullMode = CW;

		VertexShader = compile vs_2_0 UntexturedVertexShader();
		PixelShader = compile ps_2_0 UntexturedPixelShader();
	}

	// textured
	pass Second
	{
		Lighting = TRUE; // Turn off lights
		ZEnable = TRUE; // Turn on the Z-buffer
		ZWriteEnable = TRUE; // Turn on the Z-buffer
		AlphaBlendEnable = TRUE; // Turn on alpha blending
		SrcBlend = SRCALPHA; // Set SrcBlend flag for standard alpha blending
		DestBlend = INVSRCALPHA; // Set DestBlend flag for standard alpha blending
		CullMode = CW;

		Sampler[0] = (TextureSampler); // Set sampler

		VertexShader = compile vs_2_0 TexturedVertexShader();
		PixelShader = compile ps_2_0 TexturedPixelShader();
	}
})=====";

#endif // MARIOEFFECT_H_INCLUDED
