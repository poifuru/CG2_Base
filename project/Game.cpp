#include "Game.h"
#include "CameraOrganizer.h"
#include "BaseScene.h"
#include "SceneType.h"

#include <cassert>

// 描画システム関連のヘッダー
#include "RenderSystem.h"
#include "ShaderManager.h"
#include "PSOManager.h"

Game::Game() {
	engine_ = std::make_unique<Engine>();
	engine_->Initialize();
	/*std::unique_ptr<BaseScene> scene = std::make_unique<PlayScene>();
	sceneManager_ = SceneManager::GetInstance();
	sceneManager_->SetNextScene(std::move(scene));*/

	// 1. 各種ゲッターをEngineから通して生オブジェクトを取得
	ID3D12Device* device = engine_->GetDevice();
	auto heapManager = engine_->GetHeapManager();
	auto shaderManager = engine_->GetShaderManager();

	// 2. シェーダーのコンパイルとキャッシュ登録
	// ※パスはプロジェクトの配置に合わせて調整してね
	vsID_ = shaderManager->CompileAndCacheShader(L"Resources/shader/Triangle.VS.hlsl", L"vs_6_0");
	psID_ = shaderManager->CompileAndCacheShader(L"Resources/shader/Triangle.PS.hlsl", L"ps_6_0");

	// 3. 頂点バッファ・インデックスバッファの初期化（3頂点分）
	vertexBuffer_.Initialize(device, 3);
	indexBuffer_.Initialize(device, 3);

	// 三角形の頂点データ（正面を向いた三角形）
	std::vector<Vertex> vertices = {
		{ {  0.0f,  0.5f, 0.0f, 1.0f }, { 0.5f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
		{ {  0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
		{ { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } }
	};
	std::vector<uint32_t> indices = { 0, 1, 2 };

	vertexBuffer_.Update(vertices);
	indexBuffer_.Update(indices);

	// 4. 【ここがバインドレス！】構造化バッファ（マテリアルの本棚）の初期化
	// 今回は1個分のマテリアル領域を確保
	materialBuffer_.Initialize(device, *heapManager, 1);

	// 赤オレンジ色っぽいマテリアルを設定（テクスチャはまだ使わないので0）
	std::vector<MaterialData> materials = {
		{ { 1.0f, 0.5f, 0.2f, 1.0f }, 0, {0, 0, 0} }
	};
	materialBuffer_.Update(materials);
}

Game::~Game() {
	// 終了時にバッファを手動解放（Engineが消えるより前に消す）
	vertexBuffer_.Release();
	indexBuffer_.Release();
	materialBuffer_.Release();
}

void Game::Run() {
	/*メインループ！！！！！！！！！*/
	//ウィンドウの×ボタンが押されるまでループ
	while(true) {

		if(engine_->ProcessMessage()) {
			break;
		}

		//フレーム開始
		engine_->BeginFrame();

		//***更新処理***//
		//sceneManager_->Update();
		//*************//

		// ====================================================
		// ★ 描画コマンドの組み立てと積み込み（ベタ書き）
		// ====================================================
		RenderCommand cmd{};

		// パイプライン状態の設定（PSOManagerに送る記述情報）
		cmd.psoDesc.VS_ID = vsID_;
		cmd.psoDesc.PS_ID = psID_;
		cmd.psoDesc.InputLayoutID = InputLayoutType::Standard3D;
		cmd.psoDesc.BlendMode = BlendModeType::Opaque;
		cmd.psoDesc.CullMode = D3D12_CULL_MODE_NONE; // 両面見えるようにカリングなし
		cmd.psoDesc.DepthEnable = FALSE;
		cmd.psoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN;

		// 三角形のバッファビューを設定
		cmd.vbView = vertexBuffer_.GetView();
		cmd.ibv = indexBuffer_.GetView();
		cmd.indexCount = 3;

		// ★バインドレスの核心：
		// 構造化バッファ自身が記憶している「ヒープ内のアロケーションインデックス」を引き渡す！
		cmd.materialIndex = materialBuffer_.GetDescriptorIndex();
		cmd.textureIndex = 0; // 今回は未使用
		cmd.layer = 0;        // 不透明

		// レンダーシステムにコマンドを積む！
		engine_->GetRenderSystem()->PushCommand(cmd);
		// ====================================================

		//***描画処理***//
		//sceneManager_->Draw();
		//*************//

		//フレーム終了
		engine_->EndFrame();
	}
}