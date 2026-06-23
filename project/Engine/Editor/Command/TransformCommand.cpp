#include "TransformCommand.h"

TransformCommand::TransformCommand(GameObject* target, const EulerTransform& before, const EulerTransform& after) 
	: target_(target), before_(before), after_(after) {
}

void TransformCommand::Execute() {
	if (target_) target_->GetTransform() = after_;
}

void TransformCommand::Undo() {
	if (target_) target_->GetTransform() = before_;
}