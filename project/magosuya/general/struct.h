#pragma once
#include <Windows.h>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <vector>
#include <string>
#include <fstream>
#include <Mmreg.h>
#include <map>
#include <optional>
#include <span>
#include "DirectXTex.h"

//*** シェーダーに送るための構造体を作るときは16バイト境界を意識してパディングを入れてね ***//

// Vector2構造体
struct Vector2 {
	float x;
	float y;

	// 足し算
	Vector2 operator+(const Vector2& other) const {
		return { x + other.x, y + other.y };
	}
	// 引き算
	Vector2 operator-(const Vector2& other) const {
		return { x - other.x, y - other.y };
	}
	// スカラー倍
	Vector2 operator*(float s) const {
		return { x * s, y * s };
	}
};

// Vector3構造体
struct Vector3 {
	float x;
	float y;
	float z;

	// 足し算
	Vector3 operator+(const Vector3& other) const {
		return { x + other.x, y + other.y, z + other.z };
	}
	// 引き算
	Vector3 operator-(const Vector3& other) const {
		return { x - other.x, y - other.y, z - other.z };
	}
	// スカラー倍
	Vector3 operator*(float s) const {
		return { x * s, y * s, z * s };
	}

	/*複合代入演算子*/
	Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
	Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
	Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
	Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
};

// Vector4構造体
struct Vector4 {
	float x;
	float y;
	float z;
	float w;

	// 足し算
	Vector4 operator+(const Vector4& other) const {
		return { x + other.x, y + other.y, z + other.z, w + other.w };
	}
	// 引き算
	Vector4 operator-(const Vector4& other) const {
		return { x - other.x, y - other.y, z - other.z, w - other.w };
	}
	// スカラー倍
	Vector4 operator*(float s) const {
		return { x * s, y * s, z * s, w * s };
	}
};

// 3x3Matrix構造体
struct Matrix3x3 {
	float m[3][3];
};

// 4x4Matrix構造体
struct Matrix4x4 {
	float m[4][4];
};

struct Sphere {
	Vector3 center;	//中心点
	float radius;	//半径
};

// 直線
struct Line {
	Vector3 origin; //始点
	Vector3 diff;	//終点への差分ベクトル
};

// 半直線
struct Ray {
	Vector3 origin; //始点
	Vector3 diff;	//終点への差分ベクトル
};


// 線分
struct Segment {
	Vector3 origin; //始点
	Vector3 diff;	//終点への差分ベクトル
};

// 平面
struct Plane {
	Vector3 normal;	//法線
	float distance;	//距離
};

// 三角形
struct Triangle {
	Vector3 vertices[3];	//頂点
};

// AABB
struct AABB {
	Vector3 min;
	Vector3 max;
};

// バネ構造体
struct Spring {
	// アンカー。固定された端の位置
	Vector3 anchor;
	float naturalLength;		//自然長
	float stiffness;			//剛性。バネ定数k
	float dampingCoefficient;	//減衰係数
};

// ボール構造体
struct Ball {
	Vector3 position;		//ボールの位置
	Vector3 velocity;		//ボールの速度
	Vector3 acceleration;	//ボールの加速度
	float mass;				//ボールの質量
	float radius;			//ボールの半径
	unsigned int color;		//ボールの色
};

// 振り子構造体
struct Pendulum {
	Vector3 anchor;				//アンカーポイント。固定された端の位置
	Vector3 position;			//振り子の先端
	float length;				//紐の長さ
	float angle;				//現在の角度
	float angularVelocity;		//角速度ω
	float angularAcceleration;	//角加速度ω
};

// 円錐振り子構造体
struct ConicalPendulum {
	Vector3 anchor;			//アンカー
	Vector3 position;		//振り子の先端
	float length;			//紐の長さ
	float halfApexAngle;	//円錐の頂角の半分
	float angle;			//現在の角度
	float angularVelocity;	//角速度ω
};

// Quaternion構造体
struct Quaternion {
	float x;
	float y;
	float z;
	float w;

	// 基本的な演算子のオーバーロード
	Quaternion operator*(float s) const { return { x * s, y * s, z * s, w * s }; }
	Quaternion operator+(const Quaternion& q) const { return { x + q.x, y + q.y, z + q.z, w + q.w }; }
	Quaternion operator-() const { return { -x, -y, -z, -w }; }
};

// EulerTransform構造体
struct EulerTransform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

// Quaternion
struct QuaternionTransform {
	Vector3 scale;
	Quaternion rotate;
	Vector3 translate;
};

// SpriteRendererが参照する最小限の情報構造体
struct TransformData {
	EulerTransform transform;
	EulerTransform uvTransform;
	Matrix4x4 wvpMatrix;
};

// 頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

// ライティングの反射モデル
enum LightReflectionModel {
	None,
	Lambert,
	HalfLambert,
};

