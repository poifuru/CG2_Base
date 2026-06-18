#pragma once
#include <d3d12.h>
#include <dxcapi.h>
#include <wrl.h>
#include <string>
#include <unordered_map>

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

	void Initialize (IDxcCompiler3* compiler, IDxcUtils* utils, IDxcIncludeHandler* includeHandler);

	// シェーダーのコンパイルとキャッシュ登録
	uint32_t CompileAndCacheShader(const std::wstring& filePath, const wchar_t* profile);

	// IDに基づいてD3D12_SHADER_BYTECODEを返す
	D3D12_SHADER_BYTECODE GetShaderBytecode(uint32_t shaderID) const;

private:
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
	Microsoft::WRL::ComPtr<IDxcBlob> CompilerShader (const std::wstring& filePath, const wchar_t* profile);

private:	// メンバ変数
	std::unordered_map<uint32_t, ShaderInfo> shaderCache_;
	std::unordered_map<std::wstring, uint32_t> pathProfileToID_;
	uint32_t nextID_ = 0;

	IDxcCompiler3* dxcCompiler_ = nullptr;
	IDxcUtils* dxcUtils_ = nullptr;
	IDxcIncludeHandler* includeHandler_ = nullptr;
};