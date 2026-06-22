#pragma once
// すべての「操作コマンド」の親となるインターフェース
class ICommand {
public:
	virtual ~ICommand() = default;
	virtual void Execute() = 0; // やり直す / 実行する
	virtual void Undo() = 0;    // 元に戻す
};