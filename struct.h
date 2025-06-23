#pragma once

//Vector2構造体
struct Vector2 {
	float x;
	float y;
};

//Vector3構造体
struct Vector3 {
	float x;
	float y;
	float z;
};

//Vector4構造体
struct Vector4 {
	float x;
	float y;
	float z;
	float w;
};

//4x4Matrix構造体
struct Matrix4x4 {
	float m[4][4];
};

//Transform構造体
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

//頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcooord;
	Vector3 normal;
};

//マテリアルの構造体
struct Material {
	Vector4 color;
	//int32_t enableLighting;
};