#include "PCH.h"
#include "LevelEditor.h"
#include "CommandManager.h"
#include "TransformCommand.h"
#include "MeshRendererComponent.h"
#include "MathFunction.h"
#include "CameraComponent.h"
#include "EditorManager.h"

void LevelEditor::Initialize(SceneContext* context) {
	context_ = context;
}

void LevelEditor::Update(std::vector<std::unique_ptr<GameObject>>& gameObjects, GameObject*& selectedObject, CameraData* cameraData) {
	if(!context_) return;

	// Ctrl + Z で Undo (元に戻す)
	if(ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
		CommandManager::GetInstance()->Undo();
	}
	// Ctrl + Y で Redo (やり直す)
	if(ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
		CommandManager::GetInstance()->Redo();
	}
	// ---- ヒエラルキー ----
	ImGui::Begin("ヒエラルキー");
	// シーン保存ボタン
	if(ImGui::Button("シーンを保存")) {
		SaveScene(gameObjects);
	}
	ImGui::SameLine();
	// シーン読込ボタン
	if(ImGui::Button("シーン読み込み")) {
		LoadScene(gameObjects, selectedObject);
	}
	ImGui::Separator();
	// 新規GameObject作成ボタン
	if(ImGui::Button("新規 GameObject 作成")) {
		gameObjects.push_back(std::make_unique<GameObject>(context_, "New GameObject"));
	}
	ImGui::Spacing();
	// オブジェクト一覧を表示
	for(auto it = gameObjects.begin(); it != gameObjects.end(); ) {
		GameObject* obj = it->get();
		bool isSelected = (selectedObject == obj);
		ImGui::PushID(obj);
		// 削除ボタン
		if(ImGui::Button("X")) {
			if(selectedObject == obj) {
				selectedObject = nullptr;
			}
			it = gameObjects.erase(it);
			ImGui::PopID();
			continue;
		}
		ImGui::SameLine();
		// 選択用のSelectable項目
		if(ImGui::Selectable(obj->GetName().c_str(), isSelected)) {
			selectedObject = obj;
		}
		ImGui::PopID();
		++it;
	}
	ImGui::End();
	// ---- インスペクター ----
	ImGui::Begin("Inspector");
	if(selectedObject != nullptr) {
		selectedObject->ImGui();
	}
	else {
		ImGui::Text("オブジェクトが選択されていません");
	}
	ImGui::End();
	// ---- 3Dギズモの描画 ----
	if(selectedObject != nullptr) {
		auto& camData = *cameraData;
		auto& transform = selectedObject->GetTransform();
		// ワールド行列を作成
		Matrix4x4 worldMatrix = Math::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::BeginFrame();
		ImGuizmo::AllowAxisFlip(false);
		static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);
		static ImGuizmo::MODE currentGizmoMode(ImGuizmo::LOCAL);
		ImGuiIO& io = ImGui::GetIO();

		ImVec2 viewportPos = ImVec2(0.0f, 0.0f);
		ImVec2 viewportSize = ImVec2(0.0f, 0.0f);

		// "Game" ウィンドウをアペンドオープンし、位置とサイズを取得する
		ImGui::Begin("Game");
		viewportPos = ImGui::GetWindowPos();
		viewportSize = ImGui::GetWindowSize();

		// ギズモ操作用の設定用ウィンドウ
		ImGui::Begin("Gizmo");
		ImGui::Text("Gizmo Operation");
		if(ImGui::RadioButton("S", currentGizmoOperation == ImGuizmo::SCALE)) {
			currentGizmoOperation = ImGuizmo::SCALE;
		}
		ImGui::SameLine();
		if(ImGui::RadioButton("R", currentGizmoOperation == ImGuizmo::ROTATE)) {
			currentGizmoOperation = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();
		if(ImGui::RadioButton("T", currentGizmoOperation == ImGuizmo::TRANSLATE)) {
			currentGizmoOperation = ImGuizmo::TRANSLATE;
		}

		ImGui::Text("Gizmo Space");
		if(ImGui::RadioButton("Local", currentGizmoMode == ImGuizmo::LOCAL)) {
			currentGizmoMode = ImGuizmo::LOCAL;
		}
		ImGui::SameLine();
		if(ImGui::RadioButton("World", currentGizmoMode == ImGuizmo::WORLD)) {
			currentGizmoMode = ImGuizmo::WORLD;
		}
		ImGui::End();

		// タイトルバー（タブ）の高さ分、位置とサイズを補正する
		float titleBarHeight = ImGui::GetFrameHeight();
		viewportPos.y += titleBarHeight;
		viewportSize.y -= titleBarHeight;

		// 2. ギズモの描画範囲を "Game" ウィンドウの座標に合わせる
		ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);

		// 座標変換 of 右手系補正
		Matrix4x4 projGizmo = camData.proj;
		projGizmo.m[2][2] = projGizmo.m[2][2] * 2.0f - projGizmo.m[2][3];
		projGizmo.m[3][2] = projGizmo.m[3][2] * 2.0f;

		// 入力判定用の代替ウィンドウとして "Game" ウィンドウをセットする
		ImGuizmo::SetAlternativeWindow(ImGui::GetCurrentWindow());

		// ギズモの上にマウスがあるか、ドラッグ操作中のときにフラグを立てて、
		// 次のフレームのGameウィンドウドラッグ移動を防止する
		bool isGizmoActive = ImGuizmo::IsOver() || ImGuizmo::IsUsing();
		EditorManager::GetInstance()->SetGizmoActive(isGizmoActive);

		static EulerTransform transformBeforeDrag;
		static bool wasUsingGizmo = false;

		if(ImGuizmo::IsOver() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			transformBeforeDrag = transform;
		}

		// 3. 描画は最前面に
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		ImGuizmo::Manipulate(
			&camData.view.m[0][0],
			&projGizmo.m[0][0],
			currentGizmoOperation,
			currentGizmoMode,
			&worldMatrix.m[0][0]
		);

		ImGui::End();
		if(ImGuizmo::IsUsing()) {
			wasUsingGizmo = true;
			float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			ImGuizmo::DecomposeMatrixToComponents(&worldMatrix.m[0][0], matrixTranslation, matrixRotation, matrixScale);
			transform.translate = { matrixTranslation[0], matrixTranslation[1], matrixTranslation[2] };
			const float DEG_TO_RAD = 3.14159265f / 180.0f;
			transform.rotate = {
				matrixRotation[0] * DEG_TO_RAD,
				matrixRotation[1] * DEG_TO_RAD,
				matrixRotation[2] * DEG_TO_RAD
			};
			transform.scale = { matrixScale[0], matrixScale[1], matrixScale[2] };
		}
		else if(wasUsingGizmo) {
			wasUsingGizmo = false;
			auto command = std::make_unique<TransformCommand>(selectedObject, transformBeforeDrag, transform);
			CommandManager::GetInstance()->AddAndExecute(std::move(command));
		}
	}
	// ---- アセットブラウザ ----
	ImGui::Begin("アセットブラウザ");
	if(currentDirectory_ != "Resources" && currentDirectory_.has_parent_path()) {
		if(ImGui::Button("Back (↑)")) {
			currentDirectory_ = currentDirectory_.parent_path();
		}
		ImGui::SameLine();
	}
	ImGui::Text("Path: %s", currentDirectory_.generic_string().c_str());
	ImGui::Separator();
	ImGui::Columns(2, "AssetBrowserSplit", true);
	static bool setColumnWidth = true;
	if(setColumnWidth) {
		ImGui::SetColumnWidth(0, 180.0f);
		setColumnWidth = false;
	}
	// 左ペイン (フォルダツリー)
	ImGui::BeginChild("FolderTreeChild", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
	if(currentDirectory_ == "Resources") {
		rootFlags |= ImGuiTreeNodeFlags_Selected;
	}
	bool rootOpen = ImGui::TreeNodeEx("Resources", rootFlags);
	if(ImGui::IsItemClicked()) {
		currentDirectory_ = "Resources";
	}
	if(rootOpen) {
		DrawDirectoryTree("Resources");
		ImGui::TreePop();
	}
	ImGui::EndChild();
	ImGui::NextColumn();
	// 右ペイン (ファイル一覧グリッド)
	ImGui::BeginChild("FileGridChild");
	float thumbnailSize = 90.0f;
	float padding = 16.0f;
	float cellSize = thumbnailSize + padding;
	float panelWidth = ImGui::GetContentRegionAvail().x;
	int columnCount = (int)(panelWidth / cellSize);
	if(columnCount < 1) columnCount = 1;
	ImGui::Columns(columnCount, nullptr, false);
	if(std::filesystem::exists(currentDirectory_) && std::filesystem::is_directory(currentDirectory_)) {
		int id = 0;
		for(const auto& entry : std::filesystem::directory_iterator(currentDirectory_)) {
			ImGui::PushID(id++);
			std::string filename = entry.path().filename().string();
			bool isDir = entry.is_directory();
			std::string label = isDir ? "[Folder]\n" + filename : "[File]\n" + filename;
			if(isDir) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.25f, 0.1f, 0.8f));
			}
			else {
				auto ext = entry.path().extension().string();
				if(ext == ".obj" || ext == ".gltf") {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.2f, 0.4f, 0.8f));
				}
				else if(ext == ".png" || ext == ".jpg") {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.3f, 0.1f, 0.8f));
				}
				else {
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
				}
			}
			if(ImGui::Button(label.c_str(), ImVec2(thumbnailSize, thumbnailSize))) {
				if(isDir) {
					currentDirectory_ = entry.path();
				}
				else {
					auto ext = entry.path().extension().string();
					if((ext == ".obj" || ext == ".gltf") && selectedObject != nullptr) {
						auto* meshRenderer = selectedObject->GetComponent<MeshRendererComponent>();
						if(meshRenderer != nullptr) {
							meshRenderer->SetModel(entry.path().generic_string());
						}
					}
				}
			}
			ImGui::PopStyleColor();
			if(ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(filename.c_str());
				ImGui::EndTooltip();
			}
			ImGui::NextColumn();
			ImGui::PopID();
		}
	}

	ImGui::Columns(1);
	ImGui::EndChild();
	ImGui::Columns(1);
	ImGui::End();
}

