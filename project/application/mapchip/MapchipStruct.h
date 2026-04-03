#pragma once
#include <stdint.h>
#include "struct.h"

//ブロックの種類
enum class MapChipType {
	kBlank,		//空白
	kFloor,		//床
	kWall,		//壁
	kCeiling,	//天井
	kDamage,	//ダメージ
	kGoal,		//ゴール

	kKorokoro = 8,	//コロコロ
	kFly,			//フライ
};

struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};

struct Rect {
	float left;		//左端
	float right;	//右端
	float bottom;	//下端
	float top;		//上端
};

struct MapforGPU {
	Matrix4x4 wvp;
	Matrix4x4 world;
	Matrix4x4 WorldInverseTranspose;
};

struct InstancingResource {
	ComPtr<ID3D12Resource> resource;
	MapforGPU* mappedData;
	std::vector<MapforGPU> cpuData;

	int srvIndex;
};