#include "mapChipRenderer.h"
#include "MathFunction.h"
#include "SRVManager.h"
#include "DxCommon.h"
#include "TextureManager.h"

//インスタンスバッファの最大確保数
const int kMaxChipNum = 2000;
const int kVertexNum = 24;
const int kIndexNum = 36;

void MapChipRenderer::Initialize() {
	// 種類リスト
	std::vector<MapChipType> types = { MapChipType::kFloor, MapChipType::kWall, MapChipType::kCeiling }; // 床・壁・天井
	
	for(auto type : types) {
		InstancingResource& res = resources_[type];

		//インスタンスバッファの作成
		res.resource = DxCommon::GetInstance()->CreateBufferResource(sizeof(MapforGPU) * kMaxChipNum);
		res.resource->Map(0, nullptr, reinterpret_cast<void**>(&res.mappedData));
		res.mappedData->wvp = Math::MakeIdentity4x4();
		res.mappedData->world = Math::MakeIdentity4x4();
		res.mappedData->color = { 1.0f, 1.0f, 1.0f, 1.0f };

		//SRVの作成
		uint32_t srvIndex = SRVManager::GetInstance()->Allocate();
		SRVManager::GetInstance()->CreateSRVStructuredBuffer(srvIndex, res.resource.Get(), kMaxChipNum, sizeof(MapforGPU));

		//SRVインデックスをどこかに保存しておく（描画時に使うため）
		res.srvIndex = srvIndex;
	}

	//他のリソースの初期化	
	//頂点バッファー作成とマッピング
	vertexBuffer_ = DxCommon::GetInstance()->CreateBufferResource(sizeof(VertexData) * kVertexNum);
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));
	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = sizeof(VertexData) * kVertexNum;
	vbView_.StrideInBytes = sizeof(VertexData);

	//インデックスバッファー作成とマッピング
	indexBuffer_ = DxCommon::GetInstance()->CreateBufferResource(sizeof(uint32_t) *kIndexNum);
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData_));
	ibView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	ibView_.SizeInBytes = sizeof(uint32_t) * kIndexNum;
	ibView_.Format = DXGI_FORMAT_R32_UINT;

	//マテリアルバッファー
	materialBuffer_ = DxCommon::GetInstance()->CreateBufferResource(sizeof(Material));
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = LightReflectionModel::HalfLambert;
	materialData_->uvTransform = Math::MakeIdentity4x4();

	// 立方体の1辺の長さの半分（中心から各頂点への距離）
	float w = kBlockWidth / 2.0f;
	float h = kBlockHeight / 2.0f;
	float d = kBlockWidth / 2.0f;

	// --- 頂点座標の定義（前面、背面、上面、下面、左面、右面） ---
	// 前面 (Z+)
	vertexData_[0] = { {-w,  h, -d, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f} };
	vertexData_[1] = { { w,  h, -d, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f} };
	vertexData_[2] = { {-w, -h, -d, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f} };
	vertexData_[3] = { { w, -h, -d, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f} };

	// 背面 (Z-)
	vertexData_[4] = { { w,  h,  d, 1.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f} };
	vertexData_[5] = { {-w,  h,  d, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f} };
	vertexData_[6] = { { w, -h,  d, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f} };
	vertexData_[7] = { {-w, -h,  d, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f} };

	// 上面 (Y+)
	vertexData_[8] = { {-w,  h, -d, 1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };
	vertexData_[9] = { { w,  h, -d, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };
	vertexData_[10] = { {-w, -h, -d, 1.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f} };
	vertexData_[11] = { { w, -h, -d, 1.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f} };

	// 下面 (Y-)
	vertexData_[12] = { { w,  h,  d, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f} };
	vertexData_[13] = { {-w,  h,  d, 1.0f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f} };
	vertexData_[14] = { { w, -h,  d, 1.0f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f} };
	vertexData_[15] = { {-w, -h,  d, 1.0f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f} };

	// 左面 (X-)
	vertexData_[16] = { {-w,  h, -d, 1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} };
	vertexData_[17] = { { w,  h, -d, 1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f} };
	vertexData_[18] = { {-w, -h, -d, 1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} };
	vertexData_[19] = { { w, -h, -d, 1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f} };

	// 右面 (X+)
	vertexData_[20] = { { w,  h,  d, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f} };
	vertexData_[21] = { {-w,  h,  d, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f} };
	vertexData_[22] = { { w, -h,  d, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f} };
	vertexData_[23] = { {-w, -h,  d, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f} };

	//インデックス割り当て
	// 各面共通のオフセット
	for(uint32_t i = 0; i < 6; ++i) {
		uint32_t vOff = i * 4; // 頂点のオフセット
		uint32_t iOff = i * 6; // インデックスのオフセット

		indexData_[iOff + 0] = vOff + 0;
		indexData_[iOff + 1] = vOff + 1;
		indexData_[iOff + 2] = vOff + 2;

		indexData_[iOff + 3] = vOff + 1;
		indexData_[iOff + 4] = vOff + 3;
		indexData_[iOff + 5] = vOff + 2;
	}
	
	//PSO設定
	desc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Mapchip);
	desc_.VS_ID = ShaderManager::GetInstance()->CompileAndCasheShader(L"Resources/shader/Particle.VS.hlsl", L"vs_6_0");
	desc_.PS_ID = ShaderManager::GetInstance()->CompileAndCasheShader(L"Resources/shader/Particle.PS.hlsl", L"ps_6_0");
	desc_.InputLayoutID = InputLayoutType::Particle;
	desc_.BlendMode = BlendModeType::Opaque;
	desc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;	//三角形で描画
	PSOManager::GetInstance()->GetOrCreratePSO(desc_);
}

