#include "SphereMesh.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <imgui.h>
#include "function.h"
#include "MathFunction.h"

SphereMesh::SphereMesh(DxCommon* dxCommon) {
	dxCommon_ = dxCommon;

	//球の分割数を決める
	kSubdivision_ = 16;

	const int vertexCount = kSubdivision_ * kSubdivision_ * 6;
	vertexData_.resize(vertexCount); // ★ これ重要！

	//それぞれのバッファを作成してMapする、頂点バッファ・インデックスバッファのビューも設定しておく
	vertexBuffer_ = dxCommon_->CreateBufferResource(sizeof(VertexData) * vertexCount);
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataPtr_));
	vbView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vbView_.SizeInBytes = UINT(sizeof(VertexData) * vertexData_.size());
	vbView_.StrideInBytes = sizeof(VertexData);

	//お前は今じゃない、また今度な！
	/*indexBuffer_ = CreateBufferResource (device, sizeof (uint32_t) * (kSubdivision_ * kSubdivision_) * 6);
	indexBuffer_->Map (0, nullptr, reinterpret_cast<void**>(&indexData_));
	ibView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress ();
	ibView_.SizeInBytes = UINT (sizeof (uint32_t) * (kSubdivision_ * kSubdivision_) * 6);
	ibView_.Format = DXGI_FORMAT_R32_UINT;*/

	matrixBuffer_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
	matrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&matrixData_));
	matrixData_->World = Math::MakeIdentity4x4();
	matrixData_->WVP = Math::MakeIdentity4x4();
	matrixData_->WorldInverseTranspose = Math::MakeIdentity4x4();

	materialBuffer_ = dxCommon_->CreateBufferResource(sizeof(Material));
	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = LightReflectionModel::None;
	materialData_->uvTransform = Math::MakeIdentity4x4();
	materialData_->shininess = 50.0f;
	materialData_->isSpecular = false;

	cameraBuffer_ = dxCommon_->CreateBufferResource(sizeof(Vector3));
	cameraBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));

	transform_ = {};
	uvTransform_ = {};
	transformationMatrix_.World = Math::MakeIdentity4x4();
	transformationMatrix_.WVP = Math::MakeIdentity4x4();
	radius_ = 1.0f;

	for(int i = 0; i < 4; i++) {
		color_[i] = 1.0f;
	}
}

SphereMesh::~SphereMesh() {
	if(vertexBuffer_) {
		vertexBuffer_->Unmap(0, nullptr);
		vertexBuffer_.Reset();
	}
	if(matrixBuffer_) {
		matrixBuffer_->Unmap(0, nullptr);
		matrixBuffer_.Reset();
	}
	if(materialBuffer_) {
		materialBuffer_->Unmap(0, nullptr);
		materialBuffer_.Reset();
	}
}

void SphereMesh::Initialize(Vector3 position, float radius) {
	transform_ = {
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		position
	};
	uvTransform_ = {
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f }
	};
	radius_ = radius;

	const float kLonEvery = float(M_PI) * 2.0f / float(kSubdivision_);
	const float kLatEvery = float(M_PI) / float(kSubdivision_);

	const int vertexCount = kSubdivision_ * kSubdivision_ * 6;
	vertexData_.resize(vertexCount); // ★ これ重要！

	uint32_t index = 0;

	for(int latIndex = 0; latIndex < kSubdivision_; ++latIndex) {
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex;
		float latNext = lat + kLatEvery;

		for(int lonIndex = 0; lonIndex < kSubdivision_; ++lonIndex) {
			float lon = lonIndex * kLonEvery;
			float lonNext = lon + kLonEvery;

			Vector3 a = {
				radius * cosf(lat) * cosf(lon),
				radius * sinf(lat),
				radius * cosf(lat) * sinf(lon)
			};
			Vector3 b = {
				radius * cosf(latNext) * cosf(lon),
				radius * sinf(latNext),
				radius * cosf(latNext) * sinf(lon)
			};
			Vector3 c = {
				radius * cosf(lat) * cosf(lonNext),
				radius * sinf(lat),
				radius * cosf(lat) * sinf(lonNext)
			};
			Vector3 d = {
				radius * cosf(latNext) * cosf(lonNext),
				radius * sinf(latNext),
				radius * cosf(latNext) * sinf(lonNext)
			};

			//texcoordをしっかり書き込むための変数
			float u = float(lonIndex) / float(kSubdivision_);
			float uNext = float(lonIndex + 1) / float(kSubdivision_);
			float v = 1.0f - float(latIndex) / float(kSubdivision_);
			float vNext = 1.0f - float(latIndex + 1) / float(kSubdivision_);

			// 三角形1: a, b, d
			vertexData_[index].position = Vector4(a.x, a.y, a.z, 1.0f);
			vertexData_[index].normal = Math::Normalize(a);
			vertexData_[index].texcoord = { u, v };
			//indexData_[index] = index;
			index++;

			vertexData_[index].position = Vector4(b.x, b.y, b.z, 1.0f);
			vertexData_[index].normal = Math::Normalize(b);
			vertexData_[index].texcoord = { u, vNext };
			//indexData_[index] = index;
			index++;

			vertexData_[index].position = Vector4(d.x, d.y, d.z, 1.0f);
			vertexData_[index].normal = Math::Normalize(d);
			vertexData_[index].texcoord = { uNext, vNext };
			//indexData_[index] = index;
			index++;

			// 三角形2: a, d, c
			vertexData_[index].position = Vector4(a.x, a.y, a.z, 1.0f);
			vertexData_[index].normal = Math::Normalize(a);
			vertexData_[index].texcoord = { u, v };
			//indexData_[index] = index;
			index++;

			vertexData_[index].position = Vector4(d.x, d.y, d.z, 1.0f);
			vertexData_[index].normal = Math::Normalize(d);
			vertexData_[index].texcoord = { uNext, vNext };
			//indexData_[index] = index;
			index++;

			vertexData_[index].position = Vector4(c.x, c.y, c.z, 1.0f);
			vertexData_[index].normal = Math::Normalize(c);
			vertexData_[index].texcoord = { uNext, v };
			//indexData_[index] = index;
			index++;
		}
	}

	/*for (int i = 0; i < 6; ++i) {
		char buffer[128];
		sprintf_s (buffer, "uv[%d] = (%f, %f)\n", i, vertexData_[i].texcoord.x, vertexData_[i].texcoord.y);
		OutputDebugStringA (buffer);
	}*/
	//GPUに渡すために手動でコピーする
	memcpy(vertexDataPtr_, vertexData_.data(), sizeof(VertexData) * vertexCount);

	//PSO設定
	desc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::Standard3D);
	desc_.VS_ID = ShaderManager::GetInstance()->CompileAndCasheShader(L"Resources/shader/Object3d.VS.hlsl", L"vs_6_0");
	desc_.PS_ID = ShaderManager::GetInstance()->CompileAndCasheShader(L"Resources/shader/Object3d.PS.hlsl", L"ps_6_0");
	desc_.InputLayoutID = InputLayoutType::Standard3D;
	desc_.BlendMode = BlendModeType::Opaque;
}

