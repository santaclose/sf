#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec3 aBitangent;
layout(location = 4) in vec2 aTextureCoord;
layout(location = 5) in vec2 aExtraData;

layout(location = 6) in ivec4 aBoneIndices;
layout(location = 7) in vec4 aBoneWeights;

//out vec3 normal;
out vec2 texCoord;
out mat3 TBN;
out vec2 extraData;

out ivec4 boneIndices;
out vec4 boneWeights;

uniform mat4 cameraMatrix;
uniform mat4 modelMatrix;

void main()
{
	texCoord = aTextureCoord;
	extraData = aExtraData;

	boneIndices = aBoneIndices;
	boneWeights = aBoneWeights;

	vec3 T = normalize(vec3(modelMatrix * vec4(aTangent, 0.0)));
	vec3 B = normalize(vec3(modelMatrix * vec4(aBitangent, 0.0)));
	vec3 N = normalize(vec3(modelMatrix * vec4(aNormal, 0.0)));
	TBN = mat3(T, B, N);

	// standard normal passing
	//normal = (modelMatrix * vec4(aNormal, 0.0)).xyz;
	// for allowing non uniform scaling
	//normal = (transpose(inverse(modelMatrix))* aNormal).xyz;

	gl_Position = cameraMatrix * modelMatrix * vec4(aPosition, 1.0);
}