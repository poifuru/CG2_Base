#include "ParticleSystem.h"
#include "ParticleSystem.h"
#include "ParticleField.h" // GravityField など
#include "imgui.h"
#include "TextureManager.h"

ParticleSystem::ParticleSystem(DxCommon* dxCommon) : dxCommon_(dxCommon) {}

void ParticleSystem::Initialize() {
	// 完全に空の状態でスタート。
	// 最初から何か出したい場合は、ここで AddEmitter や AddField を呼ぶ。
	groups_.clear();
	emitters_.clear();
	fields_.clear();
}

void ParticleSystem::Update(const CameraData& cameraData) {
	// 全てのエミッターの更新（発生処理）
	for (size_t i = 0; i < emitters_.size(); ++i) {
		emitters_[i]->Update();
	}

	// 全てのグループの更新（物理計算・バッファ転送）
	for (size_t i = 0; i < groups_.size(); ++i) {
		groups_[i]->Update(cameraData);
	}
}

void ParticleSystem::Draw() {
	// 全てのグループを描画コマンドに積む
	for (size_t i = 0; i < groups_.size(); ++i) {
		groups_[i]->Draw();
	}
}

void ParticleSystem::ImGui() {
#ifdef USEIMGUI
	ImGui::Begin("Particle System");

	// ----------------------------------------------------
	// 選択インデックスが範囲外を指さないように丸める
	// ----------------------------------------------------
	if(currentSelectedEmitter_ >= static_cast<int>(emitters_.size())) {
		currentSelectedEmitter_ = static_cast<int>(emitters_.size()) - 1;
	}
	if(currentSelectedEmitter_ < 0) currentSelectedEmitter_ = 0;

	if(currentSelectedGroup_ >= static_cast<int>(groups_.size())) {
		currentSelectedGroup_ = static_cast<int>(groups_.size()) - 1;
	}
	if(currentSelectedGroup_ < 0) currentSelectedGroup_ = 0;

	if(currentSelectedField_ >= static_cast<int>(fields_.size())) {
		currentSelectedField_ = static_cast<int>(fields_.size()) - 1;
	}
	if(currentSelectedField_ < 0) currentSelectedField_ = 0;

	// タブバーの開始
	if(ImGui::BeginTabBar("ParticleEditorTabs")) {

		// ----------------------------------------------------
		// Emitter タブ（エミッター単体のパラメータ編集）
		// ----------------------------------------------------
		if(ImGui::BeginTabItem("Emitters")) {

			// エミッター一覧
			ImGui::Text("Emitter List");
			if(ImGui::BeginListBox("##EmitterList", ImVec2(-1, 0))) {
				for(size_t i = 0; i < emitters_.size(); ++i) {
					bool isSelected = (currentSelectedEmitter_ == static_cast<int>(i));
					if(ImGui::Selectable(emitters_[i]->GetName().c_str(), isSelected)) {
						currentSelectedEmitter_ = static_cast<int>(i);
					}
				}
				ImGui::EndListBox();
			}

			ImGui::Spacing();

			// 新しいエミッターをその場で追加するボタン
			if(ImGui::Button("Add Emitter", ImVec2(-2, 0))) {
				std::string newName = "Emitter_" + std::to_string(emitters_.size());
				AddEmitter(newName);
				currentSelectedEmitter_ = static_cast<int>(emitters_.size()) - 1;
			}

			// 選択中のエミッターの編集
			if(!emitters_.empty() && currentSelectedEmitter_ < emitters_.size()) {
				ImGui::SeparatorText("Emitter Setting");
				emitters_[currentSelectedEmitter_]->ImGui();
			}
			else {
				ImGui::Text("No active Emitter selected.");
			}
			ImGui::EndTabItem();
		}

		// ----------------------------------------------------
		// Field タブ（フィールド単体のパラメータ編集）
		// ----------------------------------------------------
		if(ImGui::BeginTabItem("Fields")) {
			ImGui::Columns(2, "FieldColumns");

			// 左側：フィールド一覧
			if(ImGui::BeginListBox("##FieldList", ImVec2(-1, -1))) {
				for(size_t i = 0; i < fields_.size(); ++i) {
					bool isSelected = (currentSelectedField_ == static_cast<int>(i));
					if(ImGui::Selectable(fields_[i]->GetName().c_str(), isSelected)) {
						currentSelectedField_ = static_cast<int>(i);
					}
				}
				ImGui::EndListBox();
			}

			ImGui::NextColumn();

			// 右側：選択中のフィールドの編集
			if(!fields_.empty() && currentSelectedField_ < fields_.size()) {
				ImGui::Text("Field Settings");
				fields_[currentSelectedField_]->ImGui();
			}
			ImGui::Columns(1);
			ImGui::EndTabItem();
		}

		// ----------------------------------------------------
		// Particle (Group) タブ（見た目・マテリアルの編集）
		// ----------------------------------------------------
		if(ImGui::BeginTabItem("Particles")) {

			// パーティクルグループ一覧
			ImGui::Text("Particle Group List");
			if(ImGui::BeginListBox("##GroupList", ImVec2(-1, 0))) {
				for(size_t i = 0; i < groups_.size(); ++i) {
					bool isSelected = (currentSelectedGroup_ == static_cast<int>(i));
					if(ImGui::Selectable(groups_[i]->GetName().c_str(), isSelected)) {
						currentSelectedGroup_ = static_cast<int>(i);
					}
				}
				ImGui::EndListBox();
			}

			ImGui::Spacing();

			// 新しいパーティクルグループ（挙動・見た目アセット）を追加するボタン
			if(ImGui::Button("Add Group", ImVec2(-1, 0))) {
				std::string newName = "Group_" + std::to_string(groups_.size());
				D3D12_GPU_DESCRIPTOR_HANDLE texHandle = TextureManager::GetInstance()->GetTextureHandle("particle");

				AddGroup(newName, texHandle);
				currentSelectedGroup_ = static_cast<int>(groups_.size()) - 1;
			}

			// 右側：選択中のグループの編集
			if(!groups_.empty() && currentSelectedGroup_ < groups_.size()) {
				ImGui::SeparatorText("Particle Group Settings");
				groups_[currentSelectedGroup_]->ImGui(); 
			}
			else {
				ImGui::Text("No active particle group selected.");
			}
			ImGui::EndTabItem();
		}

		// ----------------------------------------------------
		// System (組み合わせ) タブ（関係性の構築）
		// ----------------------------------------------------
		if(ImGui::BeginTabItem("Setup")) {
			ImGui::Columns(2, "SystemSetupColumns");

			// 【左カラム】紐付け対象のエミッターを1つ選択
			ImGui::Text("1. Select Emitter");
			if(ImGui::BeginListBox("##SystemEmitterList", ImVec2(-1, 0))) {
				for(size_t i = 0; i < emitters_.size(); ++i) {
					bool isSelected = (currentSelectedEmitter_ == static_cast<int>(i));
					if(ImGui::Selectable(emitters_[i]->GetName().c_str(), isSelected)) {
						currentSelectedEmitter_ = static_cast<int>(i);
					}
				}
				ImGui::EndListBox();
			}

			// 【右カラム】選択したエミッターに対する紐付け設定
			ImGui::NextColumn();

			if(!emitters_.empty() && currentSelectedEmitter_ < emitters_.size()) {
				ParticleEmitter* selectedEmitter = emitters_[currentSelectedEmitter_].get();
				//ImGui::Separator();

				// ====================================================
				// パーティクルグループとの紐付け
				// ====================================================
				ImGui::Text("2. Connected Particle Groups");

				if(ImGui::BeginListBox("##ConnectedGroupList", ImVec2(-1, 0))) {
					for(size_t g = 0; g < groups_.size(); ++g) {
						if(selectedEmitter->IsTargeting(groups_[g].get())) {
							ImGui::Text("- %s", groups_[g]->GetName().c_str());
							ImGui::SameLine(ImGui::GetWindowWidth() - 80);

							if(ImGui::Button(("Disconnect##G" + std::to_string(g)).c_str())) {
								selectedEmitter->UntargetGroup(groups_[g].get());
							}
						}
					}
					ImGui::EndListBox();
				}

				ImGui::Columns(1);

				static int comboSelectedGroup = 0;
				if(comboSelectedGroup >= static_cast<int>(groups_.size())) comboSelectedGroup = 0;

				if(!groups_.empty()) {
					std::string comboPreview = groups_[comboSelectedGroup]->GetName();
					ImGui::Text("Select Group to Connect");
					if(ImGui::BeginCombo("##Select Group to Connect", comboPreview.c_str())) {
						for(size_t g = 0; g < groups_.size(); ++g) {
							bool isSelected = (comboSelectedGroup == static_cast<int>(g));
							if(ImGui::Selectable(groups_[g]->GetName().c_str(), isSelected)) {
								comboSelectedGroup = static_cast<int>(g);
							}
						}
						ImGui::EndCombo();
					}

					ImGui::SameLine();
					if(ImGui::Button("Connect Group")) {
						selectedEmitter->TargetGroup(groups_[comboSelectedGroup].get());
					}
				}
				else {
					ImGui::Text("No available groups to connect. Add a group first!");
				}

				ImGui::Text("Choice Emitter: %s", selectedEmitter->GetName().c_str());
				ImGui::Spacing();
				ImGui::Separator();

				// ====================================================
				// フィールドとの紐付け
				// ====================================================
				ImGui::Text("3. Connected Fields (Applies to ALL target groups)");

				if(ImGui::BeginListBox("##ConnectedFieldList", ImVec2(-1, 100))) {
					for(size_t f = 0; f < fields_.size(); ++f) {
						bool isFieldConnected = false;
						for(size_t g = 0; g < groups_.size(); ++g) {
							if(selectedEmitter->IsTargeting(groups_[g].get()) && groups_[g]->HasField(fields_[f].get())) {
								isFieldConnected = true;
								break;
							}
						}

						if(isFieldConnected) {
							ImGui::Text("- %s", fields_[f]->GetName().c_str());
							ImGui::SameLine(ImGui::GetWindowWidth() - 60);
							if(ImGui::Button(("Disconnect##F" + std::to_string(f)).c_str())) {
								for(size_t g = 0; g < groups_.size(); ++g) {
									if(selectedEmitter->IsTargeting(groups_[g].get())) {
										groups_[g]->RemoveField(fields_[f].get());
									}
								}
							}
						}
					}
					ImGui::EndListBox();
				}

				static int comboSelectedField = 0;
				if(comboSelectedField >= static_cast<int>(fields_.size())) comboSelectedField = 0;

				if(!fields_.empty()) {
					std::string comboFieldPreview = fields_[comboSelectedField]->GetName();
					if(ImGui::BeginCombo("Select Field to Connect", comboFieldPreview.c_str())) {
						for(size_t f = 0; f < fields_.size(); ++f) {
							bool isSelected = (comboSelectedField == static_cast<int>(f));
							if(ImGui::Selectable(fields_[f]->GetName().c_str(), isSelected)) {
								comboSelectedField = static_cast<int>(f);
							}
						}
						ImGui::EndCombo();
					}

					ImGui::SameLine();
					if(ImGui::Button("Connect Field")) {
						for(size_t g = 0; g < groups_.size(); ++g) {
							if(selectedEmitter->IsTargeting(groups_[g].get())) {
								groups_[g]->AddField(fields_[comboSelectedField].get());
							}
						}
					}
				}
			}
			else {
				ImGui::Text("Please select or create an emitter first.");
			}

			ImGui::EndTabItem();
		}

		// 【修正】EndTabBarとEndのペア関係を正しい位置に修正！
		ImGui::EndTabBar(); 
	}

	ImGui::End();
#endif
}

void ParticleSystem::AddEmitter(const std::string& name) {
	// エミッターを生成
	auto newEmitter = std::make_unique<ParticleEmitter>(name);

	// 自身の管理用 vector に追加
	emitters_.push_back(std::move(newEmitter));
}

void ParticleSystem::AddGroup(const std::string& name, D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
	// パーティクルグループ（アセット）を単体で生成する！
	auto newGroup = std::make_unique<ParticleGroup>(dxCommon_);
	newGroup->Initialize(name);
	newGroup->SetTexture(textureHandle);
	groups_.push_back(std::move(newGroup));
}

void ParticleSystem::AddField(std::unique_ptr<IParticleField> field) {
	if (!field) return;

	// すべてのグループにこのフィールドを登録する
	for (size_t i = 0; i < groups_.size(); ++i) {
		groups_[i]->AddField(field.get());
	}

	// 寿命管理用にシステム側で保持
	fields_.push_back(std::move(field));
}