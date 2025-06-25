#pragma once
#include "header.h"

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

//3x3Matrix構造体
struct Matrix3x3 {
	float m[3][3];
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
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTranform;
};

//TransformationMatrix構造体
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

//平行光源構造体
struct DirectionalLight {
	Vector4 color;		//ライトの色
	Vector3 direction;	//ライトの向き
	float intensity;	//輝度
};