void SphereMesh::Update(Vector3 cameraWorld, Matrix4x4* vp) {
	Matrix4x4 world = Math::MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	matrixData_->World = world;
	matrixData_->WVP = Math::Multiply(matrixData_->World, *vp);
	matrixData_->WorldInverseTranspose = Math::Transpose(Math::Inverse(matrixData_->World));

	//uvTranform更新
	materialData_->uvTransform = Math::MakeAffineMatrix(uvTransform_.scale, uvTransform_.rotate, uvTransform_.translate);

	*cameraData_ = cameraWorld;
}

void SphereMesh::Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle, ID3D12Resource* light) {
	RootSignatureManager::GetInstance()->SetRootSignature(desc_.RootSignatureID);
	PSOManager::GetInstance()->SetPSO(desc_);
	//どんな形状で描画するのか
	dxCommon_->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//頂点バッファをセットする
	dxCommon_->GetCommandList()->IASetVertexBuffers(0, 1, &vbView_);	//VBVを設定
	//定数バッファのルートパラメータを設定する	
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(0, matrixBuffer_->GetGPUVirtualAddress());
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(1, materialBuffer_->GetGPUVirtualAddress());
	//テクスチャのSRVを設定
	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureHandle);
	//ライトをセット
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(3, light->GetGPUVirtualAddress());
	//カメラのPositionをセット
	dxCommon_->GetCommandList()->SetGraphicsRootConstantBufferView(4, cameraBuffer_->GetGPUVirtualAddress());
	//実際に描画する(後々Index描画に変える)
	dxCommon_->GetCommandList()->DrawInstanced((kSubdivision_ * kSubdivision_ * 6), 1, 0, 0);
}

void SphereMesh::ImGui() {
	if(ImGui::ColorEdit4("Color##sphere", color_)) {
		// 色が変更されたらmaterialDataに反映
		materialData_->color.x = color_[0];
		materialData_->color.y = color_[1];
		materialData_->color.z = color_[2];
		materialData_->color.w = color_[3];
	}
	ImGui::DragFloat3("scale##sphere", &transform_.scale.x, 0.01f);
	ImGui::DragFloat3("rotate##sphere", &transform_.rotate.x, 0.01f);
	ImGui::DragFloat3("translate##sphere", &transform_.translate.x, 0.01f);
	ImGui::DragFloat3("UVscale##sphere", &uvTransform_.scale.x, 0.01f);
	ImGui::DragFloat3("UVrotate##sphere", &uvTransform_.rotate.x, 0.01f);
	ImGui::DragFloat3("UVtranslate##sphere", &uvTransform_.translate.x, 0.01f);
	ImGui::DragFloat("shininess##sphere", &materialData_->shininess, 0.1f, 0.0f, 1000.0f);
	//ライトの種類を選べるようにする
	int currentNum = 0;
	const char* lights[] = { "None", "lambert", "halfLambert" };
	if(ImGui::Combo("ライティング", &currentNum, lights, IM_ARRAYSIZE(lights))) {
		if(currentNum == 0) {
			materialData_->enableLighting = LightReflectionModel::None;
			currentNum = 0;
		}
		else if(currentNum == 1) {
			materialData_->enableLighting = LightReflectionModel::Lambert;
			currentNum = 1;
		}
		else if(currentNum == 2) {
			materialData_->enableLighting = LightReflectionModel::HalfLambert;
			currentNum = 2;
		}
	}
	ImGui::Checkbox("IsSpecular", &materialData_->isSpecular);
	ImGui::Separator();
}