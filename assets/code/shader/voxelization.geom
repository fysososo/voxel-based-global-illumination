#version 450 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in Vertex
{
	vec2 texCoord;
	//vec3 normal;
} In[3];


out vec3 FragPos;
out vec2 TexCoord;
out vec3 ClipPos;
out vec4 BoundingBox;

uniform mat4 viewProject[3];
uniform mat4 viewProjectI[3];
uniform uint dimension;

uint selectVP()
{
	//计算三角形面片的法线
	vec3 p1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 p2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
	vec3 faceNormal = cross(p1, p2);

	
	float nDX = abs(faceNormal.x);//法线在x轴上的投影
	float nDY = abs(faceNormal.y);//法线在y轴上的投影
	float nDZ = abs(faceNormal.z);//法线在z轴上的投影

	if( nDX > nDY && nDX > nDZ )
	{
		return 0;//选择投影到yz平面
	}
	else if( nDY > nDX && nDY > nDZ  )
	{
		return 1;//选择投影到xz平面
	}
	else
	{
		return 2;//选择投影到xy平面
	}
} 

//计算包围盒
vec4 AxisAlignedBoundingBox(vec4 pos[3], vec2 pixelDiagonal)
{
	vec4 aabb;

	aabb.xy = min(pos[2].xy, min(pos[1].xy, pos[0].xy));
	aabb.zw = max(pos[2].xy, max(pos[1].xy, pos[0].xy));

	aabb.xy -= pixelDiagonal;
	aabb.zw += pixelDiagonal;

	return aabb;
}

void main() {
	//选择vp矩阵
	uint projectIndex = selectVP();
	mat4 viewProjection = viewProject[projectIndex];
	mat4 viewProjectionI = viewProjectI[projectIndex];

	//获取裁剪空间坐标
	vec4 clipPos[3] = vec4[3]
	(
		viewProjection * gl_in[0].gl_Position,
		viewProjection * gl_in[1].gl_Position,
		viewProjection * gl_in[2].gl_Position
	);

	//获取纹理坐标
	vec2 texCoord[3];
	for (int i = 0; i < gl_in.length(); i++)
	{
		texCoord[i] = In[i].texCoord; 
	}

	//确保输入的三角形的顶点环绕顺序都是逆时针方向
	vec4 trianglePlane;
	trianglePlane.xyz = cross(clipPos[1].xyz - clipPos[0].xyz, clipPos[2].xyz - clipPos[0].xyz);
	trianglePlane.xyz = normalize(trianglePlane.xyz);
	if(trianglePlane.z == 0.0f) return;
	trianglePlane.w = -dot(clipPos[0].xyz, trianglePlane.xyz);
	if (dot(trianglePlane.xyz, vec3(0.0, 0.0, 1.0)) < 0.0)
	{
		vec4 vertexTemp = clipPos[2];
		vec2 texCoordTemp = texCoord[2];
		
		clipPos[2] = clipPos[1];
		texCoord[2] = texCoord[1];
	
		clipPos[1] = vertexTemp;
		texCoord[1] = texCoordTemp;
	}

	//过原点与三角边的三个齐次平面
	vec3 planes[3];
	planes[0] = cross(clipPos[0].xyw - clipPos[2].xyw, clipPos[2].xyw);
	planes[1] = cross(clipPos[1].xyw - clipPos[0].xyw, clipPos[0].xyw);
	planes[2] = cross(clipPos[2].xyw - clipPos[1].xyw, clipPos[1].xyw);

	//将三个齐次平面沿各自法线向外平移一个像素单元格
	vec2 halfPixel = vec2(1.0f / dimension);
	planes[0].z -= dot(halfPixel, abs(planes[0].xy));
	planes[1].z -= dot(halfPixel, abs(planes[1].xy));
	planes[2].z -= dot(halfPixel, abs(planes[2].xy));

	//计算包围盒
	BoundingBox = AxisAlignedBoundingBox(clipPos, halfPixel);

	//三个齐次平面的两两交线
	vec3 intersection[3];
	intersection[0] = cross(planes[0], planes[1]);
	intersection[1] = cross(planes[1], planes[2]);
	intersection[2] = cross(planes[2], planes[0]);
	intersection[0] /= intersection[0].z;
	intersection[1] /= intersection[1].z;
	intersection[2] /= intersection[2].z;

	//交线与原三角面的交点就是扩大后的三角面新顶点
	float z[3];
	z[0] = -(intersection[0].x * trianglePlane.x + intersection[0].y * trianglePlane.y + trianglePlane.w) / trianglePlane.z;
	z[1] = -(intersection[1].x * trianglePlane.x + intersection[1].y * trianglePlane.y + trianglePlane.w) / trianglePlane.z;
	z[2] = -(intersection[2].x * trianglePlane.x + intersection[2].y * trianglePlane.y + trianglePlane.w) / trianglePlane.z;
	clipPos[0].xyz = vec3(intersection[0].xy, z[0]);
	clipPos[1].xyz = vec3(intersection[1].xy, z[1]);
	clipPos[2].xyz = vec3(intersection[2].xy, z[2]);


	//重新绘制新三角形
	for(int i = 0; i < 3; ++i)
	{
		//逆投影变换，计算新顶点的世界坐标
		vec4 voxelPos = viewProjectionI * clipPos[i];
		FragPos = voxelPos.xyz;

		gl_Position = clipPos[i];
		ClipPos = clipPos[i].xyz;
		TexCoord = In[i].texCoord;
		EmitVertex();
	}

	//结束绘制
	EndPrimitive();
}