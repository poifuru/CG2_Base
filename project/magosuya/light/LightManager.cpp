#include "LightManager.h"
#include "SRVManager.h"
#include "imgui.h"

//ライトの最大数
const uint32_t MaxCount = 20;

LightManager::LightManager(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
}

LightManager::~LightManager() {
}

void LightManager::Initialize() {
	//バッファー確保
	lightCountBuffer_ = dxCommon_->CreateBufferResource(sizeof(LightCount));
	//書き込むアドレスを取得してマッピング
	lightCountBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&lightCountData_));

	//バッファー確保
	dirLightBuffer_ = dxCommon_->CreateBufferResource(sizeof(DirectionalLightForGPU) * MaxCount);
	//書き込むアドレスを取得してマッピング
	dirLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&dirLightData_));

	//バッファー確保
	pointLightBuffer_ = dxCommon_->CreateBufferResource(sizeof(PointLightForGPU) * MaxCount);
	//書き込むアドレスを取得してマッピング
	pointLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));

	//バッファー確保
	spotLightBuffer_ = dxCommon_->CreateBufferResource(sizeof(SpotLightForGPU) * MaxCount);
	//書き込むアドレスを取得してマッピング
	spotLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));

	//DierctionalLightのSRVを作る
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // 構造化バッファの場合はUNKNOWN
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = MaxCount; // 最大数
	srvDesc.Buffer.StructureByteStride = sizeof(DirectionalLightForGPU); // 1要素のサイズ
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	dirLightSRVIndex_ = SRVManager::GetInstance()->Allocate();
	SRVManager::GetInstance()->CreateSRVStructuredBuffer(dirLightSRVIndex_, dirLightBuffer_.Get(), MaxCount, sizeof(DirectionalLightForGPU));

	//PointLight用のSRV作成
	srvDesc.Buffer.StructureByteStride = sizeof(PointLightForGPU);

	pointLightSRVIndex_ = SRVManager::GetInstance()->Allocate();
	SRVManager::GetInstance()->CreateSRVStructuredBuffer(pointLightSRVIndex_, pointLightBuffer_.Get(), MaxCount, sizeof(PointLightForGPU));

	//PointLight用のSRV作成
	srvDesc.Buffer.StructureByteStride = sizeof(SpotLightForGPU);

	spotLightSRVIndex_ = SRVManager::GetInstance()->Allocate();
	SRVManager::GetInstance()->CreateSRVStructuredBuffer(spotLightSRVIndex_, spotLightBuffer_.Get(), MaxCount, sizeof(SpotLightForGPU));
}

void LightManager::Update() {
	//各ライトを更新
	for(auto& light : dirLights_) {	//directionalLight
		light->Update();
	}

	for(auto& light : pointLights_) {	//pointLight
		light->Update();
	}

	for(auto& light : spotLights_) {
		light->Update();
	}

	// 書き込む数を制限
	size_t count = (std::min)(dirLights_.size(), (size_t)MaxCount);

	//データを集めてMap済みのメモリに書き込む
	for(size_t i = 0; i < count; ++i) {
		dirLightData_[i].color = dirLights_[i]->GetColor();
		dirLightData_[i].direction = dirLights_[i]->GetDirection();
		dirLightData_[i].intensity = dirLights_[i]->GetIntensity();
	}

	// 書き込む数を制限
	count = (std::min)(pointLights_.size(), (size_t)MaxCount);

	//データを集めてMap済みのメモリに書き込む
	for(size_t i = 0; i < count; ++i) {
		pointLightData_[i].color = pointLights_[i]->GetColor();
		pointLightData_[i].position = pointLights_[i]->GetPosition();
		pointLightData_[i].intensity = pointLights_[i]->GetIntensity();
		pointLightData_[i].radius = pointLights_[i]->GetRadius();
		pointLightData_[i].decay = pointLights_[i]->GetDecay();
	}

	// 書き込む数を制限
	count = (std::min)(spotLights_.size(), (size_t)MaxCount);

	//データを集めてMap済みのメモリに書き込む
	for(size_t i = 0; i < count; ++i) {
		spotLightData_[i].color = spotLights_[i]->GetColor();
		spotLightData_[i].position = spotLights_[i]->GetPosition();
		spotLightData_[i].intensity = spotLights_[i]->GetIntensity();
		spotLightData_[i].direction = spotLights_[i]->GetDirection();
		spotLightData_[i].distance = spotLights_[i]->GetDistance();
		spotLightData_[i].decay = spotLights_[i]->GetDecay();
		spotLightData_[i].cosAngle = spotLights_[i]->GetCosAngle();
	}
}

void LightManager::ImGui() {
#ifdef USEIMGUI
	ImGui::Begin("Light Manager");

	// タブバーを開始するでやんす
	if(ImGui::BeginTabBar("LightTabs")) {

		// --- DirectionalLightタブ ---
		if(ImGui::BeginTabItem("Directional")) {
			if(dirLights_.empty()) {
				ImGui::Text("No Directional Lights");
			}
			else {
				int maxIdx = static_cast<int>(dirLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectDirLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectDirLightIndex_ >= 0 && selectDirLightIndex_ < (int)dirLights_.size()) {
					dirLights_[selectDirLightIndex_]->ImGui(selectDirLightIndex_);
				}
			}
			ImGui::EndTabItem(); // タブを閉じ忘れないように！
		}

		// --- PointLightタブ ---
		if(ImGui::BeginTabItem("Point")) {
			if(pointLights_.empty()) {
				ImGui::Text("No Point Lights");
			}
			else {
				// 点光源用のインデックス変数(selectPointLightIndex_)をメンバに追加しておくといいでやんす！
				int maxIdx = static_cast<int>(pointLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectPointLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectPointLightIndex_ >= 0 && selectPointLightIndex_ < (int)pointLights_.size()) {
					pointLights_[selectPointLightIndex_]->ImGui(selectPointLightIndex_);
				}
			}
			ImGui::EndTabItem();
		}

		// --- SpotLightタブ ---
		if(ImGui::BeginTabItem("Spot")) {
			if(spotLights_.empty()) {
				ImGui::Text("No Spot Lights");
			}
			else {
				// 点光源用のインデックス変数(selectPointLightIndex_)をメンバに追加しておくといいでやんす！
				int maxIdx = static_cast<int>(spotLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectSpotLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectSpotLightIndex_ >= 0 && selectSpotLightIndex_ < (int)spotLights_.size()) {
					spotLights_[selectSpotLightIndex_]->ImGui(selectSpotLightIndex_);
				}
			}
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar(); // タブバーを終了
	}

	ImGui::End();
#endif
}

void LightManager::AddLight(LightType type) {
	switch(type) {
	case DIRECTIONALLIGHT:
		dirLights_.push_back(std::make_unique<DirectionalLight>());
		lightCountData_->dirLight++;
		break;

	case POINTLIGHT:
		pointLights_.push_back(std::make_unique<PointLight>());
		lightCountData_->pointLight++;
		break;

	case SPOTLIGHT:
		spotLights_.push_back(std::make_unique<SpotLight>());
		lightCountData_->spotLight++;
		break;

	default:
		break;
	}
}