#include "LightManager.h"
#include "imgui.h"

//ライトの最大数
const uint32_t MaxCount = 20;

LightManager::LightManager(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;
}

LightManager::~LightManager() {
}

void LightManager::Initialize() {
	lightBuffer_.Initialize(dxCommon_);

	////バッファー確保
	//lightCountBuffer_ = dxCommon_->CreateBufferResource(sizeof(LightCount));
	////書き込むアドレスを取得してマッピング
	//lightCountBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&lightCountData_));

	////バッファー確保
	//dirLightBuffer_ = dxCommon_->CreateBufferResource(sizeof(DirectionalLightForGPU) * MaxCount);
	////書き込むアドレスを取得してマッピング
	//dirLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&dirLightData_));

	////バッファー確保
	//pointLightBuffer_ = dxCommon_->CreateBufferResource(sizeof(PointLightForGPU) * MaxCount);
	////書き込むアドレスを取得してマッピング
	//pointLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&pointLightData_));

	////バッファー確保
	//spotLightBuffer_ = dxCommon_->CreateBufferResource(sizeof(SpotLightForGPU) * MaxCount);
	////書き込むアドレスを取得してマッピング
	//spotLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&spotLightData_));

	////バッファー確保
	//rectLightBuffer_ = dxCommon_->CreateBufferResource(sizeof(RectLightForGPU) * MaxCount);
	////書き込むアドレスを取得してマッピング
	//rectLightBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&rectLightData_));

	////DierctionalLightのSRVを作る
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.Format = DXGI_FORMAT_UNKNOWN; // 構造化バッファの場合はUNKNOWN
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	//srvDesc.Buffer.FirstElement = 0;
	//srvDesc.Buffer.NumElements = MaxCount; // 最大数
	//srvDesc.Buffer.StructureByteStride = sizeof(DirectionalLightForGPU); // 1要素のサイズ
	//srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	//dirLightSRVIndex_ = SRVManager::GetInstance()->Allocate();
	//SRVManager::GetInstance()->CreateSRVStructuredBuffer(dirLightSRVIndex_, dirLightBuffer_.Get(), MaxCount, sizeof(DirectionalLightForGPU));

	////PointLight用のSRV作成
	//srvDesc.Buffer.StructureByteStride = sizeof(PointLightForGPU);

	//pointLightSRVIndex_ = SRVManager::GetInstance()->Allocate();
	//SRVManager::GetInstance()->CreateSRVStructuredBuffer(pointLightSRVIndex_, pointLightBuffer_.Get(), MaxCount, sizeof(PointLightForGPU));

	////SpotLight用のSRV作成
	//srvDesc.Buffer.StructureByteStride = sizeof(SpotLightForGPU);

	//spotLightSRVIndex_ = SRVManager::GetInstance()->Allocate();
	//SRVManager::GetInstance()->CreateSRVStructuredBuffer(spotLightSRVIndex_, spotLightBuffer_.Get(), MaxCount, sizeof(SpotLightForGPU));

	////RectLight用のSRV作成
	//srvDesc.Buffer.StructureByteStride = sizeof(RectLightForGPU);

	//rectLightSRVIndex_ = SRVManager::GetInstance()->Allocate();
	//SRVManager::GetInstance()->CreateSRVStructuredBuffer(rectLightSRVIndex_, rectLightBuffer_.Get(), MaxCount, sizeof(RectLightForGPU));
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

	for(auto& light : rectLights_) {
		light->Update();
	}

	//LightCountのデータを更新
	lightCPUData_.count.dirLight = static_cast<int32_t>(dirLights_.size());
	lightCPUData_.count.pointLight = static_cast<int32_t>(pointLights_.size());
	lightCPUData_.count.spotLight = static_cast<int32_t>(spotLights_.size());
	lightCPUData_.count.rectLight = static_cast<int32_t>(rectLights_.size());

	// データの詰め込み
	size_t dirCount = (std::min)(dirLights_.size(), (size_t)MaxCount);
	for(size_t i = 0; i < dirCount; ++i) {
		lightCPUData_.dirLights[i].color = dirLights_[i]->GetColor();
		lightCPUData_.dirLights[i].direction = dirLights_[i]->GetDirection();
		lightCPUData_.dirLights[i].intensity = dirLights_[i]->GetIntensity();
	}

	size_t pointCount = (std::min)(pointLights_.size(), (size_t)MaxCount);
	for(size_t i = 0; i < pointCount; ++i) {
		lightCPUData_.pointLights[i].color = pointLights_[i]->GetColor();
		lightCPUData_.pointLights[i].position = pointLights_[i]->GetPosition();
		lightCPUData_.pointLights[i].intensity = pointLights_[i]->GetIntensity();
		lightCPUData_.pointLights[i].radius = pointLights_[i]->GetRadius();
		lightCPUData_.pointLights[i].decay = pointLights_[i]->GetDecay();
	}

	size_t spotCount = (std::min)(spotLights_.size(), (size_t)MaxCount);
	for(size_t i = 0; i < spotCount; ++i) {
		lightCPUData_.spotLights[i].color = spotLights_[i]->GetColor();
		lightCPUData_.spotLights[i].position = spotLights_[i]->GetPosition();
		lightCPUData_.spotLights[i].intensity = spotLights_[i]->GetIntensity();
		lightCPUData_.spotLights[i].direction = spotLights_[i]->GetDirection();
		lightCPUData_.spotLights[i].distance = spotLights_[i]->GetDistance();
		lightCPUData_.spotLights[i].decay = spotLights_[i]->GetDecay();
		lightCPUData_.spotLights[i].cosAngle = spotLights_[i]->GetCosAngle();
	}

	size_t rectCount = (std::min)(rectLights_.size(), (size_t)MaxCount);
	for(size_t i = 0; i < spotCount; ++i) {
		lightCPUData_.rectLights[i].color = rectLights_[i]->GetColor();
		lightCPUData_.rectLights[i].position = rectLights_[i]->GetPosition();
		lightCPUData_.rectLights[i].intensity = rectLights_[i]->GetIntensity();
		lightCPUData_.rectLights[i].direction = rectLights_[i]->GetDirection();
		lightCPUData_.rectLights[i].size = rectLights_[i]->GetSize();
		lightCPUData_.rectLights[i].right = rectLights_[i]->GetRight();
		lightCPUData_.rectLights[i].up = rectLights_[i]->GetUp();
		lightCPUData_.rectLights[i].decay = rectLights_[i]->GetDecay();
	}

	// まとめてGPUバッファに転送
	lightBuffer_.Update(lightCPUData_);
}

