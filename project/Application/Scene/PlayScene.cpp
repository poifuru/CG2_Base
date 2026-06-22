#include "PlayScene.h"
#include "RenderSystem.h"
#include "TextureManager.h"
#include "ModelFactory.h"
#include "ModelManager.h"
#include "CameraOrganizer.h"
#include "ShaderManager.h"
#include <format>
#include "MeshRendererComponent.h"
#include "imgui.h"
#include "imGuizmo.h"

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
	// ヒエラルキー
	ImGui::Begin("Hierarchy");

	// オブジェクトの新規作成ボタン
	if (ImGui::Button("Create Empty GameObject")) {

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
	ImGui::Begin("Inspector");
	if (selectedObject_ != nullptr) {
		// 選択されているオブジェクトの情報を表示
		selectedObject_->ImGui();
	} else {
		ImGui::Text("Select an object in Hierarchy to inspect.");
	}
	ImGui::End();

	// ライト用
	ImGui::Begin("Light Edit");
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
	}
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