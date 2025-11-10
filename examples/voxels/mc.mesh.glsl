#define WORK_GROUP_SIZE 16
layout(local_size_x = WORK_GROUP_SIZE) in;
layout(max_vertices = 15 * WORK_GROUP_SIZE, max_primitives = 5 * WORK_GROUP_SIZE) out;
layout(triangles) out;

#include <assets/shaders/marching_cubes.h>

uniform float threshold = 0.5;
uniform float voxelSize;
uniform uvec3 voxelCountPerAxis;
uniform uint threadsNeeded;
uniform uint svoDepth;
uniform vec3 offset;

vec3 interpolateVerts(vec3 posA, vec3 posB, float a, float b)
{
	float t = (threshold - a) / (b - a);
	return posA + t * (posB - posA);
}

layout(binding = 0) uniform SharedGpuData
{
	mat4 modelMatrix;
	mat4 cameraMatrix;
	float cameraPositionX;
	float cameraPositionY;
	float cameraPositionZ;
	float windowSizeX;
	float windowSizeY;
};

uint sampleSvo(uvec3 localCoords)
{
	uint currentDepth = 0u;
	uint currentDataIndex = 0u;
	while (currentDepth < svoDepth)
	{
		uint voxelsPerUnit = 1u << (svoDepth - currentDepth);

		if (voxelsPerUnit == 2u)
		{
			/* Base case */
			uint corner = localCoords.x | (localCoords.y << 1) | (localCoords.z << 2);
			return SVO_BUFFER[currentDataIndex * 8 + corner];
		}

		uint corner =
			uint(localCoords.x >= voxelsPerUnit / 2) |
			(uint(localCoords.y >= voxelsPerUnit / 2) << 1) |
			(uint(localCoords.z >= voxelsPerUnit / 2) << 2);
		uint value = SVO_BUFFER[currentDataIndex * 8 + corner];
		if (value == 0 || value == 0xffffffff)
			return value;

		localCoords.x %= voxelsPerUnit / 2;
		localCoords.y %= voxelsPerUnit / 2;
		localCoords.z %= voxelsPerUnit / 2;
		currentDataIndex = value;
		currentDepth++;
	}
	return 0; // should never happen
}

float sampleSvoFloat(uvec3 coords)
{
	return sampleSvo(coords) == 0 ? 0.0 : 1.0;
}

