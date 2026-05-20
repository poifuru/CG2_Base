//#include "SphereMeshRenderer.h"
//#define _USE_MATH_DEFINES
//#include <math.h>
//#include <imgui.h>
//#include "mathFunction.h"
//
//SphereMeshRenderer::SphereMeshRenderer(DxCommon* dxCommon) {
//	dxCommon_ = dxCommon;
//	commandList_ = dxCommon->GetCommandList();
//	modelCount_++;
//	//その時のカウントをinstanceIDにコピー
//	instanceID_ = modelCount_;
//	for(int i = 0; i < 4; ++i) {
//		color_[i] = 1.0f;
//	}
//
//	kSubdivision_ = 16;
//}
//
//SphereMeshRenderer::~SphereMeshRenderer() {
//}
//
//void SphereMeshRenderer::Initialize(float radius) {
//	radius_ = radius;
//	const int vertexCount = kSubdivision_ * kSubdivision_ * 6;
//	vertexData_.resize(vertexCount); // ★ これ重要！
//
//	//それぞれのバッファを作成してMapする、頂点バッファ・インデックスバッファのビューも設定しておく
//	vertexBuffer_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * vertexCount);
//	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataPtr_));
//	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
//	vbView_.SizeInBytes = UINT(sizeof(VertexData) * vertexData_.size());
//	vbView_.StrideInBytes = sizeof(VertexData);
//
//	//お前は今じゃない、また今度な！
//	/*indexBuffer_ = CreateBufferResource (device, sizeof (uint32_t) * (kSubdivision_ * kSubdivision_) * 6);
//	indexBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&indexData_));
//	ibView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress ();
//	ibView_.SizeInBytes = UINT (sizeof (uint32_t) * (kSubdivision_ * kSubdivision_) * 6);
//	ibView_.Format = DXGI_FORMAT_R32_UINT;*/
//
//	//行列データ
//	matrixBuffer_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
//	matrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&matrixData_));
//	matrixData_->World = Math::MakeIdentity4x4();
//	matrixData_->WVP = Math::MakeIdentity4x4();
//	matrixData_->WorldInverseTranspose = Math::MakeIdentity4x4();
//
//	//マテリアルデータ
//	materialBuffer_ = dxCommon_->CreateBufferResource(sizeof(Material));
//	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
//	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
//	materialData_->enableLighting = LightReflectionModel::None;
//	materialData_->uvTransform = Math::MakeIdentity4x4();
//
//	//PSO設定
//	desc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Standard3D);
//	desc_.VS_ID = ShaderManager::GetInstance()->CompileAndCasheShader(L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0");
//	desc_.PS_ID = ShaderManager::GetInstance()->CompileAndCasheShader(L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0");
//	desc_.InputLayoutID = InputLayoutType::Standard3D;
//	desc_.BlendMode = BlendModeType::Alpha;
//
//
//	//球の形に頂点を配置
//	const float kLonEvery = float(M_PI) * 2.0f / float(kSubdivision_);
//	const float kLatEvery = float(M_PI) / float(kSubdivision_);
//
//	uint32_t index = 0;
//
//	for(int latIndex = 0; latIndex < kSubdivision_; ++latIndex) {
//		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex;
//		float latNext = lat + kLatEvery;
//
//		for(int lonIndex = 0; lonIndex < kSubdivision_; ++lonIndex) {
//			float lon = lonIndex * kLonEvery;
//			float lonNext = lon + kLonEvery;
//
//			Vector3 a = {
//				radius_* cosf(lat) * cosf(lon),
//				radius_* sinf(lat),
//				radius_* cosf(lat) * sinf(lon)
//			};
//			Vector3 b = {
//				radius_* cosf(latNext) * cosf(lon),
//				radius_* sinf(latNext),
//				radius_* cosf(latNext) * sinf(lon)
//			};
//			Vector3 c = {
//				radius_* cosf(lat) * cosf(lonNext),
//				radius_* sinf(lat),
//				radius_* cosf(lat) * sinf(lonNext)
//			};
//			Vector3 d = {
//				radius_* cosf(latNext) * cosf(lonNext),
//				radius_* sinf(latNext),
//				radius_* cosf(latNext) * sinf(lonNext)
//			};
//
//			//texcoordをしっかり書き込むための変数
//			float u = float(lonIndex) / float(kSubdivision_);
//			float uNext = float(lonIndex + 1) / float(kSubdivision_);
//			float v = 1.0f - float(latIndex) / float(kSubdivision_);
//			float vNext = 1.0f - float(latIndex + 1) / float(kSubdivision_);
//
//			// 三角形1: a, b, d
//			vertexData_[index].position = Vector4(a.x, a.y, a.z, 1.0f);
//			vertexData_[index].normal = Math::Normalize(a);
//			vertexData_[index].texcoord = { u, v };
//			//indexData_[index] = index;
//			index++;
//
//			vertexData_[index].position = Vector4(b.x, b.y, b.z, 1.0f);
//			vertexData_[index].normal = Math::Normalize(b);
//			vertexData_[index].texcoord = { u, vNext };
//			//indexData_[index] = index;
//			index++;
//
//			vertexData_[index].position = Vector4(d.x, d.y, d.z, 1.0f);
//			vertexData_[index].normal = Math::Normalize(d);
//			vertexData_[index].texcoord = { uNext, vNext };
//			//indexData_[index] = index;
//			index++;
//
//			// 三角形2: a, d, c
//			vertexData_[index].position = Vector4(a.x, a.y, a.z, 1.0f);
//			vertexData_[index].normal = Math::Normalize(a);
//			vertexData_[index].texcoord = { u, v };
//			//indexData_[index] = index;
//			index++;
//
//			vertexData_[index].position = Vector4(d.x, d.y, d.z, 1.0f);
//			vertexData_[index].normal = Math::Normalize(d);
//			vertexData_[index].texcoord = { uNext, vNext };
//			//indexData_[index] = index;
//			index++;
//
//			vertexData_[index].position = Vector4(c.x, c.y, c.z, 1.0f);
//			vertexData_[index].normal = Math::Normalize(c);
//			vertexData_[index].texcoord = { uNext, v };
//			//indexData_[index] = index;
//			index++;
//		}
//	}
//
//	//GPUに渡すために手動でコピーする
//	memcpy(vertexDataPtr_, vertexData_.data(), sizeof(VertexData) * vertexCount);
//}
//
//void SphereMeshRenderer::Update(Matrix4x4 world, Matrix4x4 vp, Transform uvTransform) {
//	matrixData_->World = world;
//	matrixData_->WVP = Math::Multiply(matrixData_->World, vp);
//	matrixData_->WorldInverseTranspose = Math::Transpose(Math::Inverse(matrixData_->World));
//
//	//uvTranform更新
//	materialData_->uvTransform = Math::MakeAffineMatrix(uvTransform.scale, uvTransform.rotate, uvTransform.translate);
//}
//
//void SphereMeshRenderer::Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, ID3D12Resource* light) {
//	RootSignatureManager::GetInstance()->SetRootSignature(desc_.RootSignatureID);
//	PSOManager::GetInstance()->SetPSO(desc_);
//	//どんな形状で描画するのか
//	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	//頂点バッファをセットする
//	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vbView_);	//VBVを設定
//	//定数バッファのルートパラメータを設定する	
//	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, matrixBuffer_->GetGPUVirtualAddress());
//	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, materialBuffer_->GetGPUVirtualAddress());
//	//テクスチャのSRVを設定
//	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureHandle);
//	//ライトをセット
//	commandList_->SetGraphicsRootConstantBufferView(3, light->GetGPUVirtualAddress());
//	//実際に描画する(後々Index描画に変える)
//	dxCommon_->GetCommandList()->DrawInstanced((kSubdivision_ * kSubdivision_ * 6), 1, 0, 0);
//}
//
//void SphereMeshRenderer::ImGui(Transform& transform, Transform& uvTransform, const std::string& windowName) {
//	std::string num = std::to_string(instanceID_);
//	std::string label = "##" + tag_ + num;
//	ImGui::Text(("obj : " + windowName).c_str());
//	if(ImGui::ColorEdit4(("Color" + label).c_str(), color_)) {
//		// 色が変更されたらmaterialDataに反映
//		materialData_->color.x = color_[0];
//		materialData_->color.y = color_[1];
//		materialData_->color.z = color_[2];
//		materialData_->color.w = color_[3];
//	}
//	ImGui::DragFloat3(("scale" + label).c_str(), &transform.scale.x, 0.01f);
//	ImGui::DragFloat3(("rotate" + label).c_str(), &transform.rotate.x, 0.01f);
//	ImGui::DragFloat3(("translate" + label).c_str(), &transform.translate.x, 0.01f);
//	ImGui::DragFloat3(("UVscale" + label).c_str(), &uvTransform.scale.x, 0.01f);
//	ImGui::DragFloat3(("UVrotate" + label).c_str(), &uvTransform.rotate.x, 0.01f);
//	ImGui::DragFloat3(("UVtranslate" + label).c_str(), &uvTransform.translate.x, 0.01f);
//	//ライトの種類を選べるようにする
//	int currentNum = 0;
//	const char* lights[] = { "None", "lambert", "halfLambert" };
//	if(ImGui::Combo(("ライティング" + label).c_str(), &currentNum, lights, IM_ARRAYSIZE(lights))) {
//		if(currentNum == 0) {
//			materialData_->enableLighting = LightReflectionModel::None;
//			currentNum = 0;
//		}
//		else if(currentNum == 1) {
//			materialData_->enableLighting = LightReflectionModel::Lambert;
//			currentNum = 1;
//		}
//		else if(currentNum == 2) {
//			materialData_->enableLighting = LightReflectionModel::HalfLambert;
//			currentNum = 2;
//		}
//		else if(currentNum == 3) {
//			materialData_->enableLighting = LightReflectionModel::Specular;
//			currentNum = 3;
//		}
//	}
//	ImGui::Separator();
//}