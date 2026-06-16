#include <imgui.h>
#include <numbers>
#include "Particle.h"
#include "DxCommon.h"
#include "MathFunction.h"

//デルタタイムを定義
const float kDeltaTime = 1.0f / 60.0f;

Particle::Particle (DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
	device_ = dxCommon->GetDevice ();
	commandList_ = dxCommon->GetCommandList ();
	srvManager_ = SRVManager::GetInstance();
	data_ = std::make_unique<ModelData> ();
	uvTransform_ = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
	//乱数エンジンのインスタンスを作成してrdの結果で初期化する
	randomEngine_.seed (rd ());
	emitter_.count = 10;
	emitter_.frequency = 0.3f;
	emitter_.frequencyTime = 0.0f;
}

Particle::~Particle () {

}

void Particle::Initialize () {
	//頂点バッファ作成
	data_->vertexBuffer = dxCommon_->CreateBufferResource (sizeof (VertexData) * 4);
	data_->vertexBuffer->Map (0, nullptr, reinterpret_cast<void**>(&vertexData_));
	data_->vbView.BufferLocation = data_->vertexBuffer->GetGPUVirtualAddress ();
	data_->vbView.SizeInBytes = sizeof (VertexData) * 4;
	data_->vbView.StrideInBytes = sizeof (VertexData);

	//インデックスバッファ作成
	data_->indexBuffer = dxCommon_->CreateBufferResource (sizeof (uint32_t) * 6);
	data_->indexBuffer->Map (0, nullptr, reinterpret_cast<void**>(&indexData_));
	data_->ibView.BufferLocation = data_->indexBuffer->GetGPUVirtualAddress ();
	data_->ibView.SizeInBytes = sizeof (uint32_t) * 6;
	data_->ibView.Format = DXGI_FORMAT_R32_UINT;

	//Instancing用の行列データ
	instancingBuffer_ = dxCommon_->CreateBufferResource (sizeof (ParticleForGPU) * kMaxParticleNum_);
	instancingBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&instancingData_));
	//マテリアルデータ
	materialBuffer_ = dxCommon_->CreateBufferResource (sizeof (Material) * kMaxParticleNum_);
	materialBuffer_->Map (0, nullptr, reinterpret_cast<void**> (&materialData_));
	for (uint32_t i = 0; i < kMaxParticleNum_; ++i) {
		instancingData_[i].WVP = Math::MakeIdentity4x4 ();
		instancingData_[i].World = Math::MakeIdentity4x4 ();
		instancingData_[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		materialData_[i].color = { 1.0f, 1.0f, 1.0f, 1.0f };
		materialData_[i].enableLighting = LightReflectionModel::None;
		materialData_[i].uvTransform = Math::MakeIdentity4x4 ();
	}

	//vertexData_に書き込み
	//左上
	vertexData_[0] = { { -1.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	//右上
	vertexData_[1] = { { 1.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
	//左下
	vertexData_[2] = { { -1.0f, -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };
	//右下
	vertexData_[3] = { { 1.0f, -1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } };

	//indexData_に書き込み
	indexData_[0] = 0;
	indexData_[1] = 1;
	indexData_[2] = 2;
	indexData_[3] = 1;
	indexData_[4] = 3;
	indexData_[5] = 2;

	//PSO設定
	desc_.RootSignatureID = RootSignatureManager::GetInstance ()->GetOrCreateRootSignature (RootSigType::Particle);
	desc_.VS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/Particle.VS.hlsl", L"vs_6_0");
	desc_.PS_ID = ShaderManager::GetInstance ()->CompileAndCacheShader (L"Resources/shader/Particle.PS.hlsl", L"ps_6_0");
	desc_.InputLayoutID = InputLayoutType::Particle;
	desc_.BlendMode = BlendModeType::Additive;
	desc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	//Depthの書き込みを行わない
	desc_.DepthEnable = FALSE;								//デプステストを無効化

	UINT useIndex = srvManager_->Allocate();

	srvManager_->CreateSRVStructuredBuffer(useIndex, instancingBuffer_.Get(), kMaxParticleNum_, sizeof(ParticleForGPU));

	particleSrvHandleCPU = srvManager_->GetCPUDescriptorHandle (useIndex);
	particleSrvHandleGPU = srvManager_->GetGPUDescriptorHandle (useIndex);

	billBoardMatrix_ = Math::MakeIdentity4x4 ();
}

void Particle::Update (Matrix4x4* cameraMatrix, Matrix4x4* vp) {
	// カメラのワールド位置を取得
	Vector3 cameraPos = { cameraMatrix->m[3][0], cameraMatrix->m[3][1], cameraMatrix->m[3][2] };

	//Emitter更新
	if (isMarineSnow_) {
		EmitterUpdateMarineSnow (cameraPos);
	}
	else {
		EmitterUpdate ();
	}

	//numInstanceのリセット
	numInstance_ = 0;
	uint32_t dstIndex = 0; //書き込み先インデックス
	for (particleIterator_ = particles_.begin (); particleIterator_ != particles_.end ();) {
		// 生存可能時間を過ぎていたら更新処理をしない (マリンスノー以外)
		if (!isMarineSnow_ && particleIterator_->lifeTime <= particleIterator_->currentTime) {
			particleIterator_ = particles_.erase (particleIterator_);	//生存時間をすぎたパーティクルはリストから削除
			continue;
		}

		if (dstIndex < kMaxParticleNum_) {
			// 時間を進める
			particleIterator_->currentTime += kDeltaTime;

			if (isMarineSnow_) {
				// Y方向は落下、X, Z方向はゆっくりサイン・コサイン波でゆらゆらさせる
				particleIterator_->transform.translate.y += particleIterator_->velocity.y * kDeltaTime;
				particleIterator_->transform.translate.x += sinf (particleIterator_->currentTime * marineSnowDriftSpeed_) * marineSnowDriftScale_ * kDeltaTime;
				particleIterator_->transform.translate.z += cosf (particleIterator_->currentTime * marineSnowDriftSpeed_) * marineSnowDriftScale_ * kDeltaTime;

				// カメラ周囲の範囲外に出た場合のラッピング処理
				float diffX = particleIterator_->transform.translate.x - cameraPos.x;
				float diffY = particleIterator_->transform.translate.y - cameraPos.y;
				float diffZ = particleIterator_->transform.translate.z - cameraPos.z;

				if (diffX < -marineSnowRange_.x) particleIterator_->transform.translate.x += marineSnowRange_.x * 2.0f;
				else if (diffX > marineSnowRange_.x) particleIterator_->transform.translate.x -= marineSnowRange_.x * 2.0f;

				if (diffY < -marineSnowRange_.y) particleIterator_->transform.translate.y += marineSnowRange_.y * 2.0f;
				else if (diffY > marineSnowRange_.y) particleIterator_->transform.translate.y -= marineSnowRange_.y * 2.0f;

				if (diffZ < -marineSnowRange_.z) particleIterator_->transform.translate.z += marineSnowRange_.z * 2.0f;
				else if (diffZ > marineSnowRange_.z) particleIterator_->transform.translate.z -= marineSnowRange_.z * 2.0f;
			}
			else {
				// 通常の速度反映
				particleIterator_->transform.translate += particleIterator_->velocity * kDeltaTime;
			}

			instancingData_[dstIndex].World = Math::MakeAffineMatrix (
				particleIterator_->transform.scale,
				particleIterator_->transform.rotate,
				particleIterator_->transform.translate
			);

			//ビルボードフラグが立っていたら
			if (useBillBoard) {
				//カメラのWorld行列の回転成分だけを使う
				billBoardMatrix_ = *cameraMatrix;
				billBoardMatrix_.m[3][0] = 0.0f;	//平行移動成分はいらない
				billBoardMatrix_.m[3][1] = 0.0f;
				billBoardMatrix_.m[3][2] = 0.0f;
				billBoardMatrix_.m[3][3] = 1.0f;

				instancingData_[dstIndex].World = Math::Multiply (instancingData_[dstIndex].World, billBoardMatrix_);
				instancingData_[dstIndex].WVP = Math::Multiply (instancingData_[dstIndex].World, *vp);
			}
			else {
				instancingData_[dstIndex].WVP = Math::Multiply (instancingData_[dstIndex].World, *vp);
			}

			// 色と不透明度(アルファ)の計算
			instancingData_[dstIndex].color = particleIterator_->color;

			if (isMarineSnow_) {
				// カメラからの距離
				float dist = Math::Length (Math::Subtract (particleIterator_->transform.translate, cameraPos));
				float alpha = particleIterator_->color.w;

				// 1. 近接フェード (カメラに近すぎると消える)
				if (dist < marineSnowNearFadeLimit_) {
					alpha *= (dist / marineSnowNearFadeLimit_);
				}

				// 2. 境界フェード (存在範囲の端に近づくと消える。ラッピング時のチラつき防止)
				float distRatioX = fabsf (particleIterator_->transform.translate.x - cameraPos.x) / marineSnowRange_.x;
				float distRatioY = fabsf (particleIterator_->transform.translate.y - cameraPos.y) / marineSnowRange_.y;
				float distRatioZ = fabsf (particleIterator_->transform.translate.z - cameraPos.z) / marineSnowRange_.z;
				float maxRatio = (std::max) ( (std::max) (distRatioX, distRatioY), distRatioZ);
				if (maxRatio > 0.8f) {
					alpha *= (1.0f - maxRatio) / 0.2f;
				}

				instancingData_[dstIndex].color.w = alpha;
			}
			else {
				// 通常のパーティクルは寿命で消える
				float alpha = 1.0f - (particleIterator_->currentTime / particleIterator_->lifeTime);
				instancingData_[dstIndex].color.w = alpha;
			}

			//uvTransform更新
			materialData_[dstIndex].uvTransform = Math::MakeAffineMatrix (
				uvTransform_.scale,
				uvTransform_.rotate,
				uvTransform_.translate);
		}

		//書き込み先インデックスをインクリメント
		++dstIndex;
		++particleIterator_;
	}
	numInstance_ = dstIndex;
}

void Particle::Draw () {
	RootSignatureManager::GetInstance ()->SetRootSignature (desc_.RootSignatureID);
	PSOManager::GetInstance ()->SetPSO (desc_);
	commandList_->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList_->IASetVertexBuffers (0, 1, &data_->vbView);   //VBVを設定
	commandList_->IASetIndexBuffer (&data_->ibView);	        //IBVを設定
	commandList_->SetGraphicsRootDescriptorTable (0, particleSrvHandleGPU);
	commandList_->SetGraphicsRootConstantBufferView (1, materialBuffer_->GetGPUVirtualAddress ());
	commandList_->SetGraphicsRootDescriptorTable (2, handle_);
	//インデックスバッファを使った描画
	commandList_->DrawIndexedInstanced (6, numInstance_, 0, 0, 0);
}

void Particle::ImGui () {
#ifdef USEIMGUI
	//BlendMode切り替え
	ImGui::Begin ("Particle");
	if (ImGui::Combo ("BlendMode", &currentBlendMode_, blendModeNames_, kBlendModeCount_)) {
		desc_.BlendMode = static_cast<BlendModeType>(currentBlendMode_);
	}

	if (ImGui::Button ("spawn")) {
		particles_.splice (particles_.end (), Emit (emitter_, randomEngine_));
	}

	ImGui::Checkbox ("useBillBoard", &useBillBoard);

	ImGui::Separator ();

	// マリンスノーモードの切り替え
	bool prevMarineSnow = isMarineSnow_;
	if (ImGui::Checkbox ("isMarineSnow", &isMarineSnow_)) {
		if (prevMarineSnow != isMarineSnow_) {
			particles_.clear (); // モードが切り替わったらパーティクルをリセット
		}
	}

	if (isMarineSnow_) {
		ImGui::Text ("Marine Snow Settings");
		ImGui::DragFloat3 ("Range", &marineSnowRange_.x, 0.1f, 1.0f, 100.0f);
		ImGui::DragFloat ("Fall Speed", &marineSnowFallSpeed_, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat ("Drift Speed", &marineSnowDriftSpeed_, 0.01f, 0.0f, 10.0f);
		ImGui::DragFloat ("Drift Scale", &marineSnowDriftScale_, 0.01f, 0.0f, 5.0f);
		ImGui::DragFloat ("Near Fade Limit", &marineSnowNearFadeLimit_, 0.1f, 0.0f, 20.0f);
		ImGui::DragFloat ("Min Size", &marineSnowMinSize_, 0.005f, 0.001f, marineSnowMaxSize_);
		ImGui::DragFloat ("Max Size", &marineSnowMaxSize_, 0.005f, marineSnowMinSize_, 5.0f);
	}
	else {
		ImGui::DragFloat3 ("scale", &emitter_.transform.scale.x, 0.01f, 0.0f, 100.f);
		ImGui::DragFloat3 ("rotate", &emitter_.transform.rotate.x, 0.01f, -100.0f, 100.f);
		ImGui::DragFloat3 ("translate", &emitter_.transform.translate.x, 0.01f, -100.0f, 100.f);
	}

	ImGui::End ();
#endif
}

ParticleData Particle::MakeNewParticle (std::mt19937 randomEngine, const Emitter& emitter_) {
	//乱数エンジンのインスタンスを作成してrdの結果で初期化する
	randomEngine.seed (rd ());

	ParticleData data;

	//使う分布を初期化する
	pos_x = std::uniform_real_distribution<float> (-emitter_.transform.scale.x, emitter_.transform.scale.x);
	pos_y = std::uniform_real_distribution<float> (-emitter_.transform.scale.y, emitter_.transform.scale.y);
	pos_z = std::uniform_real_distribution<float> (-emitter_.transform.scale.z, emitter_.transform.scale.z);
	rand_ = std::uniform_real_distribution<float> (-1.0f, 1.0f);
	randColor_ = std::uniform_real_distribution<float> (0.0f, 1.0f);
	randTime_ = std::uniform_real_distribution<float> (1.0f, 3.0f);

	//パーティクル情報の初期化
	data.transform.scale = { 1.0f, 1.0f, 1.0f };
	data.transform.rotate = { 0.0f, 0.0f, 0.0f };
	data.transform.translate = { pos_x (randomEngine), pos_y (randomEngine), pos_z (randomEngine) };
	data.velocity = { rand_ (randomEngine), rand_ (randomEngine), rand_ (randomEngine) };
	data.color = { randColor_ (randomEngine), randColor_ (randomEngine), randColor_ (randomEngine), 1.0f };
	data.lifeTime = randTime_ (randomEngine);
	data.currentTime = 0.0f;

	//emitterを加味してtranslateを再計算
	data.transform.translate += emitter_.transform.translate;

	return data;
}

std::list<ParticleData> Particle::Emit (const Emitter& emitter, std::mt19937& randomEngine) {
	std::list<ParticleData> particles;
	for (uint32_t count = 0; count < emitter.count; ++count) {
		particles.push_back (MakeNewParticle (randomEngine, emitter_));
	}
	return particles;
}

void Particle::EmitterUpdate () {
	emitter_.frequencyTime += kDeltaTime;	//発生時刻を進める
	if (emitter_.frequency <= emitter_.frequencyTime) {		//頻度より大きいなら
		particles_.splice (particles_.end (), Emit (emitter_, randomEngine_));	//particle発生
		emitter_.frequencyTime -= emitter_.frequency;	//進めた時間を戻す
	}
}

ParticleData Particle::MakeNewMarineSnow (std::mt19937 randomEngine, const Vector3& cameraPos) {
	randomEngine.seed (rd ());

	ParticleData data;

	// 使う分布を初期化する (カメラ位置を中心とした範囲)
	std::uniform_real_distribution<float> snow_x (-marineSnowRange_.x, marineSnowRange_.x);
	std::uniform_real_distribution<float> snow_y (-marineSnowRange_.y, marineSnowRange_.y);
	std::uniform_real_distribution<float> snow_z (-marineSnowRange_.z, marineSnowRange_.z);
	std::uniform_real_distribution<float> scale_rand (0.05f, 0.2f); // 粒の大きさ
	std::uniform_real_distribution<float> speed_y_offset (-0.1f, 0.1f); // 落下速度のばらつき
	std::uniform_real_distribution<float> color_rand (0.8f, 1.0f);
	std::uniform_real_distribution<float> time_offset (0.0f, 100.0f); // サイン波の位相ズレ用

	float size = scale_rand (randomEngine);
	data.transform.scale = { size, size, size };
	data.transform.rotate = { 0.0f, 0.0f, 0.0f };
	data.transform.translate = {
		cameraPos.x + snow_x (randomEngine),
		cameraPos.y + snow_y (randomEngine),
		cameraPos.z + snow_z (randomEngine)
	};
	// Y方向は設定落下速度 + ランダムな揺らぎ
	data.velocity = { 0.0f, -(marineSnowFallSpeed_ + speed_y_offset (randomEngine)), 0.0f };
	// マリンスノーらしい白〜少し薄水色のランダム
	float r = color_rand (randomEngine);
	float g = color_rand (randomEngine);
	data.color = { r, (r + g) * 0.5f, 1.0f, 0.8f }; // 青寄りの白

	data.lifeTime = 999999.0f; // ほぼ無限寿命（ラッピングでループさせる）
	// currentTime を位相ズレ（シード）用としてランダムにずらしておく
	data.currentTime = time_offset (randomEngine);

	return data;
}

void Particle::EmitterUpdateMarineSnow (const Vector3& cameraPos) {
	// 最大数に達するまでマリンスノーを生成して追加
	while (particles_.size() < kMaxParticleNum_) {
		particles_.push_back (MakeNewMarineSnow (randomEngine_, cameraPos));
	}
}