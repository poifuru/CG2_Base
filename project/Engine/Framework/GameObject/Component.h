#pragma once
#include <string>
#include <json.hpp>

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

	// エディタで表示するためのコンポーネント名
	virtual const char* GetName() const = 0;

	// 所属するGameObjectへのアクセス
	void SetGameObject(GameObject* gameObject) { gameObject_ = gameObject; }
	GameObject* GetGameObject() const { return gameObject_; }

protected:
	GameObject* gameObject_ = nullptr; // 親オブジェクトへのポインタ
};