void MapChipRenderer::Update(MapChip& mapChip, const Matrix4x4& vp) {
	//前フレームのデータをクリア
	for(auto& [type, resources] : resources_) {
		resources.cpuData.clear();
	}

	//マップをループで回してデータを集める
	for(uint32_t y = 0; y < kNumBlockVirtical; ++y) {
		for(uint32_t x = 0; x < kNumBlockHorizontal; ++x) {
			MapChipType type = mapChip.GetMapChipTypeByIndex(x, y);
			if(type == MapChipType::kBlank) continue;

			// 座標を取得してワールド行列を作成
			Vector3 pos = mapChip.GetMapChipPositionByIndex(x, y);
			Matrix4x4 world = Math::MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f }, pos);
			Matrix4x4 wvp = world * vp;

			// 対応する種類の vector に追加
			resources_[type].cpuData.push_back({ wvp, world, {1.0f, 1.0f, 1.0f, 1.0f} });
		}
	}

	//memcpyでGPUに転送する
	for(auto& [type, res] : resources_) {
		if(res.cpuData.empty()) continue;
		std::memcpy(res.mappedData, res.cpuData.data(), sizeof(MapforGPU) * res.cpuData.size());
	}
}

void MapChipRenderer::Draw() {
	//PSOとプリミティブトポロジをセット
	auto commandList = DxCommon::GetInstance()->GetCommandList();
	PSOManager::GetInstance()->SetPSO(desc_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//共通の頂点バッファとインデックスバッファをセット
	commandList->IASetVertexBuffers(0, 1, &vbView_);
	commandList->IASetIndexBuffer(&ibView_);

	//種類ごとに描画命令を出す
	for(auto& [type, res] : resources_) {
		if(res.cpuData.empty()) continue; // データがない種類は飛ばす

		//インスタンスバッファ(SRV)をセット
		SRVManager::GetInstance()->SetGraphicsRootDescriptorTable(0, res.srvIndex);

		//テクスチャをセット
		if(type == MapChipType::kFloor) {
			commandList->SetGraphicsRootDescriptorTable(1, TextureManager::GetInstance()->TextureManager::GetTextureHandle("map1"));
		}
		else if(type == MapChipType::kWall) {
			commandList->SetGraphicsRootDescriptorTable(1, TextureManager::GetInstance()->TextureManager::GetTextureHandle("map2"));
		}
		else if(type == MapChipType::kCeiling) {
			commandList->SetGraphicsRootDescriptorTable(1, TextureManager::GetInstance()->TextureManager::GetTextureHandle("map3"));
		}

		commandList->SetGraphicsRootConstantBufferView(2, materialBuffer_->GetGPUVirtualAddress());

		//インスタンシング描画
		commandList->DrawIndexedInstanced(kIndexNum, (UINT)res.cpuData.size(), 0, 0, 0);
	}
}