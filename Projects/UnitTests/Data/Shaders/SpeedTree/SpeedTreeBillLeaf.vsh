#ifdef GL_ES
// define default precision for float, vec, mat.
precision highp float;
#else
#define lowp
#define highp
#define mediump
#endif

attribute vec4 inPosition;
attribute vec3 inTangent;
attribute vec4 inColor;

attribute vec2 inTexCoord0;
varying mediump vec2 varTexCoord0;
varying lowp vec4 varVertexColor;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 projectionMatrix;
uniform vec3 worldTranslate;
uniform vec3 worldScale;

void main()
{
	varTexCoord0 = inTexCoord0;
	varVertexColor = inColor;
	gl_Position = projectionMatrix * vec4(worldScale * (inPosition.xyz - inTangent) + worldTranslate, inPosition.w) + modelViewProjectionMatrix * vec4(inTangent, 0.0);
}
