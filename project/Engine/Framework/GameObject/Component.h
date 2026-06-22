#pragma once
#include <string>
#include <json.hpp>
using json = nlohmann::json;

class GameObject;

class Component {
public:
	Component() = default;
	virtual ~Component() = default;

	// 基本ライフサイクル
	virtual void Initialize() {}
	virtual void Update() {}
	virtual void Draw(class RenderSystem* renderSystem) {}
	virtual void ImGui() {}

	// Jsonへの書き出し
	virtual void Serialize(json& j) const {}
	// Jsonからの読み込み
	virtual void Deserialize(const json& j) {}

	// エディタで表示するためのコンポーネント名
	virtual const char* GetName() const = 0;

	// 所属するGameObjectへのアクセス
	void SetGameObject(GameObject* gameObject) { gameObject_ = gameObject; }
	GameObject* GetGameObject() const { return gameObject_; }

protected:
	GameObject* gameObject_ = nullptr; // 親オブジェクトへのポインタ
};