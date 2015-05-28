// Specular Light Reference http://www.rastertek.com/dx11tut10.html

texture2D baseTexture : register (t0);
SamplerState filter : register(s0);

struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float2 textureCoords : TEXTURE;
	float3 normalOut : NORMAL;
	float4 projectedCoordinate : SV_POSITION;
	float3 position : POSITION;
};

cbuffer Light : register (b0)
{
	float3 dir;
	float specularPower;
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float3 CamPos;
	float padding;
	float3 LightPos;
	float range;
	float3 att;
	float pad2;
}

cbuffer PointLight : register (b1)
{
	float3 positionPL;
	float padPL;
	float4 colorPL;
	float3 attPL;
	float pad2PL;
}

cbuffer SpotLight : register (b2)
{
	float3 positionSL;
	float coneSL;
	float3 dirSL;
	float padSL;
	float4 colorSL;
}


float4 main(OUTPUT_VERTEX input) : SV_TARGET
{
	float3 ViewDir = normalize(CamPos - input.position);

	float4 newSpecular = float4(0.0f, 0.0f, 0.0f, 1.0f);

	float3 normalizednrm = normalize(input.normalOut);
	float3 normalizedView = ViewDir;
	// get texture color from uvs
	float4 TextureColor = baseTexture.Sample(filter, input.textureCoords.xy);
	// set color to the ambient color as default
	float4 color = ambient;
	// invert light direction for calculations
	float3 lightDir = normalize(-dir);
	// amount of light on the pixel
	//float3 halfvec = normalize((lightDir)+normalizedView);
	float3 reflectvec = reflect(dir, input.normalOut);
	float lightIntensity = saturate(dot(normalize(reflectvec), ViewDir));
	float lightRatio = saturate(dot(lightDir, normalizednrm));
	float4 newDiffuse = saturate(diffuse * lightRatio * TextureColor);

		// point light
	float3 PLDir = positionPL - input.position;
	float PLDis = length(PLDir);
	PLDir = normalize(PLDir);
	float PLRatio = saturate(dot(PLDir, normalizednrm));
	float4 PLColor = PLRatio * colorPL * TextureColor;
	float att = (1.0f - saturate(PLDis / 1.0f));
	att *= att;
	PLColor *= att;
		//SpotLight
	float3 SLDir = normalize(positionSL - input.position);
	float SLRatio = saturate(dot(-SLDir, dirSL));
	float SLFactor = SLRatio > coneSL ? 1 : 0;
	float SLLightRatio = saturate(dot(SLDir, normalizednrm));
	float4 SLColor = SLFactor * SLLightRatio * colorSL * TextureColor;
	//if (lightIntensity > 0.0f)
	// Calculate final diffuse color by applying the light intensity to it
	//color += (diffuse * lightIntensity);
	// Saturate the outcome which was the ambient and the diffuse with light intensity applied to it
	//color = saturate(color);
	// Calculate reflection vector

	//float3 reflection = normalize(2 * lightIntensity * input.normalOut - lightDir);
	// Calculate Specular light based off reflection, viewing , and specular power
	newSpecular.xyz = saturate(specular.xyz * pow(lightIntensity, specularPower) * 1);


	color += saturate(newSpecular + newDiffuse + PLColor + SLColor);

	// determine textured color
	color = color * TextureColor;
	// Add specular to the color
	//color = saturate(color + newSpecular);

	//float4 newColor = diffuse * ambient;
	//newColor += saturate(dot(dir, input.normalOut) * diffuse * color);

	return color;
}