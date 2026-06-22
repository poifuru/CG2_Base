#pragma once
#include <vector>
#include <memory>
#include "ICommand.h"
class CommandManager {
public:
	static CommandManager* GetInstance() {
		static CommandManager instance;
		return &instance;
	}

	// 新しい操作を登録して実行する
	void AddAndExecute(std::unique_ptr<ICommand> command);

	// 元に戻す
	void Undo();

	// やり直す
	void Redo();

	// シーン切り替え時などにスタックをクリアする
	void Clear();

private:
	CommandManager() = default;
	std::vector<std::unique_ptr<ICommand>> undoStack_;
	std::vector<std::unique_ptr<ICommand>> redoStack_;
};