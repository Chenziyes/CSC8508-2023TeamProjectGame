#version 400 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform sampler2D 	depthTex;
uniform sampler2D normTex;

uniform vec2 pixelSize;
uniform vec3 cameraPos;
uniform vec3 lightDirection;
uniform vec3 lightPos;
uniform float lightRadius;
uniform vec4	lightColour;
uniform mat4 inverseProjView;

out vec4 diffuseOutput;
out vec4 specularOutput;

void main(void)
{
	vec2 texCoord = vec2(gl_FragCoord.xy * pixelSize);
	float depth = texture(depthTex, texCoord.xy).r;
	vec3 ndcPos = vec3(texCoord, depth) * 2.0 - 1.0;
	vec4 invClipPos = inverseProjView * vec4(ndcPos, 1.0);
	vec3 worldPos = invClipPos.xyz / invClipPos.w;

	float dist = length(lightPos - worldPos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);

	if(atten == 0.0) { discard; }


	vec3 normal = normalize(texture(normTex, texCoord.xy).xyz * 2.0 - 1.0);
	vec3 incident = normalize(-lightDirection);
	vec3 viewDir = normalize(cameraPos - worldPos);
	vec3 halfDir = normalize(incident + viewDir);

	float lambert = clamp(dot(incident, normal), 0.0, 1.0);
	float specFactor = clamp(dot(halfDir, normal), 0.0, 1.0);
	specFactor = pow(specFactor, 60.0);
	diffuseOutput = vec4(lightColour.xyz * lambert, 1.0);

	specularOutput = vec4(lightColour.xyz * specFactor * 0.33, 1.0);

}