void LightManager::ImGui() {
#ifdef USEIMGUI
	ImGui::Begin("Light Manager");

	// タブバーを開始する
	if(ImGui::BeginTabBar("LightTabs")) {

		// --- DirectionalLightタブ ---
		if(ImGui::BeginTabItem("Directional")) {

			//ライトを増やすボタン
			if(ImGui::Button("Add DirectionalLight")) {
				if(dirLights_.size() < MaxCount) {
					AddLight(DIRECTIONALLIGHT); // 既存のAddLight関数を呼ぶ
					selectDirLightIndex_ = static_cast<int>(dirLights_.size()) - 1; // 追加したやつを選択
				}
			}

			if(dirLights_.empty()) {
				ImGui::Text("No Directional Lights");
			}
			else {
				int maxIdx = static_cast<int>(dirLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectDirLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectDirLightIndex_ >= 0 && selectDirLightIndex_ < (int)dirLights_.size()) {
					dirLights_[selectDirLightIndex_]->ImGui(selectDirLightIndex_);
				
					//ライトを消すボタン
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f)); // 暗い赤
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // 明るい赤
					if(ImGui::Button("Delete This Light", ImVec2(-1, 0))) { // 横いっぱいのボタン
						dirLights_.erase(dirLights_.begin() + selectDirLightIndex_);
						// インデックスが範囲外にならないように調整
						if(selectDirLightIndex_ > 0) {
							selectDirLightIndex_--;
						}
					}
					ImGui::PopStyleColor(2);
				}
			}
			ImGui::EndTabItem(); // タブを閉じ忘れないように！
		}

		// --- PointLightタブ ---
		if(ImGui::BeginTabItem("Point")) {

			//ライトを増やすボタン
			if(ImGui::Button("Add PointLight")) {
				if(pointLights_.size() < MaxCount) {
					AddLight(POINTLIGHT); // 既存のAddLight関数を呼ぶ
					selectPointLightIndex_ = static_cast<int>(pointLights_.size()) - 1; // 追加したやつを選択
				}
			}

			if(pointLights_.empty()) {
				ImGui::Text("No Point Lights");
			}
			else {
				int maxIdx = static_cast<int>(pointLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectPointLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectPointLightIndex_ >= 0 && selectPointLightIndex_ < (int)pointLights_.size()) {
					pointLights_[selectPointLightIndex_]->ImGui(selectPointLightIndex_);
				
					//ライトを消すボタン
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f)); // 暗い赤
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // 明るい赤
					if(ImGui::Button("Delete This Light", ImVec2(-1, 0))) { // 横いっぱいのボタン
						pointLights_.erase(pointLights_.begin() + selectPointLightIndex_);
						// インデックスが範囲外にならないように調整
						if(selectPointLightIndex_ > 0) {
							selectPointLightIndex_--;
						}
					}
					ImGui::PopStyleColor(2);
				}
			}
			ImGui::EndTabItem();
		}

		// --- SpotLightタブ ---
		if(ImGui::BeginTabItem("Spot")) {

			//ライトを増やすボタン
			if(ImGui::Button("Add SpotLight")) {
				if(spotLights_.size() < MaxCount) {
					AddLight(SPOTLIGHT); // 既存のAddLight関数を呼ぶ
					selectSpotLightIndex_ = static_cast<int>(spotLights_.size()) - 1; // 追加したやつを選択
				}
			}

			if(spotLights_.empty()) {
				ImGui::Text("No Spot Lights");
			}
			else {
				int maxIdx = static_cast<int>(spotLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectSpotLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectSpotLightIndex_ >= 0 && selectSpotLightIndex_ < (int)spotLights_.size()) {
					spotLights_[selectSpotLightIndex_]->ImGui(selectSpotLightIndex_);

					//ライトを消すボタン
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f)); // 暗い赤
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // 明るい赤
					if(ImGui::Button("Delete This Light", ImVec2(-1, 0))) { // 横いっぱいのボタン
						spotLights_.erase(spotLights_.begin() + selectSpotLightIndex_);
						// インデックスが範囲外にならないように調整
						if(selectSpotLightIndex_ > 0) {
							selectSpotLightIndex_--;
						}
					}
					ImGui::PopStyleColor(2);
				}
			}
			ImGui::EndTabItem();
		}

		// --- RectLightタブ ---
		if(ImGui::BeginTabItem("Rect")) {

			//ライトを増やすボタン
			if(ImGui::Button("Add RectLight")) {
				if(rectLights_.size() < MaxCount) {
					AddLight(RECTLIGHT); // 既存のAddLight関数を呼ぶ
					selectRectLightIndex_ = static_cast<int>(rectLights_.size()) - 1; // 追加したやつを選択
				}
			}

			if(rectLights_.empty()) {
				ImGui::Text("No Rect Lights");
			}
			else {
				int maxIdx = static_cast<int>(rectLights_.size()) - 1;
				ImGui::SliderInt("Select", &selectRectLightIndex_, 0, maxIdx);
				ImGui::Separator();
				if(selectRectLightIndex_ >= 0 && selectRectLightIndex_ < (int)rectLights_.size()) {
					rectLights_[selectRectLightIndex_]->ImGui(selectRectLightIndex_);

					//ライトを消すボタン
					ImGui::Spacing();
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f)); // 暗い赤
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.1f, 0.1f, 1.0f)); // 明るい赤
					if(ImGui::Button("Delete This Light", ImVec2(-1, 0))) { // 横いっぱいのボタン
						rectLights_.erase(rectLights_.begin() + selectRectLightIndex_);
						// インデックスが範囲外にならないように調整
						if(selectRectLightIndex_ > 0) {
							selectRectLightIndex_--;
						}
					}
					ImGui::PopStyleColor(2);
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

	case RECTLIGHT:
		rectLights_.push_back(std::make_unique<RectLight>());
		lightCountData_->rectLight++;
		break;

	default:
		break;
	}
}

void LightManager::DeleteLight(LightType type, uint32_t index) {
	switch (type) {
		case DIRECTIONALLIGHT:
		dirLights_.erase(dirLights_.begin() + index);
		break;

		case POINTLIGHT:
		pointLights_.erase(pointLights_.begin() + index);
		break;

		case SPOTLIGHT:
		spotLights_.erase(spotLights_.begin() + index);
		break;

		case RECTLIGHT:
		rectLights_.erase(rectLights_.begin() + index);
		break;

		default:
		break;
	}
}