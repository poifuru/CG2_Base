#include <fstream>
#include <iomanip>
#include <filesystem>
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
				int texHandle = TextureManager::GetInstance()->LoadTexture("Resources/Particle/circle2.png");

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

			// ====================================================
			// 【上段】エミッター選択 ＆ 接続済みグループ一覧
			// ====================================================
			ImGui::Columns(2, "SystemSetupColumns", true);

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
				ImGui::Text("Connected Particle Groups");

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

				ImGui::Columns(1); // 上段のカラム終了（一度リセット）
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// ====================================================
				// 【下段】グループをリストから選択してコネクト ＆ フィールド一覧
				// ====================================================
				ImGui::Columns(2, "LowerSetupColumns", true); // 下段のカラム開始

				// 【下段・左カラム】パーティクルグループをリストから選択してコネクト
				ImGui::Text("2. Connect Group to Emitter");

				static int listSelectedGroup = 0; // 変数名も分かりやすくlistに変更
				if(listSelectedGroup >= static_cast<int>(groups_.size())) listSelectedGroup = 0;

				if(!groups_.empty()) {
					// 【修正】BeginCombo から、右カラムと高さを合わせた BeginListBox に変更！
					if(ImGui::BeginListBox("##SelectGroupToConnectList", ImVec2(-1, 100))) {
						for(size_t g = 0; g < groups_.size(); ++g) {
							bool isSelected = (listSelectedGroup == static_cast<int>(g));

							// リスト内の項目を選択可能にする
							if (ImGui::Selectable(groups_[g]->GetName().c_str(), isSelected)) {
								listSelectedGroup = static_cast<int>(g);
							}
						}
						ImGui::EndListBox();
					}

					// 選択中のグループをエミッターに接続するボタン
					ImGui::Text("Choice Emitter: %s", selectedEmitter->GetName().c_str());
					if(ImGui::Button("Connect Group", ImVec2(-1, 0))) {
						selectedEmitter->TargetGroup(groups_[listSelectedGroup].get());
					}
				}
				else {
					// グループが一つもない時はリストの代わりにグレーアウトしたメッセージを出す
					ImGui::TextDisabled("No available groups to connect.\nAdd a group first!");
				}

				ImGui::NextColumn(); // 右カラム（フィールド一覧）へ移動

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

			ImGui::Columns(1);

			// ====================================================
			// カスタムセーブ＆ロード機能
			// ====================================================
			ImGui::SeparatorText("File Save / Load");

			ImGui::Text("Saved Files List");
			static int selectedFileIndex = 0;
			std::vector<std::string> fileNames;

			// Resources/Data/ フォルダの中身を走査してJSONファイルをリストアップ
			std::string dataDirPath = "Resources/Data";
			if (std::filesystem::exists(dataDirPath)) {
				for (const auto& entry : std::filesystem::directory_iterator(dataDirPath)) {
					if (entry.is_regular_file() && entry.path().extension() == ".json") {
						// ファイル名（拡張子なし）を取得して配列に入れる
						fileNames.push_back(entry.path().stem().string());
					}
				}
			}

			// 過去に保存されたファイルがあればコンボボックスで表示
			if (!fileNames.empty()) {
				if (selectedFileIndex >= fileNames.size()) selectedFileIndex = 0;

				std::string comboPreview = fileNames[selectedFileIndex];
				if(ImGui::BeginCombo("choice File", comboPreview.c_str())) {
					for (size_t f = 0; f < fileNames.size(); ++f) {
						bool isSelected = (selectedFileIndex == static_cast<int>(f));
						if (ImGui::Selectable(fileNames[f].c_str(), isSelected)) {
							selectedFileIndex = static_cast<int>(f);
							// 選択したファイル名を入力バッファにコピーする
							snprintf(fileNameBuffer_, sizeof(fileNameBuffer_), "%s", fileNames[f].c_str());
						}
					}
					ImGui::EndCombo();
				}
			} else {
				ImGui::TextDisabled("No saved files found in Resources/Data/");
			}
			ImGui::Spacing();

			// ファイル名の入力欄
			ImGui::InputText("File Name", fileNameBuffer_, sizeof(fileNameBuffer_));
			ImGui::Text("Save Path: Resources/Data/%s.json", fileNameBuffer_);

			ImGui::Spacing();

			// セーブボタンの処理
			if (ImGui::Button("Save System")) {
				// 入力されたファイル名からフルパスを合成する
				std::string fullPath = "Resources/Data/" + std::string(fileNameBuffer_) + ".json";
				SaveToFile(fullPath);
			}

			ImGui::SameLine();

			// ロードボタンの処理
			if (ImGui::Button("Load System")) {
				// 入力されたファイル名からフルパスを合成する
				std::string fullPath = "Resources/Data/" + std::string(fileNameBuffer_) + ".json";
				LoadFromFile(fullPath);
			}

			ImGui::SameLine();

			// ----------------------------------------------------
			// 【修正】プランB：モーダルポップアップ呼び出しボタン
			// ----------------------------------------------------
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));

			if (ImGui::Button("Delete File")) {
				// ボタンが押されたら、指定した識別名のポップアップを開く（トリガーを引く）
				ImGui::OpenPopup("Delete File?");
			}

			ImGui::PopStyleColor(3);

			// ----------------------------------------------------
			// 【新機能】モーダルポップアップのウィンドウ本体
			// ----------------------------------------------------
			// 開いている間だけ、このif文の中身が実行されるよ
			// ImGuiWindowFlags_AlwaysAutoResize をつけて、文字数に合わせて綺麗にフィットさせる
			if (ImGui::BeginPopupModal("Delete File?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				ImGui::Text("Warning : この操作は元に戻せません\n");
				ImGui::Text("このファイルを削除しますか？");
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Target: Resources/Data/%s.json", fileNameBuffer_);
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// [Yes] ボタン（危険なボタンなので真っ赤にする）
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));

				if (ImGui::Button("Delete", ImVec2(120, 0))) {
					std::string fullPath = "Resources/Data/" + std::string(fileNameBuffer_) + ".json";
					if (std::filesystem::exists(fullPath)) {
						std::filesystem::remove(fullPath);
					}

					// ファイルを消したらポップアップを閉じる
					ImGui::CloseCurrentPopup(); 
				}
				ImGui::PopStyleColor(2);

				ImGui::SameLine();

				// [No / Cancel] ボタン（安全に引き返す）
				if (ImGui::Button("Cancel", ImVec2(120, 0))) {
					// 何もせずにポップアップを閉じる
					ImGui::CloseCurrentPopup(); 
				}

				// 【最重要】BeginPopupModal が true を返したときだけ、最後にEndを呼ぶ！
				ImGui::EndPopup(); 
			}
			// ----------------------------------------------------

			ImGui::EndTabItem();
		}
		ImGui::EndTabBar(); 
	}
