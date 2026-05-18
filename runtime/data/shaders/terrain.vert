#version 330 compatibility

uniform mat4 ModelViewProjectionMatrix;

in vec3 Vertex;

in vec2 MultiTexCoord0;
in vec2 MultiTexCoord1;
in vec2 MultiTexCoord2;
in vec2 MultiTexCoord3;
in vec2 MultiTexCoord4;
in vec2 MultiTexCoord5;
in vec2 MultiTexCoord6;
in vec2 MultiTexCoord7;
in vec2 MultiTexCoord8;
in vec2 MultiTexCoord9;
in vec2 MultiTexCoord10;
in vec2 MultiTexCoord11;
in vec2 AlphaMapCoords;

/*in vec4 TexInfo1;
in vec4 TexInfo2;
in vec4 TexInfo3;*/
in float LightVal;

uniform vec3 CamPos;

out float texAlpha0;
out float texAlpha1;
out float texAlpha2;
out float texAlpha3;
out float texAlpha4;
out float texAlpha5;
out float texAlpha6;
out float texAlpha7;
out float texAlpha8;
out float texAlpha9;
out float texAlpha10;
out float texAlpha11;

out vec2 texCoord0;
out vec2 texCoord1;
out vec2 texCoord2;
out vec2 texCoord3;
out vec2 texCoord4;
out vec2 texCoord5;
out vec2 texCoord6;
out vec2 texCoord7;
out vec2 texCoord8;
out vec2 texCoord9;
out vec2 texCoord10;
out vec2 texCoord11;
out vec2 alphaCoords;

out float OutLight;

//out vec3 viewVec;
out float viewDist;

void main()
{
	vec4 tVertex = vec4(Vertex.x, Vertex.y, Vertex.z, 1.0);
	//gl_Position = ModelViewProjectionMatrix * tVertex;
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	//gl_Position = tVertex;
	
	vec4 tCamPos = vec4(CamPos.x, CamPos.y, CamPos.z, 1.0);
	
	//viewVec = (tVertex - tCamPos).xyz;
	viewDist = length(gl_ModelViewMatrix * gl_Vertex - tCamPos);
	
	texAlpha0 = 0.0;
	texAlpha1 = 0.0;
	texAlpha2 = 0.0;
	texAlpha3 = 0.0;
	texAlpha4 = 0.0;
	texAlpha5 = 0.0;
	texAlpha6 = 0.0;
	texAlpha7 = 0.0;
	texAlpha8 = 0.0;
	texAlpha9 = 0.0;
	texAlpha10 = 0.0;
	texAlpha11 = 0.0;
	
	texCoord0 = MultiTexCoord0.xy;
	texCoord1 = MultiTexCoord1.xy;
	texCoord2 = MultiTexCoord2.xy;
	texCoord3 = MultiTexCoord3.xy;
	texCoord4 = MultiTexCoord4.xy;
	texCoord5 = MultiTexCoord5.xy;
	texCoord6 = MultiTexCoord6.xy;
	texCoord7 = MultiTexCoord7.xy;
	texCoord8 = MultiTexCoord8.xy;
	texCoord9 = MultiTexCoord9.xy;
	texCoord10 = MultiTexCoord10.xy;
	texCoord11 = MultiTexCoord11.xy;
	alphaCoords = AlphaMapCoords.xy;
	
	OutLight = LightVal;
}
