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
	/// どのモデルを使うのか
	/// </summary>
	void SetModelData (const std::string& ID);

	void SetTexture (const std::string& ID);

	//アクセッサ
	Vector3 GetPosition () { return Transform_.translate; }
	void SetPosition (const Vector3& position) { Transform_.translate = position; }
	Vector3 GetRotate () { return Transform_.rotate; }
	void SetRotate (const Vector3& rotate) { Transform_.rotate = rotate; }
	const EulerTransform& GetTransform () { return Transform_; }
	void SetTransform (EulerTransform Transform) { Transform_ = Transform; }
	EulerTransform GetUVTransform () { return uvTransform_; }
	void SetUVTransform (EulerTransform Transform) { uvTransform_ = Transform; }
	void SetColor (const Vector4& color) { renderer_->SetColor (color); }
	void SetAlpha(const float& alpha) { renderer_->SetAlpha(alpha); }
	void SetRoughness(const float& roughness) { renderer_->SetRoughness(roughness); }
	void SetMetallic(const float& metallic) { renderer_->SetMetallic(metallic); }
	void IsLighting (const LightReflectionModel& lighting) { renderer_->IsLighting (lighting); }

private:		//メンバ変数
	//マネージャーから受け取るモデルデータ
	std::weak_ptr<ModelData> modelData_;
	//貼り付けるテクスチャーのハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE texture_;

	EulerTransform Transform_ = {};
	EulerTransform uvTransform_ = {};

	//レンダラークラス
	std::unique_ptr<ModelRenderer> renderer_ = nullptr;
};