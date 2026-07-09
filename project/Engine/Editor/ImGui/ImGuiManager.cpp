#include "PCH.h"
#include "ImGuiManager.h"
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>
#include "WindowsAPI.h"
#include "DescriptorHeapManager.h"
#include "EditorManager.h"
#include "Engine.h"

ImGuiManager::~ImGuiManager() {
	Finalize();
}

void ImGuiManager::Finalize() {
#ifdef USEIMGUI
	if (ImGui::GetCurrentContext()) {
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}
#endif
	engine_ = nullptr;
}

void ImGuiManager::Initialize(MyEngine::LowLevel::Engine* engine) {
	engine_ = engine;
#ifdef USEIMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();
	ImGuiIO& io = ImGui::GetIO();
	ImFont* fontJP = io.Fonts->AddFontFromFileTTF(
		"Resources/AppliMincho/PottaOne-Regular.ttf", 17.0f, nullptr,
		io.Fonts->GetGlyphRangesJapanese());
	io.FontDefault = fontJP;
	ImGui_ImplWin32_Init(WindowsAPI::GetInstance()->GetHwnd());

	MyEngine::LowLevel::DescriptorHeapManager* heapManager = engine_->GetDescriptorHeapManager();

	ImGui_ImplDX12_InitInfo initInfo = {};
	initInfo.Device = engine_->GetDevice();
	initInfo.CommandQueue = engine_->GetCommandQueue();
	initInfo.NumFramesInFlight = 3;
	initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	initInfo.SrvDescriptorHeap = heapManager->GetHeap();
	
	initInfo.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_desc_handle) {
		MyEngine::LowLevel::DescriptorHeapManager* mgr = ImGuiManager::GetInstance()->GetEngine()->GetDescriptorHeapManager();
		uint32_t srvIndex = mgr->AllocateIndex();
		*out_cpu_desc_handle = mgr->GetCpuHandle(srvIndex);
		*out_gpu_desc_handle = mgr->GetGpuHandle(srvIndex);
	};
	initInfo.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_desc_handle) {
		MyEngine::LowLevel::DescriptorHeapManager* mgr = ImGuiManager::GetInstance()->GetEngine()->GetDescriptorHeapManager();
		uint32_t srvIndex = mgr->GetIndex(cpu_desc_handle);
		mgr->FreeIndex(srvIndex);
	};

	ImGui_ImplDX12_Init(&initInfo);
#endif
}

void ImGuiManager::Draw() {
#ifdef USEIMGUI
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), engine_->GetCommandList());
#endif
}

void ImGuiManager::BeginFrame() {
#ifdef USEIMGUI
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX12_NewFrame();
	ImGui::NewFrame();

	RenderDockingSpace();
#endif
}

void ImGuiManager::RenderDockingSpace() {
#ifdef USEIMGUI
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoNavFocus;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("MyEngineMainDockSpaceWindow", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	ImGuiID dockspace_id = ImGui::GetID("MyEngineDockSpace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

	ImGui::Begin("View");
	ImGui::End();

	EditorManager::GetInstance()->UpdateAndDraw(engine_);

	ImGui::End();
#endif
}