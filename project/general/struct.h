#pragma once
#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <vector>
#include <string>
#include <fstream>
#include <Mmreg.h>
#include "DirectXTex.h"

//*** シェーダーに送るための構造体を作るときは16バイト境界を意識してパディングを入れてね ***//

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

	/*複合代入演算子*/
	Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
	Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
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

struct Sphere {
	Vector3 center;	//中心点
	float radius;	//半径
};

//直線
struct Line {
	Vector3 origin; //始点
	Vector3 diff;	//終点への差分ベクトル
};

//半直線
struct Ray {
	Vector3 origin; //始点
	Vector3 diff;	//終点への差分ベクトル
};


//線分
struct Segment {
	Vector3 origin; //始点
	Vector3 diff;	//終点への差分ベクトル
};

struct Plane {
	Vector3 normal;	//法線
	float distance;	//距離
};

//三角形
struct Triangle {
	Vector3 vertices[3];	//頂点
};

//AABB
struct AABB {
	Vector3 min;
	Vector3 max;
};

//バネ構造体
struct Spring {
	//アンカー。固定された端の位置
	Vector3 anchor;
	float naturalLength;		//自然長
	float stiffness;			//剛性。バネ定数k
	float dampingCoefficient;	//減衰係数
};

//ボール構造体
struct Ball {
	Vector3 position;		//ボールの位置
	Vector3 velocity;		//ボールの速度
	Vector3 acceleration;	//ボールの加速度
	float mass;				//ボールの質量
	float radius;			//ボールの半径
	unsigned int color;		//ボールの色
};

//振り子構造体
struct Pendulum {
	Vector3 anchor;				//アンカーポイント。固定された端の位置
	Vector3 position;			//振り子の先端
	float length;				//紐の長さ
	float angle;				//現在の角度
	float angularVelocity;		//角速度ω
	float angularAcceleration;	//角加速度ω
};

//円錐振り子構造体
struct ConicalPendulum {
	Vector3 anchor;			//アンカー
	Vector3 position;		//振り子の先端
	float length;			//紐の長さ
	float halfApexAngle;	//円錐の頂角の半分
	float angle;			//現在の角度
	float angularVelocity;	//角速度ω
};

//Transform構造体
struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

// SpriteRendererが参照する最小限の情報構造体
struct TransformData {
	Transform transform;
	Transform uvTransform;
	Matrix4x4 wvpMatrix;
};

//頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

//ライティングの反射モデル
enum LightReflectionModel {
	None,
	Lambert,
	HalfLambert,
};

//マテリアルの構造体
struct Material {
	Vector4 color;
	LightReflectionModel enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float roughness; // 粗さ
	float metallic; // 金属度
};

//TransformationMatrix構造体
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	//ライティングの時に正しい法線を計算する
	Matrix4x4 WorldInverseTranspose;
};

//テクスチャデータ構造体
struct TextureData {
	//テクスチャリソースハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
	//テクスチャリソース
	ComPtr<ID3D12Resource> textureResource = nullptr;
	//メタデータ
	DirectX::TexMetadata metadata = {};
	//どのディスクリプタヒープを使ったか
	UINT descriptorIndex = 0;
	//参照カウント
	int ref_count = 0;
};

//Sprite構造体
struct SpriteData {
	Vector2 size;			//幅と高さ
	Transform transform;	//SRT
	Transform uvTransform;	//uvのSRT
	Material* material;		//紐づけるマテリアルポインタ
	Matrix4x4 wvpMatrix;	//wvp行列ポインタ
};

//MaterialData構造体
struct MaterialFile {
	std::string textureFilePath;
};

struct Node {
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

//ModelData構造体
struct ModelData {
	//形状情報 (CPU側データ)
	MaterialFile material;
	std::vector<VertexData> vertices;
	size_t vertexCount = 0;

	//インデックス描画用のCPU側データ
	std::vector<uint32_t> indices;
	size_t indexCount = 0;

	//Dxリソース (GPU側データ) インスタンス間で共有される
	//頂点バッファ
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	//インデックスバッファ
	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW ibView{};

	//ルートノード(階層構造)
	Node rootNode;
};

//エミッター構造体
struct Emitter {
	Transform transform;	//transform
	uint32_t count;			//発生数
	float frequency;		//発生頻度
	float frequencyTime;	//頻度用時刻
};