#endif
}

void ParticleSystem::SaveToFile(const std::string& filePath) {
	nlohmann::json root;

	// 1. 全てのグループを保存
	root["groups"] = nlohmann::json::array();
	for (size_t i = 0; i < groups_.size(); ++i) {
		nlohmann::json groupJson;
		groups_[i]->SaveConfig(groupJson);
		root["groups"].push_back(groupJson);
	}

	// 2. 全てのエミッターを保存
	root["emitters"] = nlohmann::json::array();
	for (size_t i = 0; i < emitters_.size(); ++i) {
		nlohmann::json emitterJson;
		emitters_[i]->SaveConfig(emitterJson);
		root["emitters"].push_back(emitterJson);
	}

	// 保存先ファイルの親フォルダ（Resources/Data/）がなければ自動作成する
	std::filesystem::path path(filePath);
	if (path.has_parent_path()) {
		std::filesystem::create_directories(path.parent_path());
	}

	// ファイル書き出し
	std::ofstream file(filePath);
	if (file.is_open()) {
		file << std::setw(4) << root << std::endl; // インデント付きで見やすく保存
	}
}

void ParticleSystem::LoadFromFile(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) return;

	nlohmann::json root;
	file >> root;

	// 実行中の古いデータを一回綺麗にする
	groups_.clear();
	emitters_.clear();

	// グループの復元
	if (root.contains("groups")) {
		for (size_t i = 0; i < root["groups"].size(); ++i) {
			const auto& groupJson = root["groups"][i];

			std::string gName = groupJson["name"];
			std::string texPath = "";
			int texHandle = 0; // デフォルトテクスチャのハンドルなど

			// テクスチャパスが保存されていれば読み込んでハンドルを取得
			if (groupJson.contains("texturePath")) {
				texPath = groupJson["texturePath"].get<std::string>();
				if (!texPath.empty()) {
					texHandle = TextureManager::GetInstance()->LoadTexture(texPath);
				}
			}

			// 正しいテクスチャハンドルを渡してグループを生成
			AddGroup(gName, texHandle); 
			groups_.back()->LoadConfig(groupJson);
		}
	}

	// エミッターの復元とグループとの再接続
	if (root.contains("emitters")) {
		for (size_t i = 0; i < root["emitters"].size(); ++i) {
			const auto& emitterJson = root["emitters"][i];

			AddEmitter(emitterJson["name"]);
			auto* newEmitter = emitters_.back().get();

			// エミッター自体のデータをロード
			newEmitter->LoadConfig(emitterJson);

			// ターゲットグループの紐付けを名前を頼りに再構築
			if (emitterJson.contains("targetGroups")) {
				for (size_t t = 0; t < emitterJson["targetGroups"].size(); ++t) {
					std::string targetName = emitterJson["targetGroups"][t].get<std::string>();

					for (size_t g = 0; g < groups_.size(); ++g) {
						if (groups_[g]->GetName() == targetName) {
							newEmitter->TargetGroup(groups_[g].get());
							break;
						}
					}
				}
			}
		}
	}
}

void ParticleSystem::AddEmitter(const std::string& name) {
	// エミッターを生成
	auto newEmitter = std::make_unique<ParticleEmitter>(name);

	// 自身の管理用 vector に追加
	emitters_.push_back(std::move(newEmitter));
}

void ParticleSystem::AddGroup(const std::string& name, int textureHandle) {
	// パーティクルグループ（アセット）を単体で生成する！
	auto newGroup = std::make_unique<ParticleGroup>(dxCommon_);
	newGroup->Initialize(name);

	newGroup->SetTextureIndex(textureHandle);

	// 初期テクスチャのパスがTextureManagerから取れればセットし、取れなければデフォルトパスを直に割り当てておく
	std::string initPath = TextureManager::GetInstance()->GetTexturePath(textureHandle);
	if(initPath.empty()) {
		initPath = "Resources/Particle/circle2.png"; // フォールバック用
	}

	newGroup->SetTexturePath(initPath);

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