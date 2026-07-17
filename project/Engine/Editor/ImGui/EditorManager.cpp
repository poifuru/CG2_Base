#include "PCH.h"
#include "EditorManager.h"
#include <imgui.h>
#include "RenderTexture.h"
#include "DescriptorHeapManager.h"
#include "Renderer.h"
#include "RenderSystem.h"
#include "Engine.h"
#include <psapi.h>

void EditorManager::UpdateAndDraw(
	ID3D12Device* device,
	MyEngine::LowLevel::DescriptorHeapManager* heapManager,
	MyEngine::Rendering::RenderTexture* renderTexture
) {
#ifdef USEIMGUI
	// 各ウィンドウを順番に描画していく
	DrawGameWindow(heapManager, renderTexture);

	DrawPerformanceWidget(device);
#endif
}

void EditorManager::DrawGameWindow(
	MyEngine::LowLevel::DescriptorHeapManager* heapManager,
	MyEngine::Rendering::RenderTexture* renderTexture
) {
	// ギズモ操作中はウィンドウが動かないようにする
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
	if (isGizmoActive_) {
		windowFlags |= ImGuiWindowFlags_NoMove;
	}

	// ゲーム画面をImGuiウィンドウとして描画する
	ImGui::Begin("Game", nullptr, windowFlags);

	// アスペクト比選択コンボボックスの配置
	const char* aspectNames[] = { "16:9", "4:3", "Free (Fit)" };
	ImGui::SetNextItemWidth(120.0f);
	ImGui::Combo("Aspect", &selectedAspectIndex_, aspectNames, IM_ARRAYSIZE(aspectNames));
	ImGui::Separator();

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
	if(renderTexture) {
		uint32_t srvIndex = renderTexture->GetSrvIndex();
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = heapManager->GetGpuHandle(srvIndex);

		// ウィンドウで現在利用可能な領域を取得
		ImVec2 availSize = ImGui::GetContentRegionAvail();

		// 現在の描画カーソルのスクリーン座標（絶対座標）を取得
		ImVec2 screenPos = ImGui::GetCursorScreenPos();

		// 選択されたアスペクト比のターゲットを決定
		float targetAspect = 16.0f / 9.0f;
		bool isAspectFixed = true;
		if(selectedAspectIndex_ == 0) {
			targetAspect = 16.0f / 9.0f;
		}
		else if(selectedAspectIndex_ == 1) {
			targetAspect = 4.0f / 3.0f;
		}
		else {
			isAspectFixed = false; // 自由変形
		}
		ImVec2 imageSize = availSize;

		// アスペクト比を固定する場合のサイズ計算
		if(isAspectFixed && availSize.y > 0.0f) {
			float availAspect = availSize.x / availSize.y;
			if(availAspect > targetAspect) {
				// ウィンドウが横長すぎる場合 ➔ 高さに合わせる
				imageSize.y = availSize.y;
				imageSize.x = availSize.y * targetAspect;
			}
			else {
				// ウィンドウが縦長すぎる場合 ➔ 幅に合わせる
				imageSize.x = availSize.x;
				imageSize.y = availSize.x / targetAspect;
			}

			// 画面をウィンドウ中央に寄せる(センタリング)
			float offsetX = (availSize.x - imageSize.x) * 0.5f;
			float offsetY = (availSize.y - imageSize.y) * 0.5f;

			ImVec2 cursorPos = ImGui::GetCursorPos();
			cursorPos.x += offsetX;
			cursorPos.y += offsetY;
			ImGui::SetCursorPos(cursorPos);

			// スクリーン座標も中央寄せ分ずらす
			screenPos.x += offsetX;
			screenPos.y += offsetY;
		}

		// 実際の描画位置とサイズをメンバ変数に保存
		gameScreenPos_ = screenPos;
		gameScreenSize_ = imageSize;

		// 計算したサイズで描画
		ImGui::Image((ImTextureID)gpuHandle.ptr, imageSize);
	}

	ImGui::End();
}

