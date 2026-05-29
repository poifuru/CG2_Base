#include "ImGuiManager.h"
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include "DxCommon.h"
#include "Windows.h"
#include "SRVManager.h"
#include "RenderTexture.h"

ImGuiManager::~ImGuiManager () {
#ifdef USEIMGUI
	//ImGuiの終了処理
	ImGui_ImplDX12_Shutdown ();
	ImGui_ImplWin32_Shutdown ();
	ImGui::DestroyContext ();
#endif
}

void ImGuiManager::Initialize () {
#ifdef USEIMGUI
	IMGUI_CHECKVERSION ();
	ImGui::CreateContext ();
	// ドッキング機能を有効化
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark ();
	ImGuiIO& io = ImGui::GetIO ();
	ImFont* fontJP = io.Fonts->AddFontFromFileTTF (
		"Resources/AppliMincho/PottaOne-Regular.ttf", 17.0f, nullptr,
		io.Fonts->GetGlyphRangesJapanese ());
	io.FontDefault = fontJP;
	ImGui_ImplWin32_Init (WindowsAPI::GetInstance()->GetHwnd ());

	uint32_t srvIndex = SRVManager::GetInstance()->Allocate();

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = DxCommon::GetInstance()->GetDevice();
	initInfo.CommandQueue = DxCommon::GetInstance()->GetCommandQueue();
	initInfo.NumFramesInFlight = 2;
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	initInfo.SrvDescriptorHeap = SRVManager::GetInstance()->GetDescriptorHeap();
	initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle) {
		uint32_t srvIndex = SRVManager::GetInstance()->Allocate();
		*out_cpu_desc_handle = SRVManager::GetInstance()->GetCPUDescriptorHandle(srvIndex);
		*out_gpu_desc_handle = SRVManager::GetInstance()->GetGPUDescriptorHandle(srvIndex);
		};
	initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle) {
		uint32_t srvIndex = SRVManager::GetInstance()->GetIndex(cpu_desc_handle);
		SRVManager::GetInstance()->Free(srvIndex);
		};

	ImGui_ImplDX12_Init (&initInfo);
#endif
}

void ImGuiManager::Draw () {
#ifdef USEIMGUI
	//ImGuiの内部コマンドを生成する
	ImGui::Render ();
	//実際のImGui描画コマンドを詰む
	ImGui_ImplDX12_RenderDrawData (ImGui::GetDrawData (), DxCommon::GetInstance ()->GetCommandList());
#endif
}

void ImGuiManager::BeginFrame () {
#ifdef USEIMGUI
	//フレームの先頭をImGuiに伝えてあげる
	// バックエンドの初期化
	ImGui_ImplWin32_NewFrame ();
	ImGui_ImplDX12_NewFrame ();

	ImGui::NewFrame ();

	RenderDockingSpace();
#endif
}

void ImGuiManager::RenderDockingSpace() {
#ifdef USEIMGUI
	// 画面全体（ビューポート）のサイズや位置を取得
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	// 背景やタイトルバー、枠線をすべて非表示にするためのフラグ
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoNavFocus;

	// 画面にぴったり合わせるために隙間（パディング）をゼロにする
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	// 全画面の透明なウィンドウを作成
	ImGui::Begin("MyEngineMainDockSpaceWindow", nullptr, window_flags);

	ImGui::PopStyleVar(3);

	// このウィンドウの中に「ドックスペース」を設置
	ImGuiID dockspace_id = ImGui::GetID("MyEngineDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

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

	ImGui::End();
#endif
}
