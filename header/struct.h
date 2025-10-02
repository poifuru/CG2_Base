#pragma once
#include <Windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <Mmreg.h>

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

//頂点データの構造体
struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
	//float padding1;
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
	//ライティングの時に正しい法線を計算する
	Matrix4x4 WorldInverseTranspose;
};

//Sprite構造体
struct SpriteData {
	Vector2 size;			//幅と高さ
	Transform transform;	//SRT
	Transform uvTransform;	//uvのSRT
	Material* material;		//紐づけるマテリアルポインタ
	Matrix4x4 wvpMatrix;	//wvp行列ポインタ
};

//enum Light {
//	none,
//	lambert,
//	halfLambert,
//};

//平行光源構造体
struct DirectionalLight {
	Vector4 color;		//ライトの色
	Vector3 direction;	//ライトの向き
	float intensity;	//輝度
	//Light mode;			//ライティングの設定
};

//MaterialData構造体
struct MaterialData {
	std::string textureFilePath;
};

//ModelData構造体
struct ModelData {
	MaterialData material;
	std::vector<VertexData> vertices;
	size_t vertexCount;
	std::vector<uint32_t> indices;
	size_t indexCount;
};

//チャンクヘッダ
struct ChunkHeader {
	char id[4];		//チャンク毎のID
	int32_t size;	//チャンクサイズ
};

//RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk;	//"RIFF"
	char type[4];		//"WAVE"
};

//FMTチャンク
struct FormatChunk {
	ChunkHeader chunk;	//"fmt"
	WAVEFORMATEX fmt;	//波形フォーマット
};

//サウンドデータ構造体
struct SoundData {
	//音データ識別用の名前
	std::string name;
	//波形フォーマット
	WAVEFORMATEX wfex;
	//実際に再生するためのバッファ
	//XAUDIO2_BUFFER buffer;
	//バッファの先頭アドレス
	BYTE* pBuffer;
	//バッファのサイズ
	unsigned int bufferSize;

	void Unload () {
		if (pBuffer) {
			delete[] pBuffer;
			pBuffer = nullptr;
		}
	}
};

//マウス構造体
struct MouseInput {
	//マウスの座標
	float x;
	float y;
	float z;

	//クリック・トリガーなどの判定
	bool left;
	bool prevLeft;
	bool right;
	bool prevRight;
	bool mid;
	bool prevMid;

	//トリガーの判定
	bool IsLeftTriggered() const {
		return left && !prevLeft;
	}
	bool IsRightTriggered() const {
		return right && !prevRight;
	}
	bool IsMidTriggered() const {
		return mid && !prevMid;
	}

	//リリース判定
	bool IsLeftReleased() const {
		return !left && prevLeft;
	}
	bool IsRightReleased() const {
		return !right && prevRight;
	}
	bool IsMidReleased() const {
		return !mid && prevMid;
	}
};

//*****shape描画に必要なデータたちの構造体*****//
struct ShapeData {
	std::vector<VertexData> vertexData;
	Matrix4x4 matrixData;
	Material materialData;
};