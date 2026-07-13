#pragma once

namespace MyEngine::Rendering {
	//シェーダー情報を保持する構造体
	struct ShaderInfo {
		Microsoft::WRL::ComPtr<IDxcBlob> ShaderBlob;
		std::wstring FilePath;	//シェーダーのファイルパス
		std::wstring Profile;	//コンパイルにつかったプロファイル
	};

	class ShaderManager {
	public:		// メンバ関数
		ShaderManager() = default;
		~ShaderManager() = default;

		void Initialize(
			IDxcUtils* utils,
			IDxcCompiler3* compiler,
			IDxcIncludeHandler* includeHandler
		);

		// シェーダーのコンパイルとキャッシュ登録
		uint32_t CompileAndCacheShader(const std::wstring& filePath, const wchar_t* profile);

		// IDに基づいてD3D12_SHADER_BYTECODEを返す
		D3D12_SHADER_BYTECODE GetShaderBytecode(uint32_t shaderID) const;

	public:
		// コピー・移動禁止
		ShaderManager(const ShaderManager&) = delete;
		ShaderManager& operator=(const ShaderManager&) = delete;
		ShaderManager(ShaderManager&&) = delete;
		ShaderManager& operator=(ShaderManager&&) = delete;

	private:	// ヘルパー関数
		/// <summary>
		/// シェーダーをコンパイルする関数
		/// </summary>
		/// <param name="filePath">shaderファイルへのパス</param>
		/// <param name="profile">使用するプロファイル</param>
		/// <returns>shader</returns>
		Microsoft::WRL::ComPtr<IDxcBlob> CompilerShader(const std::wstring& filePath, const wchar_t* profile);

	private:	// メンバ変数
		std::unordered_map<uint32_t, ShaderInfo> shaderCache_;
		std::unordered_map<std::wstring, uint32_t> pathProfileToID_;
		uint32_t nextID_ = 1;

		IDxcCompiler3* dxcCompiler_ = nullptr;
		IDxcUtils* dxcUtils_ = nullptr;
		IDxcIncludeHandler* includeHandler_ = nullptr;
	};
}