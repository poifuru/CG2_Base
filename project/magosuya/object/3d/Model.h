#pragma once
#include <Windows.h>
#include <Wrl.h>
using namespace Microsoft::WRL;
#include <vector>
#include <d3d12.h>
#include <memory>
#include "struct.h"
#include "ModelRenderer.h"
#include "CameraComponent.h"

class LightManager;

class Model {
public:	//メンバ関数
	//コンストラクタ
	/// <summary>
	/// 指定されたディレクトリパスとファイル名から3Dモデルを初期化します。
	/// </summary>
	/// <param name="directoryPath">3Dモデルファイルが存在するディレクトリのパス。</param>
	/// <param name="filename">読み込む3Dモデルのファイル名。</param>
	Model (DxCommon* dxCommon, LightManager* lightManager);

	~Model ();

	/// <summary>
	/// モデルのSRT
	/// </summary>
	/// <param name="scale">大きさ</param>
	/// <param name="rotate">回転</param>
	/// <param name="position">位置</param>
	void Initialize (Vector3 scale = { 1.0f, 1.0f, 1.0f }, Vector3 rotate = { 0.0f, 0.0f, 0.0f }, Vector3 position = { 0.0f, 0.0f, 0.0f });
	
	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="vp">vp行列</param>
	void Update (CameraData* cameraData);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="cmdList">コマンドリスト</param>
	/// <param name="textureHandle">使うテクスチャ</param>
	/// <param name="light">ライト</param>
	void Draw ();
	
	/// <summary>
	/// ImGuiで編集できるよ
	/// </summary>
	void ImGui (const std::string& windowName);

	/// <summary>
	/// 使うModelをセット
	/// </summary>
	/// /// <param name="ID">ModelのID</param>
	void SetModelData (const std::string& ID);

	/// <summary>
	/// 使うTextureをセット
	/// </summary>
	/// <param name="ID">TextureのID</param>
	void SetTexture (const std::string& ID);

	/// <summary>
	/// 使うAnimationをセット
	/// </summary>
	/// <param name="animation">ロードしたAnimation</param>
	void SetAnimation(const std::string& ID);

	/// <summary>
	/// スケルトンの初期化
	/// </summary>
	void SkeletonInit();

	//アクセッサ
	Vector3 GetPosition () { return transform_.translate; }
	void SetPosition (const Vector3& position) { transform_.translate = position; }
	Vector3 GetRotate () { return transform_.rotate; }
	void SetRotate (const Vector3& rotate) { transform_.rotate = rotate; }
	Vector3 GetScale() { return transform_.scale; }
	void SetScale(const Vector3& scale) { transform_.scale = scale; }
	const EulerTransform& GetTransform () { return transform_; }
	void SetTransform (EulerTransform transform) { transform_ = transform; }
	EulerTransform GetUVTransform () { return uvTransform_; }
	void SetUVTransform (EulerTransform transform) { uvTransform_ = transform; }
	void SetColor (const Vector4& color) { renderer_->SetColor (color); }
	void SetAlpha(const float& alpha) { renderer_->SetAlpha(alpha); }
	void SetRoughness(const float& roughness) { renderer_->SetRoughness(roughness); }
	void SetMetallic(const float& metallic) { renderer_->SetMetallic(metallic); }
	void IsLighting (const LightReflectionModel& lighting) { renderer_->IsLighting (lighting); }
	void SetDepthEnable(bool flag) { renderer_->SetDepthEnable(flag); }

private:		//メンバ変数
	//貼り付けるテクスチャーのハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE texture_;

	EulerTransform transform_ = {};
	EulerTransform uvTransform_ = {};

	//レンダラークラス
	std::unique_ptr<ModelRenderer> renderer_ = nullptr;
};