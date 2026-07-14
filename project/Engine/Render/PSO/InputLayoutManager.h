#pragma once
#include "RenderingModel.h"

namespace MyEngine::Rendering {
// InputLayoutの構造体（D3D12_INPUT_LAYOUT_DESCに相当するが、配列実体を保持）
struct InputLayoutData {
    std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
    D3D12_INPUT_LAYOUT_DESC desc; // D3D12_GRAPHICS_PIPELINE_STATE_DESCに渡す構造体
};

	class InputLayoutManager {
	public:     //メンバ関数
		InputLayoutManager() = default;
		~InputLayoutManager() = default;

		//登録したい設定を全部初期化
		void Initialize();

		// D3D12_INPUT_LAYOUT_DESCへのポインタを返す
		const D3D12_INPUT_LAYOUT_DESC* GetInputLayout(MyEngine::Rendering::InputLayoutType type) const;

	public:
		InputLayoutManager(const InputLayoutManager&) = delete;
		InputLayoutManager& operator=(const InputLayoutManager&) = delete;
		InputLayoutManager(InputLayoutManager&&) = delete;
		InputLayoutManager& operator=(InputLayoutManager&&) = delete;

	private:    //メンバ変数
		// InputLayoutTypeと実体データのマップ
		std::unordered_map<MyEngine::Rendering::InputLayoutType, MyEngine::Rendering::InputLayoutData> m_LayoutCache;
	};
}