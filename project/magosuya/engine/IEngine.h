// ========================================= //
// アプリケーション層が見ることのできる唯一のヘッダー
// ========================================= //

#pragma once

class IEngine {
public:
	virtual ~IEngine() = default;

	virtual void Initialize() = 0;
	virtual bool ProcessMessage() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;
};