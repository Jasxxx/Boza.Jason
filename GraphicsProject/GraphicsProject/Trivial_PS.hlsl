// Specular Light Reference http://www.rastertek.com/dx11tut10.html

texture2D baseTexture : register (t0);
SamplerState filter : register(s0);

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float3 normalOut : NORMAL;
	float4 projectedCoordinate : SV_POSITION;
	float3 ViewDir : VIEW;
};

cbuffer Light : register (b0)
{
	float3 dir;
	float specularPower;
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float padding;
}

float4 main(OUTPUT_VERTEX input) : SV_TARGET
{
	float4 newSpecular = float4(0.0f,0.0f,0.0f,0.0f);

	// get texture color from uvs
	float4 TextureColor = baseTexture.Sample(filter, input.textureCoords.xy);
	// set color to the ambient color as default
	float4 color = ambient;
	// invert light direction for calculations
	float3 lightDir = -dir;
	// amount of light on the pixel
	float lightIntensity = saturate(dot(input.normalOut,lightDir));
	
	if (lightIntensity > 0.0f)
	{
		// Calculate final diffuse color by applying the light intensity to it
		color += (diffuse * lightIntensity);
		// Saturate the outcome which was the ambient and the diffuse with light intensity applied to it
		color = saturate(color);
		// Calculate reflection vector
		float3 reflection = normalize(2 * lightIntensity * input.normalOut - lightDir);
		// Calculate Specular light based off reflection, viewing , and specular power
		newSpecular = pow(saturate(dot(reflection,input.ViewDir)), specularPower);
	}

	// determine textured color
	color = color * TextureColor;
	// Add specular to the color
	color = saturate(color + newSpecular);

	//float4 newColor = diffuse * ambient;
	//newColor += saturate(dot(dir, input.normalOut) * diffuse * color);

	return color;
}