void EditorManager::DrawPerformanceWidget(ID3D12Device* device) {
	ImGui::Begin("Performance Debug");
	// FPSとCPU時間の表示
	float fps = ImGui::GetIO().Framerate;
	float frameTimeMs = 1000.0f / fps;
	ImGui::Text("FPS: %.1f", fps);
	ImGui::Text("Frame Time: %.3f ms", frameTimeMs);

	// FPSの簡易グラフ表示
	static float fpsHistory[120] = {};
	static int offset = 0;
	fpsHistory[offset] = fps;
	offset = (offset + 1) % 120;
	ImGui::PlotLines("FPS History", fpsHistory, 120, offset, nullptr, 0.0f, 120.0f, ImVec2(0, 50));
	ImGui::Separator();

	// ドローコール数とメインメモリ使用量の表示
	ImGui::Text("Draw Calls: %u", MyEngine::Rendering::RenderSystem::GetDrawCallCount());
	
	PROCESS_MEMORY_COUNTERS memCounter;
	GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof(memCounter));
	float ramUsageMB = static_cast<float>(memCounter.WorkingSetSize) / (1024.0f * 1024.0f);
	ImGui::Text("RAM Usage: %.1f MB", ramUsageMB);
	ImGui::Separator();

	// GPU情報とVRAM使用量の取得 (DirectX 12 / DXGI)
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	if (device && SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&dxgiDevice)))) {
		Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
		if (SUCCEEDED(dxgiDevice->GetAdapter(&dxgiAdapter))) {

			// GPUのグラフィックボード名を取得して表示
			DXGI_ADAPTER_DESC desc;
			dxgiAdapter->GetDesc(&desc);
			std::wstring wName(desc.Description);
			std::string gpuName;
			gpuName.reserve(wName.size());
			for (wchar_t w : wName) {
				gpuName.push_back(static_cast<char>(w));
			}
			ImGui::Text("GPU: %s", gpuName.c_str());

			// VRAM（ビデオメモリ）の使用状況を取得 (IDXGIAdapter3へキャストが必要)
			Microsoft::WRL::ComPtr<IDXGIAdapter3> dxgiAdapter3;
			if (SUCCEEDED(dxgiAdapter.As(&dxgiAdapter3))) {
				DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo{};
				dxgiAdapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);

				// バイトからMBに変換
				float usageMB = static_cast<float>(memoryInfo.CurrentUsage) / (1024.0f * 1024.0f);
				float budgetMB = static_cast<float>(memoryInfo.Budget) / (1024.0f * 1024.0f);

				ImGui::Text("VRAM: %.1f MB / %.1f MB", usageMB, budgetMB);

				// メモリ使用率のプログレスバー
				ImGui::ProgressBar(usageMB / budgetMB, ImVec2(0.0f, 0.0f));
			}
		}
	}
	ImGui::Separator();

	// CPUプロファイラ時間の表示
	ImGui::Text("CPU Profiler");
	float updateTime = MyEngine::LowLevel::Engine::GetUpdateTime();
	float renderTime = MyEngine::LowLevel::Engine::GetRenderTime();
	float waitTime = MyEngine::LowLevel::Engine::GetGpuWaitTime();
	float totalFrameTime = updateTime + renderTime + waitTime;

	// 割合の計算
	float updateRatio = totalFrameTime > 0.0f ? updateTime / totalFrameTime : 0.0f;
	float renderRatio = totalFrameTime > 0.0f ? renderTime / totalFrameTime : 0.0f;
	float waitRatio   = totalFrameTime > 0.0f ? waitTime / totalFrameTime : 0.0f;

	// Update (Game)
	ImGui::Text("Update (Game): %.3f ms", updateTime);
	ImGui::ProgressBar(updateRatio, ImVec2(-60.0f, 0.0f), "");
	ImGui::SameLine();
	ImGui::Text("%.0f%%", updateRatio * 100.0f);

	// Render (CPU)
	ImGui::Text("Render (CPU): %.3f ms", renderTime);
	ImGui::ProgressBar(renderRatio, ImVec2(-60.0f, 0.0f), "");
	ImGui::SameLine();
	ImGui::Text("%.0f%%", renderRatio * 100.0f);

	// GPU Wait
	ImGui::Text("GPU Wait: %.3f ms", waitTime);
	ImGui::ProgressBar(waitRatio, ImVec2(-60.0f, 0.0f), "");
	ImGui::SameLine();
	ImGui::Text("%.0f%%", waitRatio * 100.0f);

	ImGui::End();
}