// マテリアルの構造体
struct Material {
	Vector4 color;
	LightReflectionModel enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
	float roughness; // 粗さ
	float metallic; // 金属度
};

// TransformationMatrix構造体
struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
	// ライティングの時に正しい法線を計算する
	Matrix4x4 WorldInverseTranspose;
};

// テクスチャデータ構造体
struct TextureData {
	// テクスチャリソースハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
	// テクスチャリソース
	ComPtr<ID3D12Resource> textureResource = nullptr;
	// メタデータ
	DirectX::TexMetadata metadata = {};
	// どのディスクリプタヒープを使ったか
	UINT descriptorIndex = 0;
	// 参照カウント
	int ref_count = 0;
};

// Sprite構造体
struct SpriteData {
	Vector2 size;			//幅と高さ
	EulerTransform transform;	//SRT
	EulerTransform uvTransform;	//uvのSRT
	Material* material;		//紐づけるマテリアルポインタ
	Matrix4x4 wvpMatrix;	//wvp行列ポインタ
};

// MaterialData構造体
struct MaterialFile {
	std::string textureFilePath;
};

// Node構造体
struct Node {
	QuaternionTransform transform;
	Matrix4x4 localMatrix;
	std::string name;
	std::vector<Node> children;
};

// 頂点のウェイトデータ
struct VertexWeightData {
	float weight;
	uint32_t vertexIndex;
};

// ジョイントのウェイトデータ
struct JointWeightData {
	Matrix4x4 inverseBindPoseMatrix;
	std::vector<VertexWeightData> vertexWeights;
};

// ModelData構造体
struct ModelData {
	// 形状情報 (CPU側データ)
	MaterialFile material;
	std::vector<VertexData> vertices;
	size_t vertexCount = 0;

	// インデックス描画用のCPU側データ
	std::vector<uint32_t> indices;
	size_t indexCount = 0;

	// Dxリソース (GPU側データ) インスタンス間で共有される
	// 頂点バッファ
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView{};

	// インデックスバッファ
	ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW ibView{};

	// ルートノード(階層構造)
	Node rootNode;

	// スキンクラスターのデータ
	std::map<std::string, JointWeightData> skinClusterData;
};

// エミッター構造体
struct Emitter {
	EulerTransform transform;	//transform
	uint32_t count;			//発生数
	float frequency;		//発生頻度
	float frequencyTime;	//頻度用時刻
};

// キーフレーム構造体(Vector3やQuaternionに対応させる)
template<typename tValue>
struct Keyframe {
	float time;
	tValue value;
};
using KeyframeVector3 = Keyframe<Vector3>;
using KeyframeQuaternion = Keyframe<Quaternion>;

template<typename tValue>
// AnimationCurve構造体
struct AnimationCurve {
	std::vector<Keyframe<tValue>> keyframes;
};

// NodeAnimation構造体
struct NodeAnimation {
	AnimationCurve<Vector3> translate;
	AnimationCurve<Quaternion> rotate;
	AnimationCurve<Vector3> scale;
};

// Animation構造体
struct Animation {
	float duration;	// アニメーション全体の尺
	// NodeAnimationの集合。Node名で引けるようにする(map)
	std::map<std::string, NodeAnimation> nodeAnimations;
};

// Joint構造体
struct Joint {
	QuaternionTransform transform;	//Transform情報
	Matrix4x4 localMatrix;	// localMatrix
	Matrix4x4 skeletonSpaceMatrix;	 // skeletonSpaceでの変換行列
	std::string name;	// 名前
	std::vector<int32_t> children;	// 子JointのIndexのリスト。いなければ空
	int32_t index;	// 自身のIndex
	std::optional<int32_t> parent;	// 親JointのIndex。いなければnull
};

// Skeleton構造体
struct Skeleton {
	int32_t	root;	// RootJointのIndex
	std::map<std::string, int32_t> jointMap;	// joint名とIndexとの辞書
	std::vector<Joint> joints;	// 所属しているジョイント
};

// jointの影響を受ける最大数(大体4つで問題なし)
const uint32_t kNumMaxInfluence = 4;

// VertexInfluence構造体
struct VertexInfluence {
	std::array<float, kNumMaxInfluence> weights;
	std::array<int32_t, kNumMaxInfluence> jointIndices;
};

// WellForGPU構造体
struct WellForGPU {
	Matrix4x4 skeletonSpaceMatrix;	// 位置用
	Matrix4x4 skeletonSpaceInverseTransposeMatrix;	// 法線用
};

// SkinCluster構造体
struct SkinCluster {
	std::vector<Matrix4x4> inverseBinePoseMatrices;

	ComPtr<ID3D12Resource> influenceResource;
	D3D12_VERTEX_BUFFER_VIEW influenceBufferView;
	std::span<VertexInfluence> mappedInfluence;

	ComPtr<ID3D12Resource> paletteResource;
	std::span<WellForGPU> mappedPalette;
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandle;
};