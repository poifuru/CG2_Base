#include "EditorManager.h"
#include "DxCommon.h"
#include "SRVManager.h"
#include "RenderTexture.h"

void EditorManager::UpdateAndDraw() {
#ifdef USEIMGUI
	// 各ウィンドウを順番に描画していく
	DrawViewportWindow();
	DrawGameWindow();
	DrawInspectorWindow();
#endif
}

void EditorManager::DrawViewportWindow() {
	ImGui::Begin("Viewport");
	ImGui::Text("ここにエディタ用のカメラ操作画面とかを作る予定");
	ImGui::End();
}

void EditorManager::DrawGameWindow() {
	// ゲーム画面をImGuiウィンドウとして描画する
	ImGui::Begin("Game");

	// 純粋にウィンドウ上にマウスがあるか
	bool isHovered = ImGui::IsWindowHovered();
	isGameWindowFocused_ = ImGui::IsWindowFocused();

	// ドラッグ開始判定：ウィンドウ上でクリックされたらドラッグ中フラグをON
	if (isHovered && ImGui::IsAnyMouseDown()) {
		isGameWindowDragging_ = true;
	}

	// ドラッグ終了判定：マウスボタンが全て離されたらフラグをOFF
	if (!ImGui::IsAnyMouseDown()) {
		isGameWindowDragging_ = false;
	}

	// 「ウィンドウ上にマウスがある」か「ゲームウィンドウからドラッグ中」なら、ホバー状態とみなす
	isGameWindowHovered_ = isHovered || isGameWindowDragging_;

	// RenderTextureのSRVからGPUハンドルを取得
	uint32_t srvIndex = DxCommon::GetInstance()->GetPostEffectRenderTexture()->GetSrvIndex();
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = SRVManager::GetInstance()->GetGPUDescriptorHandle(srvIndex);

	// ウィンドウの大きさに合わせてゲーム画面を描画
	ImVec2 contentSize = ImGui::GetContentRegionAvail();
	ImGui::Image((ImTextureID)gpuHandle.ptr, contentSize);

	ImGui::End();
}

void EditorManager::DrawInspectorWindow() {
	// 将来的にここを増やすだけで、ImGuiManagerを一切汚さずに画面を拡張できる！
	ImGui::Begin("Inspector");
	ImGui::Text("Selected Object: None");
	ImGui::End();
}
