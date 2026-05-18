//#version 330 compatibility

uniform vec3 camPos;
varying vec2 texCoord;
uniform mat4 normalMat;
varying vec3 normal;
varying vec3 lightVec;
//varying vec3 viewVec;
uniform float cAlpha;
varying float fAlpha;

void main()
{
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	texCoord = gl_MultiTexCoord0.xy;
	normal = normalize(mat3(normalMat) * gl_Normal);
	//normal = normalize(gl_Normal);
	//lightVec = gl_LightSource[0].position.xyz - gl_Vertex.xyz;
	lightVec = vec3(50000, 40000, -32000);
	//viewVec = camPos - gl_Vertex.xyz;
	fAlpha = cAlpha;
}
