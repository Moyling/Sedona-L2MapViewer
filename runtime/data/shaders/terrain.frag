#version 330 compatibility

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D tex3;
uniform sampler2D tex4;
uniform sampler2D tex5;
uniform sampler2D tex6;
uniform sampler2D tex7;
uniform sampler2D tex8;
uniform sampler2D tex9;
uniform sampler2D tex10;
uniform sampler2D tex11;
uniform sampler2D AlphaMap1;
uniform sampler2D AlphaMap2;
uniform sampler2D AlphaMap3;

in float texAlpha0;
in float texAlpha1;
in float texAlpha2;
in float texAlpha3;
in float texAlpha4;
in float texAlpha5;
in float texAlpha6;
in float texAlpha7;
in float texAlpha8;
in float texAlpha9;
in float texAlpha10;
in float texAlpha11;

in vec2 texCoord0;
in vec2 texCoord1;
in vec2 texCoord2;
in vec2 texCoord3;
in vec2 texCoord4;
in vec2 texCoord5;
in vec2 texCoord6;
in vec2 texCoord7;
in vec2 texCoord8;
in vec2 texCoord9;
in vec2 texCoord10;
in vec2 texCoord11;
in vec2 alphaCoords;

in float OutLight;

//in vec3 viewVec;
in float viewDist;

void main()
{
	float AlphaSum = texAlpha0 + texAlpha1 + texAlpha2 + texAlpha3 + texAlpha4 + texAlpha5 + texAlpha6 + texAlpha7 + texAlpha8 + texAlpha9 + texAlpha10 + texAlpha11;
	
	float AlphaVal = 0;
	float layerAlpha = 0;
	
	/*vec4 texVal0 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 texVal1 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 texVal2 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 texVal3 = vec4(0.0, 0.0, 0.0, 0.0);
	
	vec4 texVal4 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 texVal5 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 texVal6 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 texVal7 = vec4(0.0, 0.0, 0.0, 0.0);
	
	vec4 texVal8 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 texVal9 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 texVal10 = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 texVal11 = vec4(0.0, 0.0, 0.0, 0.0);*/
	
	
	vec4 texVal11 = texture2D(tex11, texCoord11);
	layerAlpha = max(texture2D(AlphaMap3, alphaCoords).a * texVal11.a - AlphaVal, 0.0);
	texVal11.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	vec4 texVal10 = texture2D(tex10, texCoord10);
	layerAlpha = max(texture2D(AlphaMap3, alphaCoords).b * texVal10.a - AlphaVal, 0.0);
	texVal10.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	vec4 texVal9 = texture2D(tex9, texCoord9);
	layerAlpha = max(texture2D(AlphaMap3, alphaCoords).g * texVal9.a - AlphaVal, 0.0);
	texVal9.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	vec4 texVal8 = texture2D(tex8, texCoord8);
	layerAlpha = max(texture2D(AlphaMap3, alphaCoords).r * texVal8.a - AlphaVal, 0.0);
	texVal8.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	
	vec4 texVal7 = texture2D(tex7, texCoord7);
	layerAlpha = max(texture2D(AlphaMap2, alphaCoords).a * texVal7.a - AlphaVal, 0.0);
	texVal7.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	vec4 texVal6 = texture2D(tex6, texCoord6);
	layerAlpha = max(texture2D(AlphaMap2, alphaCoords).b * texVal6.a - AlphaVal, 0.0);
	texVal6.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	vec4 texVal5 = texture2D(tex5, texCoord5);
	layerAlpha = max(texture2D(AlphaMap2, alphaCoords).g * texVal5.a - AlphaVal, 0.0);
	texVal5.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	vec4 texVal4 = texture2D(tex4, texCoord4);
	layerAlpha = max(texture2D(AlphaMap2, alphaCoords).r * texVal4.a - AlphaVal, 0.0);
	texVal4.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	
	vec4 texVal3 = texture2D(tex3, texCoord3);
	layerAlpha = max(texture2D(AlphaMap1, alphaCoords).a * texVal3.a - AlphaVal, 0.0);
	texVal3.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	vec4 texVal2 = texture2D(tex2, texCoord2);
	layerAlpha = max(texture2D(AlphaMap1, alphaCoords).b * texVal2.a - AlphaVal, 0.0);
	texVal2.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	vec4 texVal1 = texture2D(tex1, texCoord1);
	layerAlpha = max(texture2D(AlphaMap1, alphaCoords).g * texVal1.a - AlphaVal, 0.0);
	texVal1.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	vec4 texVal0 = texture2D(tex0, texCoord0);
	layerAlpha = max(texture2D(AlphaMap1, alphaCoords).r * texVal0.a - AlphaVal, 0.0);
	texVal0.rgb *= layerAlpha;
	AlphaVal += layerAlpha;
	
	vec4 texSum = texVal0
								 + texVal1
								 + texVal2
								 + texVal3
								 + texVal4
								 + texVal5
								 + texVal6
								 + texVal7
								 + texVal8
								 + texVal9
								 + texVal10
								 + texVal11;
	//texSum /= AlphaSum;
	/*texSum.r = texture2D(AlphaMap3, alphaCoords).g;
	//texSum.r = texSum.a;
	texSum.g = 0;
	texSum.b = 0;*/
	//texSum = texture2D(tex1, texCoord1) * texVal1.a;//  * texture2D(AlphaMap3, alphaCoords).g
	texSum.rgb *= (OutLight * 0.8) + 0.4;
	//vec4 rcol = vec4(texAlpha6, texAlpha7, texAlpha8, 1);
	
	vec4 fogColor = vec4(0.5, 0.5, 0.55, 1.0);
	float fogPower = 0.0;
	if(viewDist >= 7000.0 && viewDist < 20000.0)
	{
		fogPower = (viewDist - 7000.0) / 13000.0;
	}
	else if(viewDist >= 20000.0)
	{
		fogPower = 1.0;
	}
	
	texSum.a = 1.0;
	
	gl_FragColor = texSum;
	//gl_FragColor = texSum * (1.0 - fogPower) + fogColor * fogPower;
}
