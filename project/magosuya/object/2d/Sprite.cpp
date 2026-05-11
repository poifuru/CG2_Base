#include "Sprite.h"
#include <DirectXTex.h>
#include "function.h"
#include "MathFunction.h"
#include "DxCommon.h"
#include "TextureManager.h"

Sprite::Sprite (DxCommon* dxCommon) {
	renderer_ = std::make_unique<SpriteRenderer> (dxCommon);
}

Sprite::~Sprite () {
}

void Sprite::Initialize (Vector3 position) {
	AdjustTextureSize ();
	//Transformの初期化
	TransformData_.Transform.scale = { size_.x, size_.y, 1.0f };
	TransformData_.Transform.rotate = { 0.0f, 0.0f, rotation_ };
	TransformData_.Transform.translate = position;

	//uvTransformの初期化
	TransformData_.uvTransform.scale = { 1.0f, 1.0f, 1.0f };
	TransformData_.uvTransform.rotate = {};
	TransformData_.uvTransform.translate = {};

	//wvpMatrixの初期化
	TransformData_.wvpMatrix = Math::MakeIdentity4x4 ();

	renderer_->Initialize ();
}

void Sprite::SetTexture (std::string ID) {
	id_ = ID;
	handle_ = TextureManager::GetInstance ()->GetTextureHandle(ID);
}

void Sprite::MakewvpMatrix () {
	Matrix4x4 world = Math::MakeAffineMatrix (TransformData_.Transform.scale, TransformData_.Transform.rotate, TransformData_.Transform.translate);
	Matrix4x4 view = Math::MakeIdentity4x4 ();
	Matrix4x4 proj = Math::MakeOrthographicMatrix (0, 0, 1280.0f, 720.0f, 0, 100.0f);
	TransformData_.wvpMatrix = Math::Multiply (world, Math::Multiply (view, proj));
}

void Sprite::Update () {
	MakewvpMatrix ();
	renderer_->Update (TransformData_.wvpMatrix, TransformData_.uvTransform,
					   anchorPoint_, isFlipX_, isFlipY_, id_, textureLeftTop_, textureSize_
	);
}

void Sprite::Draw () {
	renderer_->Draw (handle_);
}

void Sprite::ImGui () {
	renderer_->ImGui (TransformData_.Transform, TransformData_.uvTransform);
}

void Sprite::AdjustTextureSize () {
	const DirectX::TexMetadata& metadata = TextureManager::GetInstance ()->GetMetaData (id_);

	textureSize_.x = static_cast<float>(metadata.width);
	textureSize_.y = static_cast<float>(metadata.height);
	//画像サイズをテクスチャサイズに合わせる
	size_ = textureSize_;
}