void main()
{
	uint lid = gl_LocalInvocationID.x;
	uint gid = gl_WorkGroupID.x * WORK_GROUP_SIZE + lid;
	if (gid > threadsNeeded)
		return;

	uvec3 targetCoords = uvec3(
			(gid) % voxelCountPerAxis.x,
			(gid / voxelCountPerAxis.x) % voxelCountPerAxis.y,
			(gid / voxelCountPerAxis.x / voxelCountPerAxis.y));

	vec3 cubeVertexPos[8] = {
		vec3(targetCoords + uvec3(0, 0, 0)) * voxelSize + vec3(voxelSize * 0.5), vec3(targetCoords + uvec3(1, 0, 0)) * voxelSize + vec3(voxelSize * 0.5), vec3(targetCoords + uvec3(1, 0, 1)) * voxelSize + vec3(voxelSize * 0.5), vec3(targetCoords + uvec3(0, 0, 1)) * voxelSize + vec3(voxelSize * 0.5),
		vec3(targetCoords + uvec3(0, 1, 0)) * voxelSize + vec3(voxelSize * 0.5), vec3(targetCoords + uvec3(1, 1, 0)) * voxelSize + vec3(voxelSize * 0.5), vec3(targetCoords + uvec3(1, 1, 1)) * voxelSize + vec3(voxelSize * 0.5), vec3(targetCoords + uvec3(0, 1, 1)) * voxelSize + vec3(voxelSize * 0.5),
	};

	float cubeCorners[8] = {
		sampleSvoFloat(targetCoords + uvec3(0, 0, 0)), sampleSvoFloat(targetCoords + uvec3(1, 0, 0)), sampleSvoFloat(targetCoords + uvec3(1, 0, 1)), sampleSvoFloat(targetCoords + uvec3(0, 0, 1)),
		sampleSvoFloat(targetCoords + uvec3(0, 1, 0)), sampleSvoFloat(targetCoords + uvec3(1, 1, 0)), sampleSvoFloat(targetCoords + uvec3(1, 1, 1)), sampleSvoFloat(targetCoords + uvec3(0, 1, 1)),
	};

	uint cubeIndex = 0;
	if (cubeCorners[0] < threshold) cubeIndex |= 1;
	if (cubeCorners[1] < threshold) cubeIndex |= 2;
	if (cubeCorners[2] < threshold) cubeIndex |= 4;
	if (cubeCorners[3] < threshold) cubeIndex |= 8;
	if (cubeCorners[4] < threshold) cubeIndex |= 16;
	if (cubeCorners[5] < threshold) cubeIndex |= 32;
	if (cubeCorners[6] < threshold) cubeIndex |= 64;
	if (cubeCorners[7] < threshold) cubeIndex |= 128;

	uint vertexBase = 15 * lid;

	uint i = 0;
	for (; MC_AccessTriangulation(cubeIndex, i * 3) != 0xff; i++)
	{
		uint a0 = MC_CORNER_A_FROM_EDGE[MC_AccessTriangulation(cubeIndex, i * 3 + 0)];
		uint b0 = MC_CORNER_B_FROM_EDGE[MC_AccessTriangulation(cubeIndex, i * 3 + 0)];
		uint a1 = MC_CORNER_A_FROM_EDGE[MC_AccessTriangulation(cubeIndex, i * 3 + 1)];
		uint b1 = MC_CORNER_B_FROM_EDGE[MC_AccessTriangulation(cubeIndex, i * 3 + 1)];
		uint a2 = MC_CORNER_A_FROM_EDGE[MC_AccessTriangulation(cubeIndex, i * 3 + 2)];
		uint b2 = MC_CORNER_B_FROM_EDGE[MC_AccessTriangulation(cubeIndex, i * 3 + 2)];

		gl_MeshVerticesNV[vertexBase + i * 3 + 2].gl_Position = cameraMatrix * modelMatrix * vec4(interpolateVerts(cubeVertexPos[a0], cubeVertexPos[b0], cubeCorners[a0], cubeCorners[b0]) + offset, 1.0);
		gl_MeshVerticesNV[vertexBase + i * 3 + 1].gl_Position = cameraMatrix * modelMatrix * vec4(interpolateVerts(cubeVertexPos[a1], cubeVertexPos[b1], cubeCorners[a1], cubeCorners[b1]) + offset, 1.0);
		gl_MeshVerticesNV[vertexBase + i * 3 + 0].gl_Position = cameraMatrix * modelMatrix * vec4(interpolateVerts(cubeVertexPos[a2], cubeVertexPos[b2], cubeCorners[a2], cubeCorners[b2]) + offset, 1.0);
		gl_PrimitiveIndicesNV[vertexBase + i * 3 + 0] = vertexBase + i * 3 + 0;
		gl_PrimitiveIndicesNV[vertexBase + i * 3 + 1] = vertexBase + i * 3 + 1;
		gl_PrimitiveIndicesNV[vertexBase + i * 3 + 2] = vertexBase + i * 3 + 2;
	}
	for (; i < 5; i++)
	{
		/* Fill the other primitives with invisible triangles */
		gl_MeshVerticesNV[vertexBase + i * 3 + 2].gl_Position = vec4(0.0);
		gl_MeshVerticesNV[vertexBase + i * 3 + 1].gl_Position = vec4(0.0);
		gl_MeshVerticesNV[vertexBase + i * 3 + 0].gl_Position = vec4(0.0);
		gl_PrimitiveIndicesNV[vertexBase + i * 3 + 0] = vertexBase + i * 3 + 0;
		gl_PrimitiveIndicesNV[vertexBase + i * 3 + 1] = vertexBase + i * 3 + 1;
		gl_PrimitiveIndicesNV[vertexBase + i * 3 + 2] = vertexBase + i * 3 + 2;
	}

	if (lid == 0)
		gl_PrimitiveCountNV = 5 * WORK_GROUP_SIZE;
}