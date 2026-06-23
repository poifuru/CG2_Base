#pragma once
#include "ICommand.h"
#include "GameObject.h"

// Transform（位置・回転・スケール）の変更を記録するコマンド
class TransformCommand : public ICommand {
public:
	TransformCommand(GameObject* target, const EulerTransform& before, const EulerTransform& after);

	void Execute() override;

	void Undo() override;
	
private:
	GameObject* target_;
	EulerTransform before_; // 操作前のTransform
	EulerTransform after_;  // 操作後のTransform
};