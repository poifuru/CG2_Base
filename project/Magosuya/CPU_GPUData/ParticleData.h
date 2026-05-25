#pragma once
#include "struct.h"
#include <cstdint>

// エミッター構造体
struct Emitter {
	EulerTransform transform;	//transform
	uint32_t count;			//発生数
	float frequency;		//発生頻度
	float frequencyTime;	//頻度用時刻
};

struct ParticleVertex {
	Vector4 position;
	Vector2 texcoord;
};

// パーティクル一粒ごとの構造体
struct ParticleData {
	EulerTransform transform;
	Vector3 velocity;
	Vector3 acceleration;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

struct ParticleForGPU {
	Matrix4x4 world;
	Matrix4x4 WVP;
	Vector4 color;
};