#include "Skybox.h"
#include "TextureManager.h"
#include "MathFunction.h"
#include "RenderSystem.h"

const uint32_t kVertexNum = 8;
const uint32_t kIndexNum = 36;

Skybox::Skybox(DxCommon* dxCommon) 
	: BaseObject3d(dxCommon)
{
	dxCommon_ = dxCommon;

	vertexBuffer_ = std::make_unique<VertexBuffer<SkyboxVertex>>();
	indexBuffer_ = std::make_unique<IndexBuffer<uint32_t>>();
}

void Skybox::Initialize(const std::string& filePath) {
	BaseObject3d::Initialize();
	transform_ = { { 500.0f, 500.0f, 500.0f }, {}, {} };
	texInfo_.index = TextureManager::GetInstance()->LoadTexture(filePath);
	texInfo_.filePath = filePath;

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
	vertexBuffer_->Initialize(dxCommon_, vertices);
	vertexBuffer_->Update(vertices);

	// 2. インデックスデータの構築と初期化
	std::vector<uint32_t> indices = {
		0, 1, 2,  1, 3, 2, // 前
		5, 4, 7,  4, 6, 7, // 後ろ
		1, 5, 3,  5, 7, 3, // 右
		4, 0, 6,  0, 2, 6, // 左
		4, 5, 0,  5, 1, 0, // 上
		2, 3, 6,  3, 7, 6  // 下
	};
	indexBuffer_->Initialize(dxCommon_, kIndexNum);
	indexBuffer_->Update(indices);

	// 3. マテリアルデータの初期化
	materialData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_.enableLighting = FALSE; // スカイボックスは光らせない
	materialData_.uvTransform = Math::MakeIdentity4x4();
	materialData_.roughness = 0.0f;
	materialData_.metallic = 0.0f;
	materialBuffer_->Update(materialData_);

	// 4. PSOの設定
	psoDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // 背景なのでZバッファへの書き込みはOFF
	psoDesc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Skybox);
	psoDesc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Skybox.VS.hlsl", L"vs_6_0");
	psoDesc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/Skybox.PS.hlsl", L"ps_6_0");
	psoDesc_.InputLayoutID = InputLayoutType::Skybox;
	psoDesc_.BlendMode = BlendModeType::Opaque;
	layer_ = 0;

	renderType_ = RenderType::Skybox;
}

void Skybox::Update(CameraData* data) {
	if(data) {
		transform_.translate = data->transform.translate;
		transformMatrixData_.World = Math::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
		transformMatrixData_.WVP = Math::Multiply(transformMatrixData_.World, data->vp);
	}

	BaseObject3d::Update(data);
}

void Skybox::Draw() {
	RenderCommand cmd{};

	// パイプラインとPSO設定
	cmd.rootSignatureID = psoDesc_.RootSignatureID;
	cmd.psoDesc = psoDesc_;

	// メッシュビューのバインド
	cmd.vbViews[0] = vertexBuffer_->GetView();
	cmd.ibv = indexBuffer_->GetView();
	cmd.indexCount = kIndexNum;

	// 生成した各ConstantBufferの仮想アドレスをセット
	cmd.binds[0].type = BindingType::CBV;
	cmd.binds[0].gpuAddress = transformBuffer_->GetGPUVirtualAddress();

	cmd.binds[1].type = BindingType::CBV;
	cmd.binds[1].gpuAddress = materialBuffer_->GetGPUVirtualAddress();

	cmd.binds[2].type = BindingType::SRV_Table;
	cmd.binds[2].descriptorHandle = TextureManager::GetInstance()->GetTextureHandle(texInfo_.index);

	//不透明
	cmd.layer = layer_;

	// 描画タイプ
	cmd.renderType = renderType_;

	// レンダーシステムへ直行！
	RenderSystem::GetInstance()->PushCommand(cmd);
}