void LevelEditor::DrawDirectoryTree(const std::filesystem::path& path) {
	if(!std::filesystem::exists(path) || !std::filesystem::is_directory(path)) return;
	for(const auto& entry : std::filesystem::directory_iterator(path)) {
		if(entry.is_directory()) {
			std::string folderName = entry.path().filename().string();
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			if(currentDirectory_ == entry.path()) {
				flags |= ImGuiTreeNodeFlags_Selected;
			}
			bool hasSubDir = false;
			for(const auto& subEntry : std::filesystem::directory_iterator(entry.path())) {
				if(subEntry.is_directory()) {
					hasSubDir = true;
					break;
				}
			}
			if(!hasSubDir) {
				flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			}
			bool isOpen = ImGui::TreeNodeEx(folderName.c_str(), flags);
			if(ImGui::IsItemClicked()) {
				currentDirectory_ = entry.path();
			}
			if(isOpen && hasSubDir) {
				DrawDirectoryTree(entry.path());
				ImGui::TreePop();
			}
		}
	}
}

void LevelEditor::SaveScene(const std::vector<std::unique_ptr<GameObject>>& gameObjects) {
	json sceneJ;
	sceneJ["name"] = "scene";
	sceneJ["objects"] = json::array();
	for(const auto& obj : gameObjects) {
		sceneJ["objects"].push_back(obj->Serialize());
	}
	std::ofstream file("scene.json");
	if(file.is_open()) {
		file << sceneJ.dump(4);
		file.close();
	}
}

void LevelEditor::LoadScene(std::vector<std::unique_ptr<GameObject>>& gameObjects, GameObject*& selectedObject) {
	gameObjects.clear();
	selectedObject = nullptr;
	std::ifstream file("scene.json");
	if(file.is_open()) {
		json sceneJ;
		file >> sceneJ;
		file.close();
		if(sceneJ.contains("objects")) {
			for(const auto& objJ : sceneJ["objects"]) {
				auto newObj = std::make_unique<GameObject>(context_, objJ["name"]);
				newObj->Deserialize(objJ);
				newObj->Initialize();
				gameObjects.push_back(std::move(newObj));
			}
		}
	}
}