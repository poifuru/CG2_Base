#include "Skybox.h"
#include "TextureManager.h"
#include "MathFunction.h"
#include "RenderSystem.h"

const uint32_t kVertexNum = 8;
const uint32_t kIndexNum = 36;

Skybox::Skybox(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	device_ = dxCommon->GetDevice();
	commandList_ = dxCommon->GetCommandList();
}

void Skybox::Initialize(const std::string& textureTag) {
	tag_ = textureTag;

	matrixBuffer_.Initialize(dxCommon_);
	materialBuffer_.Initialize(dxCommon_);

	// 1. 頂点データの構築と初期化
	std::vector<SkyboxVertex> vertices(kVertexNum);
	vertices[0].position = { -1.0f,  1.0f,  1.0f, 1.0f };
	vertices[1].position = { 1.0f,  1.0f,  1.0f, 1.0f };
	vertices[2].position = { -1.0f, -1.0f,  1.0f, 1.0f };
	vertices[3].position = { 1.0f, -1.0f,  1.0f, 1.0f };
	vertices[4].position = { -1.0f,  1.0f, -1.0f, 1.0f };
	vertices[5].position = { 1.0f,  1.0f, -1.0f, 1.0f };
	vertices[6].position = { -1.0f, -1.0f, -1.0f, 1.0f };
	vertices[7].position = { 1.0f, -1.0f, -1.0f, 1.0f };
	vertexBuffer_.Initialize(dxCommon_, vertices);

	// 2. インデックスデータの構築と初期化
	std::vector<uint32_t> indices = {
		0, 1, 2,  1, 3, 2, // 前
		5, 4, 7,  4, 6, 7, // 後ろ
		1, 5, 3,  5, 7, 3, // 右
		4, 0, 6,  0, 2, 6, // 左
		4, 5, 0,  5, 1, 0, // 上
		2, 3, 6,  3, 7, 6  // 下
	};
	indexBuffer_.Initialize(dxCommon_, kIndexNum);
	indexBuffer_.Update(indices);

	// 3. マテリアルデータの初期化
	cpuMaterialData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	cpuMaterialData_.enableLighting = FALSE; // スカイボックスは光らせない
	cpuMaterialData_.uvTransform = Math::MakeIdentity4x4();
	cpuMaterialData_.roughness = 0.0f;
	cpuMaterialData_.metallic = 0.0f;
	materialBuffer_.Update(cpuMaterialData_);

	// 4. PSOの設定
	psoDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 背景なのでZバッファへの書き込みはOFF
	psoDesc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Skybox);
	psoDesc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Skybox.VS.hlsl", L"vs_6_0");
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Skybox.PS.hlsl", L"ps_6_0");
	psoDesc_.InputLayoutID = InputLayoutType::Skybox;
	psoDesc_.BlendMode = BlendModeType::Opaque;
}

void Skybox::Update(CameraData* data) {
	EulerTransform transform = { { 5000.0f, 5000.0f, 5000.0f }, {}, {} };
	cpuTransformData_.World = Math::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);

	if(data) {
		cpuTransformData_.WVP = Math::Multiply(cpuTransformData_.World, data->vp);
	}

	// GPUバッファへデータ更新をコミット
	matrixBuffer_.Update(cpuTransformData_);
}

void Skybox::Draw() {
	RenderCommand cmd{};

	// パイプラインとPSO設定
	cmd.rootSignatureID = psoDesc_.RootSignatureID;
	cmd.psoDesc = psoDesc_;

	// メッシュビューのバインド
	cmd.vbViews[0] = vertexBuffer_.GetView();
	cmd.ibv = indexBuffer_.GetView();
	cmd.indexCount = kIndexNum;

	// 生成した各ConstantBufferの仮想アドレスをセット
	cmd.transformCBV = matrixBuffer_.GetGPUVirtualAddress();
	cmd.materialCBV = materialBuffer_.GetGPUVirtualAddress();

	// キューブマップ用テクスチャハンドルをパラメータ2（あるいは指定スロット）へセット
	cmd.textureSRV = TextureManager::GetInstance()->GetTextureHandle(tag_);

	cmd.layer = 0;

	// レンダーシステムへ直行！
	RenderSystem::GetInstance()->PushCommand(cmd);
}