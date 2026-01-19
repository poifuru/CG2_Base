#pragma once
#include "BaseLight.h"
#include "struct.h"

class DirectionalLight : public BaseLight {
public:
	DirectionalLight();
	~DirectionalLight() override;
	void Initialize() override;
	void Update() override;

	//アクセッサ
	const Vector3& GetDirection() const { return direction_; }
	
private:
	//DirectionalLight全体の数
	static inline uint32_t numInstance_ = 0;
	//ImGuiで編集中の番号
	static inline int selectIndex_ = 0;
	//インスタンス自身の番号
	uint32_t myIndex_ = 0;

	//向き
	Vector3 direction_;
};