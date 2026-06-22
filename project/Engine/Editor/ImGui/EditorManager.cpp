#include "EditorManager.h"
#include <imgui.h>
#include "RenderTexture.h"
#include "DescriptorHeapManager.h"

void EditorManager::UpdateAndDraw(IEngine* engine) {
#ifdef USEIMGUI
	// 各ウィンドウを順番に描画していく
	DrawGameWindow(engine);
	DrawInspectorWindow();
#endif
}

void EditorManager::DrawGameWindow(IEngine* engine) {
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
	RenderTexture* renderTexture = engine->GetRenderTexture();
	if (renderTexture) {
		uint32_t srvIndex = renderTexture->GetSrvIndex();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = engine->GetDescriptorHeapManager()->GetGpuHandle(srvIndex);

		// ウィンドウの大きさに合わせてゲーム画面を描画
		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		ImGui::Image((ImTextureID)gpuHandle.ptr, contentSize);
	}

	ImGui::End();
}

void EditorManager::DrawInspectorWindow() {
	ImGui::Begin("インスペクター");
	ImGui::End();
}
