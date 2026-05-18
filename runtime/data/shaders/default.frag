//#version 330 compatibility

uniform sampler2D tex;
varying vec2 texCoord;
varying vec3 normal;
varying vec3 lightVec;
//varying vec3 viewVec;
varying float fAlpha;

void main()
{
	vec4 base = texture2D(tex, texCoord);
	vec3 norm = normalize(normal);
	vec3 lVec = normalize(lightVec);
	//vec3 vVec = normalize(viewVec);
	float diffuse = dot(lVec, norm);
	//float specular = pow(max(dot(normalize(lightVec + viewVec), norm), 0.0), 16.0);
	if(diffuse < 0.0) diffuse = 0.0;
	vec4 cvec = vec4(0., 0., 0., 1.);
	cvec = (0.7 * diffuse + 0.30) * base;
	cvec.a = base.a;
	if(fAlpha < 1.0)
		cvec.a = base.a * fAlpha;
	/*cvec.r = diffuse * base.r;
	cvec.g = diffuse * base.g;
	cvec.b = diffuse * base.b;*/
	/*cvec.r = normal.x;
	cvec.g = normal.y;
	cvec.b = normal.z;*/
	gl_FragColor = cvec;
}
