#version 400 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 viewMatrix 	= mat4(1.0f);
uniform mat4 projMatrix 	= mat4(1.0f);
uniform mat4 shadowMatrix 	= mat4(1.0f);

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;
layout(location = 7) in mat4 instanceMatrix;

uniform vec4 		objectColour = vec4(1,1,1,1);

uniform bool hasVertexColours = false;
uniform bool hasInstanceMatrix = false;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;
} OUT;

void main(void)
{
	mat4 mvp;
	mat3 normalMatrix;
	if(hasInstanceMatrix) {
		mvp 		  = (projMatrix * viewMatrix * instanceMatrix);
		normalMatrix = transpose ( inverse ( mat3 ( instanceMatrix )));
		OUT.worldPos 	= ( instanceMatrix * vec4 ( position ,1)). xyz ;
	}
	else{ 
	mvp 		  = (projMatrix * viewMatrix * modelMatrix);
	normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));
	OUT.worldPos 	= ( modelMatrix * vec4 ( position ,1)). xyz ;
	}
	vec3 wNormal = normalize ( normalMatrix * normalize ( normal ));
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

	OUT.shadowProj 	=  shadowMatrix * vec4 ( position,1);
	
	OUT.normal 		= wNormal;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wTangent, wNormal) * tangent.w;

	
	OUT.texCoord	= texCoord;
	OUT.colour		= objectColour;

	if(hasVertexColours) {
		OUT.colour		= objectColour * colour;
	}
	gl_Position		= mvp * vec4(position, 1.0);
}