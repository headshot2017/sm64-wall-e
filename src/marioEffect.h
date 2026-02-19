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


struct VSOut
{
	float4 pos : POSITION; // Position after vertex shader has executed
	float2 tex : TEXCOORD; // Texture coordinates after vertex shader has executed
};

struct PSOut
{
	float4 color : COLOR; // The pixel color to be drawn to the screen
};


VSOut TheVertexShader(float3 pos : POSITION, float2 texCoords : TEXCOORD)
{
	VSOut vOut;

	// Create the worldViewProjection matrix
	float4x4 worldViewProjMat = mul(gWorldMat, mul(gViewMat, gProjMat));

	// Put vertex position in screen space
	vOut.pos = mul(float4(pos, 1), worldViewProjMat);
	vOut.tex = texCoords; // Just pass the texture coordinates as they are
	return vOut;
}

PSOut ThePixelShader(float2 uv : TEXCOORD)
{
	PSOut pOut;

	pOut.color = tex2D(TextureSampler, uv);
	return pOut; // Return the pixel shader output
}


technique SingleTexture
{
	pass First
	{
		//Lighting = FALSE; // Turn off lights
		ZEnable = TRUE; // Turn on the Z-buffer

		Sampler[0] = (TextureSampler); // Set sampler

		VertexShader = compile vs_2_0 TheVertexShader();
		PixelShader = compile ps_2_0 ThePixelShader();
	}
})=====";

#endif // MARIOEFFECT_H_INCLUDED
