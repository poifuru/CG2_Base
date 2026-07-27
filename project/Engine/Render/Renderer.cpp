#include "PCH.h"
#include "Renderer.h"
#include "RootSignatureManager.h"
#include "ShaderManager.h"
#include "InputLayoutManager.h"
#include "BlendModeManager.h"
#include "PSOManager.h"
#include "RenderSystem.h"
#include "RenderTexture.h"
#include "RenderingModel.h"
#include "WindowsAPI.h"
#include "CameraOrganizer.h"
#include "MainCameraComponent.h"
#include "Function.h"
#include "Model.h"
#include "Material.h"
#include "MeshData.h"
#include "ComponentType.h"

MyEngine::Rendering::Renderer::Renderer() = default;
MyEngine::Rendering::Renderer::~Renderer() = default;

void MyEngine::Rendering::Renderer::Initialize(
	ID3D12Device* device,
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,
	MyEngine::LowLevel::DescriptorHeapManager* heapManager
) {
	device_ = device;

	rootSigManager_ = std::make_unique<MyEngine::Rendering::RootSignatureManager>();
	rootSigManager_->Initialize(device);

	shaderManager_ = std::make_unique<MyEngine::Rendering::ShaderManager>();
	shaderManager_->Initialize(
		dxcUtils,
		dxcCompiler,
		includeHandler
	);

	inputLayoutManager_ = std::make_unique<MyEngine::Rendering::InputLayoutManager>();
	inputLayoutManager_->Initialize();

	blendModeManager_ = std::make_unique<MyEngine::Rendering::BlendModeManager>();
	blendModeManager_->Initialize();

	psoManager_ = std::make_unique<MyEngine::Rendering::PSOManager>();

	renderSystem_ = std::make_unique<MyEngine::Rendering::RenderSystem>();
	renderSystem_->Initialize(
		device,
		heapManager,
		psoManager_.get(),
		shaderManager_.get(),
		inputLayoutManager_.get(),
		blendModeManager_.get(),
		rootSigManager_->GetCommonRootSignature()
	);

	renderTexture_ = std::make_unique<MyEngine::Rendering::RenderTexture>();
	renderTexture_->Initialize(device, heapManager);

	InitializeShaderTable();
}

namespace {
	void SetupViewport(ID3D12GraphicsCommandList* cmdList) {
		D3D12_VIEWPORT viewport{};
		viewport.Width = 1280.0f;
		viewport.Height = 720.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		cmdList->RSSetViewports(1, &viewport);
	}

	void SetupScissorRect(ID3D12GraphicsCommandList* cmdList) {
		D3D12_RECT scissorRect{};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = 1280;
		scissorRect.bottom = 720;

		cmdList->RSSetScissorRects(1, &scissorRect);
	}
}

