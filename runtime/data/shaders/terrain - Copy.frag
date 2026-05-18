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
	
	vec4 texVal11 = texture2D(tex11, texCoord11) * max(texture2D(AlphaMap3, alphaCoords).a - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap3, alphaCoords).a - AlphaVal, 0.0) * texVal11.a;
	AlphaVal +=  texVal11.a;
	vec4 texVal10 = texture2D(tex10, texCoord10) * max(texture2D(AlphaMap3, alphaCoords).b - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap3, alphaCoords).b - AlphaVal, 0.0) * texVal10.a;
	AlphaVal +=  texVal10.a;
	vec4 texVal9 = texture2D(tex9, texCoord9) * max(texture2D(AlphaMap3, alphaCoords).g - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap3, alphaCoords).g - AlphaVal, 0.0) * texVal9.a;
	AlphaVal +=  texVal9.a;
	vec4 texVal8 = texture2D(tex8, texCoord8) * max(texture2D(AlphaMap3, alphaCoords).r - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap3, alphaCoords).r - AlphaVal, 0.0) * texVal8.a;
	AlphaVal +=  texVal8.a;
	
	vec4 texVal7 = texture2D(tex7, texCoord7) * max(texture2D(AlphaMap2, alphaCoords).a - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap2, alphaCoords).a - AlphaVal, 0.0) * texVal7.a;
	AlphaVal +=  texVal7.a;
	vec4 texVal6 = texture2D(tex6, texCoord6) * max(texture2D(AlphaMap2, alphaCoords).b - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap2, alphaCoords).b - AlphaVal, 0.0) * texVal6.a;
	AlphaVal +=  texVal6.a;
	vec4 texVal5 = texture2D(tex5, texCoord5) * max(texture2D(AlphaMap2, alphaCoords).g - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap2, alphaCoords).g - AlphaVal, 0.0) * texVal5.a;
	AlphaVal +=  texVal5.a;
	vec4 texVal4 = texture2D(tex4, texCoord4) * max(texture2D(AlphaMap2, alphaCoords).r - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap2, alphaCoords).r - AlphaVal, 0.0) * texVal4.a;
	AlphaVal +=  texVal4.a;
	
	vec4 texVal3 = texture2D(tex3, texCoord3) * max(texture2D(AlphaMap1, alphaCoords).a - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap1, alphaCoords).a - AlphaVal, 0.0) * texVal3.a;
	AlphaVal +=  texVal3.a;
	vec4 texVal2 = texture2D(tex2, texCoord2) * max(texture2D(AlphaMap1, alphaCoords).b - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap1, alphaCoords).b - AlphaVal, 0.0) * texVal2.a;
	AlphaVal +=  texVal2.a;
	vec4 texVal1 = texture2D(tex1, texCoord1) * max(texture2D(AlphaMap1, alphaCoords).g - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap1, alphaCoords).g - AlphaVal, 0.0) * texVal1.a;
	AlphaVal +=  texVal1.a;
	vec4 texVal0 = texture2D(tex0, texCoord0) * max(texture2D(AlphaMap1, alphaCoords).r - AlphaVal, 0.0);
	//AlphaVal += max(texture2D(AlphaMap1, alphaCoords).r - AlphaVal, 0.0) * texVal0.a;
	AlphaVal +=  texVal0.a;
	
	
	/*vec4 texVal0 = texture2D(tex0, texCoord0) * texAlpha0;
	vec4 texVal1 = texture2D(tex1, texCoord1) * texAlpha1;
	vec4 texVal2 = texture2D(tex2, texCoord2) * texAlpha2;
	vec4 texVal3 = texture2D(tex3, texCoord3) * texAlpha3;
	vec4 texVal4 = texture2D(tex4, texCoord4) * texAlpha4;
	vec4 texVal5 = texture2D(tex5, texCoord5) * texAlpha5;
	vec4 texVal6 = texture2D(tex6, texCoord6) * texAlpha6;
	vec4 texVal7 = texture2D(tex7, texCoord7) * texAlpha7;
	vec4 texVal8 = texture2D(tex8, texCoord8) * texAlpha8;
	vec4 texVal9 = texture2D(tex9, texCoord9) * texAlpha9;
	vec4 texVal10 = texture2D(tex10, texCoord10) * texAlpha10;
	vec4 texVal11 = texture2D(tex11, texCoord11) * texAlpha11;*/
	
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
	texSum = texture2D(tex9, texCoord9);//  * texture2D(AlphaMap3, alphaCoords).g
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
