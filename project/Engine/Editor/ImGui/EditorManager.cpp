#include "PCH.h"
#include "EditorManager.h"
#include <imgui.h>
#include "RenderTexture.h"
#include "DescriptorHeapManager.h"
#include "Engine.h"

void EditorManager::UpdateAndDraw(Engine* engine) {
#ifdef USEIMGUI
	// 各ウィンドウを順番に描画していく
	DrawGameWindow(engine);
#endif
}

void EditorManager::DrawGameWindow(Engine* engine) {
	// ギズモ操作中はウィンドウが動かないようにする
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
	if (isGizmoActive_) {
		windowFlags |= ImGuiWindowFlags_NoMove;
	}

	// ゲーム画面をImGuiウィンドウとして描画する
	ImGui::Begin("Game", nullptr, windowFlags);

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