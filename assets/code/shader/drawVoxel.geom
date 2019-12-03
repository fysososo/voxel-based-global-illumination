#version 450 core

// 接收顶点着色器输出的顶点位置
layout(points) in;
// 输出体素立方体（由六个三角形组组成，每个三角形组包含两个三角形）
layout(triangle_strip, max_vertices = 24) out;

//MVP矩阵
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

uniform float voxelSize;
uniform vec3 boxMin;

//uniform vec4 frustumPlanes[6];
////该体素是否在视景体里（暂未看懂算法）
//bool VoxelInFrustum(vec3 center, vec3 extent)
//{
//	vec4 plane;
//
//	for(int i = 0; i < 6; i++)
//	{
//		plane = frustumPlanes[i];
//		float d = dot(extent, abs(plane.xyz));
//		float r = dot(center, plane.xyz) + plane.w;
//
//		if(d + r > 0.0f == false)
//		{
//			return false;
//		}
//	}
//
//	return true;
//}

in vec4 albedo[];
out vec4 voxelColor;

//换算成世界坐标
vec3 VoxelToWorld(vec3 pos)
{
	vec3 result = pos;
	result *= voxelSize;

	return result + boxMin;
}

void main()
{
	//立方体八个顶点
	const vec4 cubeVertices[8] = vec4[8] 
	(
		vec4( 0.5f,  0.5f,  0.5f, 0.0f),
		vec4( 0.5f,  0.5f, -0.5f, 0.0f),
		vec4( 0.5f, -0.5f,  0.5f, 0.0f),
		vec4( 0.5f, -0.5f, -0.5f, 0.0f),
		vec4(-0.5f,  0.5f,  0.5f, 0.0f),
		vec4(-0.5f,  0.5f, -0.5f, 0.0f),
		vec4(-0.5f, -0.5f,  0.5f, 0.0f),
		vec4(-0.5f, -0.5f, -0.5f, 0.0f)
	);

	//立方体三角形组（四个点=两个三角形=一个面）索引
	const int cubeIndices[24]  = int[24] 
	(
		0, 2, 1, 3, // right
		6, 4, 7, 5, // left
		5, 4, 1, 0, // up
		6, 7, 2, 3, // down
		4, 6, 0, 2, // front
		1, 3, 5, 7  // back
	);

	vec3 center = VoxelToWorld(gl_in[0].gl_Position.xyz);
	vec3 extent = vec3(voxelSize);

	//确保该体素可见且在视景体内
	//if(albedo[0].a == 0.0f || !VoxelInFrustum(center, extent)) { return; }
	if(albedo[0].a == 0.0f) { return; }

	vec4 projectedVertices[8];

	//一个位置变成八个点
	for(int i = 0; i < 8; i++)
	{
		vec4 vertex = gl_in[0].gl_Position + cubeVertices[i];
		projectedVertices[i] = projection * view * model * vertex;
	}

	//绘制图元
	for(int face = 0; face < 6; face++)
	{
		for(int vertex = 0; vertex < 4; vertex++)
		{
			gl_Position = projectedVertices[cubeIndices[face * 4 + vertex]];
			
			voxelColor = albedo[0];
			EmitVertex();
		}

		EndPrimitive();//绘制完一个三角形组
	}
}