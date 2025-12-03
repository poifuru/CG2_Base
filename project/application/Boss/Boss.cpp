#include "Boss.h"
#include "MathFunction.h"
#include <imgui.h>

Boss::Boss(MagosuyaEngine* magosuya) {
	magosuya_ = magosuya;
	model_ = std::make_unique<Model>(magosuya);
	magosuya_->LoadModelData("Resources/teapot", "teapot");
}

Boss::~Boss() {
	
}

void Boss::Initialize() {
	model_->SetModelData("teapot");
	model_->SetTexture("teapot");
	model_->Initialize();

	centerStomp_ = std::make_unique<CenterStomp>(magosuya_, this);
	centerStomp_->Initialize();

	fullScreenAttack_ = std::make_unique<FullScreenAttack>(magosuya_, this);
	fullScreenAttack_->Initialize();
}

void Boss::Update(Matrix4x4* m) {
	// 行動の更新
	UpdateMove();

	if (magosuya_->GetRawInput()->Trigger('1')) {
		centerStomp_->StartAttack();
	}
	if (magosuya_->GetRawInput()->Trigger('2')) {
		fullScreenAttack_->StartAttack();
	}

	model_->Update(m);
	centerStomp_->Update(m);
	fullScreenAttack_->Update(m);

	model_->SetTransform(transform_);
}

void Boss::Draw() {
	model_->Draw();
	centerStomp_->Draw();
	fullScreenAttack_->Draw();
}

void Boss::ImGuiControl() {
#ifdef _DEBUG
	model_->ImGui ("boss");

	centerStomp_->ImGuiControl();
	fullScreenAttack_->ImGuiControl();
#endif
}

bool Boss::IsAnyAttackActive() const {
	return centerStomp_->IsAttacking() || fullScreenAttack_->IsAttacking();
}

void Boss::UpdateMove() {
	if (IsAnyAttackActive()) {
		return;
	}
	transform_.translate.x += static_cast<float>(rand() % 3 - 1) * speed_.x;
	transform_.translate.z += static_cast<float>(rand() % 3 - 1) * speed_.z;
}