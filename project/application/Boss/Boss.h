#pragma once
#include "MagosuyaEngine.h"
#include "Model.h"
#include "CenterStomp.h"

class Boss {
public:
	Boss();
	~Boss();

	void Initialize();
	void Update(Matrix4x4* m);
	void Draw();
	void ImGuiControl();

	// Getter
	Transform& GetTransform() { return transform_; }
	Vector3& GetPosition() { return transform_.translate; }
	// Setter
	void SetTransform(Transform transform) { transform_ = transform; }
	void SetPosition(Vector3 position) { transform_.translate = position; }

private:
	void UpdateMove();
private:
	std::unique_ptr<Model> model_ = nullptr;
	
	std::unique_ptr <CenterStomp> centerStomp_ = nullptr;

	Transform transform_;
	Vector3 speed_;

	//デバッグ用
	InputManager* input_ = nullptr;
};