void MyEngine::Rendering::Renderer::RenderScene(
	ID3D12GraphicsCommandList* cmdList,
	MyEngine::LowLevel::DescriptorHeapManager* heapManager
) {
	SetupViewport(cmdList);
	SetupScissorRect(cmdList);

	// 全コンポーネントのCSを一括実行
	DispatchCS(cmdList, heapManager);

	// カメラ座標を取得して RenderSystem に設定
	Vector3 cameraPos = CameraOrganizer::GetInstance()->GetCameraData().transform.translate;
	renderSystem_->SetCameraPosition(cameraPos);

	renderSystem_->WriteCommandList(cmdList);
	renderSystem_->ClearCommands();

	// RenderTextureへの描画が終わったのでSRVに遷移
	MyEngine::Utility::TransitionBarrier(
		cmdList,
		renderTexture_->GetResource(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void MyEngine::Rendering::Renderer::DispatchCS(
	ID3D12GraphicsCommandList* cmdList,
	MyEngine::LowLevel::DescriptorHeapManager* heapManager
) {
	// ★【ここを追加！】CSでディスクリプタヒープを使う前にコマンドリストへセットする！
	if (heapManager) {
		heapManager->SetGraphicsHeap(cmdList);
	}

	if (currentObjects_) {
		for (auto& obj : *currentObjects_) {
			if (auto* wakeComp = obj->GetComponent<BoatWakeComponent>()) {
				wakeComp->DispatchCS(cmdList);
			}
		}
	}
}

void MyEngine::Rendering::Renderer::Draw(std::vector<std::unique_ptr<GameObject>>& objects) {
	currentObjects_ = &objects; // 参照を保存

	for(auto& obj : objects) {

		// GameObjectの名前をあらかじめ取得しておく
		const char* objectName = obj->GetName().c_str();

		// 3Dモデルの描画
		if(auto* meshRenderer = obj->GetComponent<MeshRendererComponent>()) {
			// MeshRendererComponentを継承してるComponentの場合
			if(!obj->GetComponent<WaterSurfaceComponent>()) {
				if(auto* model = meshRenderer->GetModel()) {
					auto* modelData = model->GetModelData();
					auto* material = model->GetMaterial();
					auto transformAddr = model->GetTransformGPUAddress();
					if(modelData && material) {
						for(const auto& mesh : modelData->meshes) {
							Submit(mesh, material, transformAddr, objectName);
						}
					}
				}
			}
		}
		// 2Dスプライトの描画
		if(auto* spriteComp = obj->GetComponent<SpriteComponent>()) {
			if(auto* model = spriteComp->GetModel()) {
				auto* modelData = model->GetModelData();
				auto* material = model->GetMaterial();
				auto transformAddr = model->GetTransformGPUAddress();
				if(modelData && material) {
					for(const auto& mesh : modelData->meshes) {
						Submit(mesh, material, transformAddr, objectName);
					}
				}
			}
		}
		// スカイボックスの描画
		if(auto* skybox = obj->GetComponent<SkyboxComponent>()) {
			Submit(
				skybox->GetMesh(),
				skybox->GetMaterial(),
				skybox->GetTransformAddress(),
				objectName
			);
		}
		// ナンバードローワーの描画 
		if(auto* numDrawer = obj->GetComponent<NumberDrawerComponent>()) {
			for(const auto& model : numDrawer->GetDigitModels()) {
				if(model) {
					auto* modelData = model->GetModelData();
					auto* material = model->GetMaterial();
					auto transformAddr = model->GetTransformGPUAddress();
					if(modelData && material) {
						for(const auto& mesh : modelData->meshes) {
							Submit(mesh, material, transformAddr, objectName);
						}
					}
				}
			}
		}
		// 水面コンポーネントの描画
		if(auto* waterComp = obj->GetComponent<WaterSurfaceComponent>()) {
			if(auto* model = waterComp->GetModel()) {
				auto* modelData = model->GetModelData();
				auto* material = model->GetMaterial();
				auto transformAddr = model->GetTransformGPUAddress();
				auto customAddr = waterComp->GetCustomBufferAddress();

				if(modelData && material) {
					for(const auto& mesh : modelData->meshes) {
						// Submitをオーバーロードするか、引数を拡張して customAddr を渡す
						Submit(mesh, material, transformAddr, objectName, customAddr);
					}
				}
			}
		}
		// BoatWakeComponent の描画
		if (auto* wakeComp = obj->GetComponent<BoatWakeComponent>()) {
			/*if (auto* material = wakeComp->GetMaterial()) {
				const auto& mesh = wakeComp->GetMesh();
				if (mesh.vertexCount > 0) {
					auto transformAddr = wakeComp->GetTransformAddress();
					Submit(mesh, material, transformAddr, objectName);
				}
			}*/

			OutputDebugStringA(">>> [1] Found BoatWakeComponent!\n");
			if (auto* material = wakeComp->GetMaterial()) {
				OutputDebugStringA(">>> [2] Material is OK!\n");
				const auto& mesh = wakeComp->GetMesh();
				char buf[256];
				sprintf_s(buf, ">>> [3] VertexCount = %u\n", mesh.vertexCount);
				OutputDebugStringA(buf);
				if (mesh.vertexCount > 0) {
					OutputDebugStringA(">>> [4] Submitting BoatWake to RenderSystem!\n");
					auto transformAddr = wakeComp->GetTransformAddress();
					Submit(mesh, material, transformAddr, objectName);
				}
			} else {
				OutputDebugStringA(">>> [ERR] Material is NULL!\n");
			}
		}
	}
}

uint64_t MyEngine::Rendering::Renderer::MakeShaderKey(
	MyEngine::Rendering::ShadingModel model,
	MyEngine::Rendering::InputLayoutType layout
) {
	// 上位32bitにShadingModel、下位32bitにInputLayoutTypeを詰める
	return (static_cast<uint64_t>(model) << 32) | static_cast<uint64_t>(layout);
}

void MyEngine::Rendering::Renderer::InitializeShaderTable() {
	// シェーダーテーブルを構築する
	auto* sm = shaderManager_.get();

	// Standard x Standard3D
	uint32_t stdVS = sm->CompileAndCacheShader(L"Resources/Shader/Object3d.VS.hlsl", L"vs_6_0");
	uint32_t stdPS = sm->CompileAndCacheShader(L"Resources/Shader/Object3d.PS.hlsl", L"ps_6_0");
	shaderTable_[MakeShaderKey(ShadingModel::Standard, InputLayoutType::Standard3D)] = { stdVS, stdPS };

	// Skybox x Skybox
	uint32_t skyVS = sm->CompileAndCacheShader(L"Resources/Shader/Skybox.VS.hlsl", L"vs_6_0");
	uint32_t skyPS = sm->CompileAndCacheShader(L"Resources/Shader/Skybox.PS.hlsl", L"ps_6_0");
	shaderTable_[MakeShaderKey(ShadingModel::Skybox, InputLayoutType::Skybox)] = { skyVS, skyPS };

	// WarterSurface x Standard3D
	uint32_t watVS = sm->CompileAndCacheShader(L"Resources/Shader/WaterSurface.VS.hlsl", L"vs_6_0");
	uint32_t watPS = sm->CompileAndCacheShader(L"Resources/Shader/WaterSurface.PS.hlsl", L"ps_6_0");
	shaderTable_[MakeShaderKey(ShadingModel::WaterSurface, InputLayoutType::Standard3D)] = { watVS, watPS };

	// BoatWake x BoatWake
	uint32_t wakeVS = sm->CompileAndCacheShader(L"Resources/Shader/BoatWake.VS.hlsl", L"vs_6_0");
	uint32_t wakePS = sm->CompileAndCacheShader(L"Resources/Shader/BoatWake.PS.hlsl", L"ps_6_0");
	shaderTable_[MakeShaderKey(ShadingModel::BoatWake, InputLayoutType::BoatWake)] = { wakeVS, wakePS };
}

void MyEngine::Rendering::Renderer::Submit(
	const MyEngine::Rendering::Mesh& mesh,
	MyEngine::Rendering::Material* material,
	D3D12_GPU_VIRTUAL_ADDRESS transformAddr,
	const char* debugName,
	D3D12_GPU_VIRTUAL_ADDRESS customAddr
) {
	if(!material) return;

	auto key = MakeShaderKey(material->GetShadingModel(), mesh.inputLayout);
	auto it = shaderTable_.find(key);
	assert(it != shaderTable_.end() && "シェーダーが未登録です");
	const ShaderPair& shaders = it->second;

	// PSODescriptorを組み立てる
	PSODescriptor desc{};
	desc.VS_ID = shaders.vs_ID;
	desc.PS_ID = shaders.ps_ID;
	desc.InputLayoutID = mesh.inputLayout;
	desc.BlendMode = material->GetBlendMode();
	desc.DepthEnable = material->IsDepthEnable();

	desc.DepthWriteMask = material->IsDepthWrite()
		? D3D12_DEPTH_WRITE_MASK_ALL
		: D3D12_DEPTH_WRITE_MASK_ZERO;

	desc.CullMode = material->IsDoubleSided()
		? D3D12_CULL_MODE_NONE
		: D3D12_CULL_MODE_BACK;

	// PSOManagerに渡してPSOを取得（なければ生成・キャッシュ）
	ID3D12PipelineState* pso = psoManager_->GetOrCreatePSO(
		device_,
		desc,
		rootSigManager_->GetCommonRootSignature(),
		*shaderManager_,
		*inputLayoutManager_,
		*blendModeManager_
	);

	// RenderCommandに詰めてキューに積む
	RenderCommand cmd{};
	cmd.pso = pso;
	cmd.vbView = mesh.vbView;
	cmd.ibv = mesh.ibView;
	cmd.vertexCount = mesh.vertexCount;
	cmd.indexCount = mesh.indexCount;
	cmd.transformGPUAddress = transformAddr;
	cmd.customBufferGPUAddress = customAddr;
	cmd.materialIndex = material->GetDescriptorIndex();
	cmd.textureIndex = material->GetTextureIndex();
	cmd.layer = material->GetLayer();
	cmd.debugName = debugName;
	renderSystem_->PushCommand(cmd);
}