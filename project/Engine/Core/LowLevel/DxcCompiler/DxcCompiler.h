#pragma once

// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //
// DxcCompiler 関連の初期化・保持を行う //
// ＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝＝ //

namespace MyEngine::LowLevel {
	class DxcCompiler {
	public:
		DxcCompiler() = default;
		~DxcCompiler() = default;

		/// <summary>
		/// 初期化処理
		/// </summary>
		void Initialize();

		/// <summary>
		/// DxcUtilsを取得
		/// </summary>
		/// <returns></returns>
		IDxcUtils* GetDxcUtils() const { return dxcUtils_.Get(); }

		/// <summary>
		/// DxcCompilerを取得
		/// </summary>
		/// <returns></returns>
		IDxcCompiler3* GetDxcCompiler() const { return dxcCompiler_.Get(); }

		/// <summary>
		/// IncludeHandlerを取得
		/// </summary>
		/// <returns></returns>
		IDxcIncludeHandler* GetIncludeHandler() const { return includeHandler_.Get(); }

	public:
		// コピーと移動の禁止
		DxcCompiler(const DxcCompiler&) = delete;
		DxcCompiler& operator=(const DxcCompiler&) = delete;
		DxcCompiler(DxcCompiler&&) = delete;
		DxcCompiler& operator=(DxcCompiler&&) = delete;

	private:
		// DXCシェーダーコンパイラ関連
		Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_;
		Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;
	};
}