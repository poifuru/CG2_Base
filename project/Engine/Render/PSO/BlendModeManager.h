#pragma once

namespace MyEngine::Rendering {
	enum class BlendModeType : uint8_t {
		Opaque,         // 不透明
		Alpha,          // アルファブレンド
		Additive,       // 加算合成
		Subtract,       // 減算合成
		Multiply,       // 乗算合成
		Screen,         // スクリーン合成

		Count			// モードの個数
	};

	class BlendModeManager {
	public:		//メンバ関数
		BlendModeManager() = default;
		~BlendModeManager() = default;

		void Initialize();

		//BlectModeTypeを受け取って、対応するブレンド設定を返す
		const D3D12_BLEND_DESC& GetBlendDesc(MyEngine::Rendering::BlendModeType type) const;

	public:
		BlendModeManager(const BlendModeManager&) = delete;
		BlendModeManager& operator=(const BlendModeManager&) = delete;
		BlendModeManager(BlendModeManager&&) = delete;
		BlendModeManager& operator=(BlendModeManager&&) = delete;

	private:	//メンバ変数
		//ブレンド設定の配列
		std::array< D3D12_BLEND_DESC, static_cast<size_t>(MyEngine::Rendering::BlendModeType::Count)> m_BlendDescs;

	};
}