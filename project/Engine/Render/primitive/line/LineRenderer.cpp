//#include "LineRenderer.h"
//#include "MathFunction.h"
//#include "MaxMeshNum.h"
//#include "DxCommon.h"
//
//LineRenderer::~LineRenderer () {
//
//}
//
//void LineRenderer::Initialize (DxCommon* dxCommon) {
//	dxCommon_ = dxCommon;
//	device_ = dxCommon->GetDevice ();
//	commandList_ = dxCommon->GetCommandList ();
//	srvManager_ = SRVManager::GetInstance();
//
//	lineBuffer_ = std::make_unique<LineVertexData> ();
//
//	//頂点バッファー作成とマッピング
//	lineBuffer_->vertexBuffer = dxCommon_->CreateBufferResource (sizeof (LineData) * VertexNum::Line * MaxMeshNum::Line);
//	lineBuffer_->vertexBuffer->Map (0, nullptr, reinterpret_cast<void**>(&vertexData_));
//	lineBuffer_->vbView.BufferLocation = lineBuffer_->vertexBuffer->GetGPUVirtualAddress ();
//	lineBuffer_->vbView.SizeInBytes = sizeof (LineData) * VertexNum::Line * MaxMeshNum::Line;
//	lineBuffer_->vbView.StrideInBytes = sizeof (LineData);
//
//	//行列バッファー作成とマッピング(頂点2つにつき1つ)
//	instancingBuffer_ = dxCommon_->CreateBufferResource (sizeof (LineForGPU) * MaxMeshNum::Line);
//	instancingBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&instancingData_));
//	for (uint32_t i = 0; i < MaxMeshNum::Line; ++i) {
//		instancingData_[i].World = Math::MakeIdentity4x4 ();
//		instancingData_[i].WVP = Math::MakeIdentity4x4 ();
//	}
//
//	//使うインデックスを確保
//	instancingIndex_ = srvManager_->Allocate();
//	vertexIndex_ = srvManager_->Allocate();
//
//	//インスタンシング用のSRV作成
//	srvManager_->CreateSRVStructuredBuffer(instancingIndex_, instancingBuffer_.Get(), MaxMeshNum::Line, sizeof(LineForGPU));
//
//	//頂点バッファ用のSRV作成
//	srvManager_->CreateSRVStructuredBuffer(vertexIndex_, lineBuffer_->vertexBuffer.Get(), VertexNum::Line * MaxMeshNum::Line, sizeof(LineData));
//
//	//PSOの設定
//	desc_.RootSignatureID = RootSignatureManager::GetInstance ()->GetOrCreateRootSignature (RootSigType::LineMesh);
//	desc_.VS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/Line.VS.hlsl", L"vs_6_0");
//	desc_.PS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/Line.PS.hlsl", L"ps_6_0");
//	desc_.InputLayoutID = InputLayoutType::LineMesh;
//	desc_.BlendMode = BlendModeType::Opaque;
//	desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;	//線を描画
//	desc_.DepthEnable = FALSE; // モデルに隠されないようにZテストを切る
//	PSOManager::GetInstance ()->GetOrCreatePSO (desc_);
//}
//
//void LineRenderer::UpdateVertexData (const LineData* data) {
//	//currentLineNumが最大数を超えていないか
//	if (currentLineCount_ >= MaxMeshNum::Line) {
//		//超えてたら早期リターン
//		return;
//	}
//
//	// 現在の描画位置のオフセット (線1本 = 2頂点)
//	uint32_t offset = currentLineCount_ * VertexNum::Line;
//
//	//オフセットを使ってライン描画に必要な分だけバッファーにコピー
//	vertexData_[offset + 0] = data[0];
//	vertexData_[offset + 1] = data[1];
//
//	//線の数をインクリメント
//	currentLineCount_++;
//}
//
//void LineRenderer::TransferData (const LineForGPU& data) {
//	//描画要求数が0か最大数を超えてるなら
//	if (currentLineCount_ == 0 || currentLineCount_ > MaxMeshNum::Line) {
//		//早期リターン
//		return;
//	}
//
//	// 書き込むインスタンス配列のインデックス
//	uint32_t instanceIndex = currentLineCount_ - 1;
//
//	instancingData_[instanceIndex].World = data.World;
//	instancingData_[instanceIndex].WVP = data.WVP;
//}
//
//void LineRenderer::Draw () {
//	//RootSignatureとPSOをセット
//	RootSignatureManager::GetInstance ()->SetRootSignature (desc_.RootSignatureID);
//	PSOManager::GetInstance ()->SetPSO (desc_);
//	commandList_->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_LINELIST);							//線で描画									//VBVを設定
//	commandList_->SetGraphicsRootConstantBufferView (0, instancingBuffer_->GetGPUVirtualAddress ());	//CBVをセット
//	commandList_->SetGraphicsRootDescriptorTable (1, srvManager_->GetGPUDescriptorHandle(instancingIndex_));
//
//	if (currentLineCount_ > 0) {
//		// 描画
//		commandList_->DrawInstanced (VertexNum::Line, currentLineCount_, 0, 0); // 描画要求があった頂点数だけ描画
//	}
//
//	//次フレームのためにカウントをリセット
//	ResetCurrentLineCount ();
//}