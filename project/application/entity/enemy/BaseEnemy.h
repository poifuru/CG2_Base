//#pragma once
//#include "Entity.h"
//#include <memory>
//
//class BaseEnemy : public Entity {
//public:
//	BaseEnemy(DxCommon* dxCommon, LightManager* light, CameraOrganizer* camera);
//	virtual ~BaseEnemy();
//
//	void Initialize() override;
//	void Update() override;
//	void Draw() override;
//	void ImGui();
//
//	// ゲッター
//	const AABB& GetAABB() const { return aabb_; }
//	EulerTransform GetTransform() { return transform_; }
//	void SetPosition(const Vector3& pos) { transform_.translate = pos; }
//	void SetAABBSize(const Vector3& size) { aabbSize_ = size; }
//	bool IsActive() { return isActive_; }
//	void SetIsActive(bool flag) { isActive_ = flag; }
//	void SetAlpha(float alpha);
//	void SetScale(const Vector3& scale);
//
//protected:
//	bool isActive_ = false;
//};