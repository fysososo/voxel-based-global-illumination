#version 450 core

// ���ն�����ɫ������Ķ���λ��
layout(points) in;
// ������������壨����������������ɣ�ÿ����������������������Σ�
layout(triangle_strip, max_vertices = 24) out;

//MVP����
uniform struct Matrices
{
	mat4 view;
    mat4 model;
	mat4 projection;
} matrices;

uniform float voxelSize;
uniform vec3 boxMin;

//uniform vec4 frustumPlanes[6];
////�������Ƿ����Ӿ������δ�����㷨��
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

//�������������
vec3 VoxelToWorld(vec3 pos)
{
	vec3 result = pos;
	result *= voxelSize;

	return result + boxMin;
}

void main()
{
	//������˸�����
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

	//�������������飨�ĸ���=����������=һ���棩����
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

	//ȷ�������ؿɼ������Ӿ�����
	//if(albedo[0].a == 0.0f || !VoxelInFrustum(center, extent)) { return; }
	if(albedo[0].a == 0.0f) { return; }

	vec4 projectedVertices[8];

	//һ��λ�ñ�ɰ˸���
	for(int i = 0; i < 8; i++)
	{
		vec4 vertex = gl_in[0].gl_Position + cubeVertices[i];
		projectedVertices[i] = matrices.projection * matrices.view * vertex;
	}

	//����ͼԪ
	for(int face = 0; face < 6; face++)
	{
		for(int vertex = 0; vertex < 4; vertex++)
		{
			gl_Position = projectedVertices[cubeIndices[face * 4 + vertex]];
			
			voxelColor = albedo[0];
			EmitVertex();
		}

		EndPrimitive();//������һ����������
	}
}