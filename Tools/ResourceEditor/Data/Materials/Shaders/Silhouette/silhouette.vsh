#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

attribute vec4 inPosition;
attribute vec3 inNormal;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;

uniform mediump float silhouetteScale;
uniform mediump float silhouetteExponent;

void main()
{
	vec3 normal = normalize(normalMatrix * inNormal.xyz);
	vec4 PosView = modelViewMatrix * inPosition;

	mediump float distanceScale = length(PosView.xyz) / 100.0;

	PosView.xyz += normal * pow(silhouetteScale * distanceScale, silhouetteExponent);
	gl_Position = projectionMatrix * PosView;
}