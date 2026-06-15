#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "struct.h"

struct TimelineEvent {
	float triggerTime = 0.0f;		// 出現タイミング(秒、またはレール進行度)
	std::string objectType = "";	// オブジェクトのタイプ名
	Vector3 spawnLocalPosition{};	// レール(カメラ)から見た相対出現座標
	nlohmann::json customParams{};	// 敵の速度やHPなど、個別の追加パラメータ

	// シリアライズ用
	void Save(nlohmann::json& json) const {
		json["triggerTime"] = triggerTime;
		json["objectType"] = objectType;
		json["localPos"] = { spawnLocalPosition.x, spawnLocalPosition.y, spawnLocalPosition.z };
		json["customParams"] = customParams;
	}

	void Load(const nlohmann::json& json) {
		triggerTime = json["triggerTime"];
		objectType = json["objectType"];
		spawnLocalPosition = { json["localPos"][0], json["localPos"][1], json["localPos"][2] };
		if(json.contains("customParams")) {
			customParams = json["customParams"];
		}
	}
};