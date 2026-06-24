#include "PCH.h"
#include "CommandManager.h"

void CommandManager::AddAndExecute(std::unique_ptr<ICommand> command) {
	command->Execute();
	undoStack_.push_back(std::move(command));
	redoStack_.clear(); // 新しい操作をしたら、やり直し(Redo)スタックはクリアする
}

void CommandManager::Undo() {
	if (undoStack_.empty()) return;
	auto command = std::move(undoStack_.back());
	undoStack_.pop_back();
	command->Undo();
	redoStack_.push_back(std::move(command));
}

void CommandManager::Redo() {
	if (redoStack_.empty()) return;
	auto command = std::move(redoStack_.back());
	redoStack_.pop_back();
	command->Execute();
	undoStack_.push_back(std::move(command));
}

void CommandManager::Clear() {
	undoStack_.clear();
	redoStack_.clear();
}