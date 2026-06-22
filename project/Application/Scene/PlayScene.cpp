#include "PlayScene.h"
#include <filesystem>
#include <format>
#include "RenderSystem.h"
#include "TextureManager.h"
#include "ModelFactory.h"
#include "ModelManager.h"
#include "CameraOrganizer.h"
#include "ShaderManager.h"
#include "MeshRendererComponent.h"
#include "imgui.h"
#include "imGuizmo.h"
#include "CommandManager.h"
#include "TransformCommand.h"

void PlayScene::Initialize() {
	if (!context_) return;

	// ライトマネージャーの初期化
	lightManager_ = std::make_unique<LightManager>();
	lightManager_->Initialize(context_->device);

	// 初期ライトとして DirectionalLight を 1 つ追加して設定
	lightManager_->AddLight(LightType::DIRECTIONALLIGHT);
	lightManager_->SetDirectionalLightDir(0, { 0.5f, -1.0f, 0.5f }); // 斜め下
	lightManager_->SetDirectionalLightColor(0, { 1.0f, 1.0f, 1.0f, 1.0f }); // 白色
	lightManager_->SetDirectionalLightIntensity(0, 1.0f); // 輝度 1.0

	// 起動時にアセットリストを1回スキャン
	RefreshAssetList();
}

void PlayScene::Update(CameraData* cameraData) {
	// カメラの更新
	CameraOrganizer::GetInstance()->Update();

	// ライトの更新
	if (lightManager_) {
		lightManager_->Update();
	}

	for (auto& obj : gameObjects_) {
		obj->Update();
	}

#ifdef USEIMGUI
	// Ctrl + Z で Undo (元に戻す)
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
		CommandManager::GetInstance()->Undo();
	}

	// Ctrl + Y で Redo (やり直す)
	if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
		CommandManager::GetInstance()->Redo();
	}

	// ヒエラルキー
	ImGui::Begin("Hierarchy");

	// シーン保存ボタン
	if (ImGui::Button("Save Scene")) {
		json sceneJ;
		sceneJ["name"] = "scene";
		sceneJ["objects"] = json::array();

		for (auto& obj : gameObjects_) {
			sceneJ["objects"].push_back(obj->Serialize());
		}

		// ファイルに書き出す
		std::ofstream file("scene.json");
		if (file.is_open()) {
			file << sceneJ.dump(4); // インデント4マスで保存
			file.close();
		}
	}

	ImGui::SameLine();

	// シーン読込ボタン
	if (ImGui::Button("Load Scene")) {
		// 現在のオブジェクトを全削除してリセット
		gameObjects_.clear();
		selectedObject_ = nullptr;

		// ファイルから読み込む
		std::ifstream file("scene.json");
		if (file.is_open()) {
			json sceneJ;
			file >> sceneJ;
			file.close();

			if (sceneJ.contains("objects")) {
				for (const auto& objJ : sceneJ["objects"]) {
					// GameObjectを生成
					auto newObj = std::make_unique<GameObject>(context_, objJ["name"]);
					newObj->Deserialize(objJ);

					// 各コンポーネントのInitializeを呼んで3Dモデルなどのロードを走らせる
					newObj->Initialize(); 

					gameObjects_.push_back(std::move(newObj));
				}
			}
		}
	}

	// オブジェクトの新規作成ボタン
	if (ImGui::Button("空オブジェクト作成")) {

		// 新しいオブジェクトを作成してリストに追加
		std::string newName = std::format("GameObject_{}", gameObjects_.size());
		auto newObj = std::make_unique<GameObject>(context_, newName);

		// 作成したものをそのまま選択状態にする
		selectedObject_ = newObj.get();
		gameObjects_.push_back(std::move(newObj));
	}

	ImGui::Separator();

	// オブジェクトの一覧表示
	for (auto& obj : gameObjects_) {
		bool isSelected = (selectedObject_ == obj.get());

		// クリックしたら選択状態にする
		if (ImGui::Selectable(obj->GetName().c_str(), isSelected)) {
			selectedObject_ = obj.get();
		}
	}
	ImGui::End();

	// インスペクター
	ImGui::Begin("インスペクター");
	if (selectedObject_ != nullptr) {
		// 選択されているオブジェクトの情報を表示
		selectedObject_->ImGui();
	} 
	else {
		ImGui::Text("オブジェクトが選択されていません");
	}
	ImGui::End();

	// ライト用
	ImGui::Begin("ライト編集");
	lightManager_->ImGui();
	ImGui::End();

	// マニピュレータ
	if (selectedObject_ != nullptr) {
		ImGuizmo::BeginFrame();

		// 矢印がチラチラ反転するのを防ぐ
		ImGuizmo::AllowAxisFlip(false);

		// 画面サイズ設定
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		// カメラ行列の取得
		CameraData& camData = CameraOrganizer::GetInstance()->GetCameraData();

		// DirectXのプロジェクション行列をImGuizmo（OpenGL仕様）向けに補正する
		Matrix4x4 projGizmo = camData.proj;
		projGizmo.m[2][2] = projGizmo.m[2][2] * 2.0f - projGizmo.m[2][3];
		projGizmo.m[3][2] = projGizmo.m[3][2] * 2.0f;

		// 選択オブジェクトのTransformから行列生成
		EulerTransform& transform = selectedObject_->GetTransform();
		Matrix4x4 worldMatrix = Math::MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		
		// 操作モード設定
		static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);
		static ImGuizmo::MODE currentGizmoMode(ImGuizmo::WORLD);
		if (ImGui::IsKeyPressed(ImGuiKey_T)) currentGizmoOperation = ImGuizmo::TRANSLATE;
		if (ImGui::IsKeyPressed(ImGuiKey_R)) currentGizmoOperation = ImGuizmo::ROTATE;
		if (ImGui::IsKeyPressed(ImGuiKey_E)) currentGizmoOperation = ImGuizmo::SCALE;

		// 画面全体を覆う透明なImGuiウィンドウを作成し、その中でギズモを処理する
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);
		
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | 
		                               ImGuiWindowFlags_NoResize | 
		                               ImGuiWindowFlags_NoScrollbar | 
		                               ImGuiWindowFlags_NoSavedSettings | 
		                               ImGuiWindowFlags_NoBackground;
		
		// 自前でオブジェクトの2D座標を求めて、マウスがギズモの近くにあるか判定する
		bool isHoverGizmo = false;
		if (ImGuizmo::IsUsing()) {
			isHoverGizmo = true;
		} else {
			// オブジェクトの3D位置を2Dスクリーン座標に変換
			Vector3 ndcPos = Math::ChangeTransform(transform.translate, camData.vp);
			float screenX = (ndcPos.x + 1.0f) * 0.5f * io.DisplaySize.x;
			float screenY = (1.0f - ndcPos.y) * 0.5f * io.DisplaySize.y;

			// マウスとギズモの距離を計算（半径150ピクセル以内ならホバーとみなす）
			float dx = io.MousePos.x - screenX;
			float dy = io.MousePos.y - screenY;
			float distSq = dx * dx + dy * dy;
			if (distSq < 150.0f * 150.0f) {
				isHoverGizmo = true;
			}
		}

		// ギズモの近くにマウスがなければ、入力をスルーしてカメラ操作できるようにする
		if (!isHoverGizmo) {
			windowFlags |= ImGuiWindowFlags_NoInputs;
		}

		ImGui::Begin("GizmoWindow", nullptr, windowFlags);
		
		// このウィンドウのDrawListをImGuizmoに渡して描画する
		ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

		// ドラッグ開始時の状態を記録するための変数
		static EulerTransform transformBeforeDrag;
		static bool wasUsingGizmo = false;
		// ドラッグを開始した瞬間、現在の状態をキャッシュしておく
		if (ImGuizmo::IsOver() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			transformBeforeDrag = transform; // 現在の値を保存
		}

		// ギズモの描画とドラッグ操作（補正したプロジェクション行列を渡す）
		ImGuizmo::Manipulate(
			&camData.view.m[0][0],
			&projGizmo.m[0][0],
			currentGizmoOperation,
			currentGizmoMode,
			&worldMatrix.m[0][0]
		);

		ImGui::End();

		// 操作中なら値をオブジェクトに反映する
		if (ImGuizmo::IsUsing()) {
			wasUsingGizmo = true;
			float matrixTranslation[3], matrixRotation[3], matrixScale[3];

			// 転置なしの worldMatrix をデコンポーズ
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
		// ドラッグが終了した瞬間（マウスを離した時）、コマンド履歴に登録
		else if (wasUsingGizmo) {
			wasUsingGizmo = false;

			// ドラッグ前とドラッグ後の差分コマンドを作成してマネージャーに積む
			auto command = std::make_unique<TransformCommand>(selectedObject_, transformBeforeDrag, transform);
			CommandManager::GetInstance()->AddAndExecute(std::move(command));
		}
	}

	// アセットブラウザ
	ImGui::Begin("アセットブラウザ");
	// 手動更新用のリフレッシュボタン
	if (ImGui::Button("更新")) {
		RefreshAssetList();
	}
	ImGui::Separator();
	ImGui::Spacing();
	// 見つかったモデルファイルをリスト表示
	for (const auto& filePath : modelFiles_) {
		// 表示用に「フォルダパス」を除外して「ファイル名」だけを抽出する（例: player.obj）
		std::string fileName = std::filesystem::path(filePath).filename().string();

		// リストの項目を表示
		if (ImGui::Selectable(fileName.c_str())) {
			// 項目がクリックされたとき、もしGameObjectが選択されていて、
			// かつ MeshRendererComponent を持っていればそのモデルを適用する！
			if (selectedObject_ != nullptr) {
				auto* meshRenderer = selectedObject_->GetComponent<MeshRendererComponent>();
				if (meshRenderer != nullptr) {
					meshRenderer->SetModel(filePath);
				}
			}
		}
	}
	ImGui::End();
#endif
}

void PlayScene::Draw(RenderSystem* renderSystem) {
	// ライトマネージャーを RenderSystem に登録
	if (lightManager_ && renderSystem) {
		renderSystem->SetLightManager(lightManager_.get());
	}

	for (auto& obj : gameObjects_) {
		obj->Draw(renderSystem);
	}
}

void PlayScene::RefreshAssetList() {
	modelFiles_.clear();
	std::string path = "Resources";

	// std::filesystem を使ってフォルダ内を再帰的に探索するよ
	if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
		for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
			if (entry.is_regular_file()) {
				auto ext = entry.path().extension().string();

				// 3Dモデルファイル（.obj や .gltf）だけをフィルタリング
				if (ext == ".obj" || ext == ".gltf") {
					// Windowsのバックスラッシュ "\" を "/" に統一して追加する
					modelFiles_.push_back(entry.path().generic_string());
				}
			}
		}
	}
}