#pragma once
#include <json.hpp>
#include <string>
#include "struct.h"

// エディターで配置できるようにするため、最低限のインターフェース
class GameObject {
public:
	// 初期化
	virtual void Initialize() = 0;

	// 更新
	virtual void Update() = 0;

	// 描画
	virtual void Draw() = 0;

	// インスペクター表示
	virtual void Inspector() = 0;

	// セーブ・ロード時の識別用
	virtual std::string GetTypeName() const = 0;

	// シリアライズ処理
	virtual void Save(nlohmann::json& json) const {
		json["type"] = GetTypeName();
		json["translate"] = { transform_.translate.x, transform_.translate.y, transform_.translate.z };
		json["rotate"] = { transform_.rotate.x, transform_.rotate.y, transform_.rotate.z };
		json["scale"] = { transform_.scale.x, transform_.scale.y, transform_.scale.z };
	}

	// デシリアライズ処理
	virtual void Load(const nlohmann::json& json) {
		if (json.contains("translate")) {
			transform_.translate = { json["translate"][0], json["translate"][1], json["translate"][2] };
		}
		if (json.contains("rotate")) {
			transform_.rotate = { json["rotate"][0], json["rotate"][1], json["rotate"][2] };
		}
		if (json.contains("scale")) {
			transform_.scale = { json["scale"][0], json["scale"][1], json["scale"][2] };
		}
	}

	// --- トランスフォーム操作 --- //
	const EulerTransform& GetTransform() const { return transform_; }
	EulerTransform& GetTransform() { return transform_; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
	void SetTranslate(const Vector3& translate) { transform_.translate; }

protected:
	EulerTransform transform_{};
};