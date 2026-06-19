// ========================================= //
// アプリケーション層が見ることのできる唯一のヘッダー
// ========================================= //

#pragma once
#include <string>
#include <cstdint>
#include <memory>

class Model;
class RenderSystem;

class IEngine {
public:
	virtual ~IEngine() = default;

	virtual void Initialize() = 0;
	virtual bool ProcessMessage() = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;

	// レンダーシステムを取得するインターフェース
	virtual RenderSystem* GetRenderSystem() = 0;
};