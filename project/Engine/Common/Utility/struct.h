#pragma once

//*** シェーダーに送るための構造体を作るときは16バイト境界を意識してパディングを入れてね ***//

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