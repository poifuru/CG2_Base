#pragma once

//*** シェーダーに送るための構造体を作るときは16バイト境界を意識してパディングを入れてね ***//

//// Vector2構造体
//struct Vector2 {
//	float x;
//	float y;
//
//	// 足し算
//	Vector2 operator+(const Vector2& other) const {
//		return { x + other.x, y + other.y };
//	}
//	// 引き算
//	Vector2 operator-(const Vector2& other) const {
//		return { x - other.x, y - other.y };
//	}
//	// スカラー倍
//	Vector2 operator*(float s) const {
//		return { x * s, y * s };
//	}
//};
//
//// Vector3構造体
//struct Vector3 {
//	float x;
//	float y;
//	float z;
//
//	// 足し算
//	Vector3 operator+(const Vector3& other) const {
//		return { x + other.x, y + other.y, z + other.z };
//	}
//	// 引き算
//	Vector3 operator-(const Vector3& other) const {
//		return { x - other.x, y - other.y, z - other.z };
//	}
//	// スカラー倍
//	Vector3 operator*(float s) const {
//		return { x * s, y * s, z * s };
//	}
//
//	/*複合代入演算子*/
//	Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
//	Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
//	Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
//	Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
//};
//
//// Vector4構造体
//struct Vector4 {
//	float x;
//	float y;
//	float z;
//	float w;
//
//	// 足し算
//	Vector4 operator+(const Vector4& other) const {
//		return { x + other.x, y + other.y, z + other.z, w + other.w };
//	}
//	// 引き算
//	Vector4 operator-(const Vector4& other) const {
//		return { x - other.x, y - other.y, z - other.z, w - other.w };
//	}
//	// スカラー倍
//	Vector4 operator*(float s) const {
//		return { x * s, y * s, z * s, w * s };
//	}
//};
//
//// 3x3Matrix構造体
//struct Matrix3x3 {
//	float m[3][3];
//};
//
//// 4x4Matrix構造体
//struct Matrix4x4 {
//	float m[4][4];
//};
//
//// 球
//struct Sphere {
//	Vector3 center;	//中心点
//	float radius;	//半径
//};
//
//// 直線
//struct Line {
//	Vector3 origin; //始点
//	Vector3 diff;	//終点への差分ベクトル
//};
//
//// 半直線
//struct Ray {
//	Vector3 origin; //始点
//	Vector3 diff;	//終点への差分ベクトル
//};
//
//
//// 線分
//struct Segment {
//	Vector3 origin; //始点
//	Vector3 diff;	//終点への差分ベクトル
//};
//
//// 平面
//struct Plane {
//	Vector3 normal;	//法線
//	float distance;	//距離
//};
//
//// 三角形
//struct Triangle {
//	Vector3 vertices[3];	//頂点
//};
//
//// AABB
//struct AABB {
//	Vector3 min;
//	Vector3 max;
//};
//
//// バネ構造体
//struct Spring {
//	// アンカー。固定された端の位置
//	Vector3 anchor;
//	float naturalLength;		//自然長
//	float stiffness;			//剛性。バネ定数k
//	float dampingCoefficient;	//減衰係数
//};
//
//// ボール構造体
//struct Ball {
//	Vector3 position;		//ボールの位置
//	Vector3 velocity;		//ボールの速度
//	Vector3 acceleration;	//ボールの加速度
//	float mass;				//ボールの質量
//	float radius;			//ボールの半径
//	unsigned int color;		//ボールの色
//};
//
//// 振り子構造体
//struct Pendulum {
//	Vector3 anchor;				//アンカーポイント。固定された端の位置
//	Vector3 position;			//振り子の先端
//	float length;				//紐の長さ
//	float angle;				//現在の角度
//	float angularVelocity;		//角速度ω
//	float angularAcceleration;	//角加速度ω
//};
//
//// 円錐振り子構造体
//struct ConicalPendulum {
//	Vector3 anchor;			//アンカー
//	Vector3 position;		//振り子の先端
//	float length;			//紐の長さ
//	float halfApexAngle;	//円錐の頂角の半分
//	float angle;			//現在の角度
//	float angularVelocity;	//角速度ω
//};
//
//// Quaternion構造体
//struct Quaternion {
//	float x;
//	float y;
//	float z;
//	float w;
//
//	// 基本的な演算子のオーバーロード
//	Quaternion operator*(float s) const { return { x * s, y * s, z * s, w * s }; }
//	Quaternion operator+(const Quaternion& q) const { return { x + q.x, y + q.y, z + q.z, w + q.w }; }
//	Quaternion operator-() const { return { -x, -y, -z, -w }; }
//};
//
//// EulerTransform構造体
//struct EulerTransform {
//	Vector3 scale;
//	Vector3 rotate;
//	Vector3 translate;
//};
//
//// Quaternion
//struct QuaternionTransform {
//	Vector3 scale;
//	Quaternion rotate;
//	Vector3 translate;
//};
//
//// SpriteRendererが参照する最小限の情報構造体
//struct TransformData {
//	EulerTransform transform;
//	EulerTransform uvTransform;
//	Matrix4x4 wvpMatrix;
//};

// 頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

// Sprite構造体
struct SpriteData {
	Vector2 size;			//幅と高さ
	EulerTransform transform;	//SRT
	EulerTransform uvTransform;	//uvのSRT
	//Material* material;		//紐づけるマテリアルポインタ
	Matrix4x4 wvpMatrix;	//wvp行列ポインタ
};

// エミッター構造体
struct Emitter {
	EulerTransform transform;	//transform
	uint32_t count;			//発生数
	float frequency;		//発生頻度
	float frequencyTime;	//頻度用時刻
};