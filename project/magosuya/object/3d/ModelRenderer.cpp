//#include "ModelRenderer.h"
//#include <imgui.h>
//#include <algorithm>
//#include "mathFunction.h"
//#include "LightManager.h"
//#include "SRVManager.h"
//#include "Mesh.h"
//#include "TextureManager.h"
//
//ModelRenderer::ModelRenderer(DxCommon* dxCommon, LightManager* lightManager) {
//	dxCommon_ = dxCommon;
//	device_ = dxCommon->GetDevice();
//	commandList_ = dxCommon->GetCommandList();
//	lightManager_ = lightManager;
//	modelCount_++;
//	//その時のカウントをinstanceIDにコピー
//	instanceID_ = modelCount_;
//	for(int i = 0; i < 4; ++i) {
//		color_[i] = 1.0f;
//	}
//}
//
//ModelRenderer::~ModelRenderer() {
//}
//
//void ModelRenderer::Initialize() {
//	//===リソースの初期化===//
//	// 行列データ
//	matrixBuffer_ = dxCommon_->CreateBufferResource(sizeof(TransformationMatrix));
//	matrixBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&matrixData_));
//	matrixData_->World = Math::MakeIdentity4x4();
//	matrixData_->WVP = Math::MakeIdentity4x4();
//	matrixData_->WorldInverseTranspose = Math::MakeIdentity4x4();
//
//	// マテリアルデータ
//	materialBuffer_ = dxCommon_->CreateBufferResource(sizeof(Material));
//	materialBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
//	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
//	materialData_->enableLighting = LightReflectionModel::HalfLambert;
//	materialData_->uvTransform = Math::MakeIdentity4x4();
//	materialData_->roughness = 0.3f;
//	materialData_->metallic = 0.5f;
//	materialData_->environmentCoefficient = 0.3f;
//
//	cameraBuffer_ = dxCommon_->CreateBufferResource(sizeof(Vector3));
//	cameraBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&cameraData_));
//	cameraData_->x = 0.0f;
//	cameraData_->y = 0.0f;
//	cameraData_->z = 0.0f;
//
//	// 共有データをロックして有効性をチェック
//	std::shared_ptr<ModelData> data = modelData_.lock();
//	// モデルデータが解放済みなら止める
//	assert(data);
//
//	// skeletonの初期化
//	skeleton_ = CreateSkeleton(data->rootNode);
//	vp_ = Math::MakeIdentity4x4();
//	world_ = Math::MakeIdentity4x4();
//
//	// SkinCluster生成
//	skinCluster_ = CreateSkinCluster(data);
//
//	// PSO設定
//	desc_.RootSignatureID = RootSignatureManager::GetInstance()->GetOrCreateRootSignature(RootSigType::SkinningStandard3D);
//	desc_.VS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/SkinningObject3d.VS.hlsl", L"vs_6_0");
//	desc_.PS_ID = ShaderManager::GetInstance()->CompileAndCacheShader(L"Resources/shader/SkinningObject3d.PS.hlsl", L"ps_6_0");
//	desc_.InputLayoutID = InputLayoutType::SkinningStandard3D;
//	desc_.BlendMode = BlendModeType::Alpha;
//}
//
//void ModelRenderer::Update(Matrix4x4 world, Matrix4x4 vp, EulerTransform uvTransform, Vector3 cameraWorld) {
//	// 共有データをロックして有効性をチェック
//	std::shared_ptr<ModelData> data = modelData_.lock();
//	if(!data) {
//		// モデルデータが解放済みなら更新をスキップ
//		return;
//	}
//	
//	Matrix4x4 localMatrix = AnimationUpdate(data.get());
//	ApplyAnimation();
//	SkeletonUpdate(skeleton_);
//	SkinClusterUpdate();
//
//	// RootのMatrixを適用する
//	matrixData_->World = localMatrix * world;
//	matrixData_->WVP = matrixData_->World * vp;
//	matrixData_->WorldInverseTranspose = Math::Transpose(Math::Inverse(matrixData_->World));
//
//	// uvTransform更新
//	materialData_->uvTransform = Math::MakeAffineMatrix(uvTransform.scale, uvTransform.rotate, uvTransform.translate);
//
//	*cameraData_ = cameraWorld;
//	vp_ = vp;
//	world_ = localMatrix * world;
//}
//
//void ModelRenderer::Draw(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
//	// 共有データをロックして有効性をチェック
//	std::shared_ptr<ModelData> data = modelData_.lock();
//	if(!data) {
//		// モデルデータが解放済みなら更新をスキップ
//		return;
//	}
//
//	RootSignatureManager::GetInstance()->SetRootSignature(desc_.RootSignatureID);
//	PSOManager::GetInstance()->SetPSO(desc_);
//	// どんな形状で描画するのか
//	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	// 頂点バッファをセットする
//	// 2種類のVBVをまとめてセット
//	D3D12_VERTEX_BUFFER_VIEW vbvs[2] = {
//		data->vbView,
//		skinCluster_.influenceBufferView
//	};
//	commandList_->IASetVertexBuffers(0, 2, vbvs);
//	// インデックスバッファをセットする
//	commandList_->IASetIndexBuffer(&data->ibView);
//	// 定数バッファのルートパラメータを設定する	
//	commandList_->SetGraphicsRootConstantBufferView(0, matrixBuffer_->GetGPUVirtualAddress());
//	commandList_->SetGraphicsRootConstantBufferView(1, materialBuffer_->GetGPUVirtualAddress());
//	// カメラのPositionをセット
//	commandList_->SetGraphicsRootConstantBufferView(2, cameraBuffer_->GetGPUVirtualAddress());
//	// ライトの個数をセット
//	commandList_->SetGraphicsRootConstantBufferView(3, lightManager_->GetLightCountBuffer().GetGPUVirtualAddress());
//	// テクスチャのSRVを設定
//	commandList_->SetGraphicsRootDescriptorTable(4, textureHandle);
//	// directionalLightのSRVをセット
//	commandList_->SetGraphicsRootDescriptorTable(
//		5, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetDirLightSrvHandle())
//	);
//	// pointLightのSRVをセット
//	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(
//		6, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetPointLightSrvHandle())
//	);
//	// spotLightのSRVをセット
//	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(
//		7, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetSpotLightSrvHandle())
//	);
//	// rectLightのSRVをセット
//	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(
//		8, SRVManager::GetInstance()->GetGPUDescriptorHandle(lightManager_->GetRectLightSrvHandle())
//	);
//	// SkinningのSRVをセット
//	dxCommon_->GetCommandList()->SetGraphicsRootDescriptorTable(
//		9, SRVManager::GetInstance()->GetGPUDescriptorHandle(DescriptorFreeIndex_)
//	);
//	// 環境マップ用のTextureをセット
//	commandList_->SetGraphicsRootDescriptorTable(10, TextureManager::GetInstance()->GetTextureHandle("skybox"));
//	
//	// 実際に描画する
//	commandList_->DrawIndexedInstanced(static_cast<UINT>(data->indexCount), 1, 0, 0, 0);
//
//	if(drawSkeleton_) {
//		DrawSkeleton();
//	}
//}
//
//void ModelRenderer::ImGui(EulerTransform& transform, EulerTransform& uvTransform, const std::string& windowName) {
//#ifdef USEIMGUI
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
//	ImGui::DragFloat3(("uvScale" + label).c_str(), &uvTransform.scale.x, 0.01f);
//	ImGui::DragFloat3(("uvRotate" + label).c_str(), &uvTransform.rotate.x, 0.01f);
//	ImGui::DragFloat3(("uvTranslate" + label).c_str(), &uvTransform.translate.x, 0.01f);
//	ImGui::DragFloat(("roughness" + label).c_str(), &materialData_->roughness, 0.01f, 0.0f, 1.0f);
//	ImGui::DragFloat(("metallic" + label).c_str(), &materialData_->metallic, 0.01f, 0.0f, 1.0f);
//	ImGui::DragFloat(("environmentCoefficient" + label).c_str(), &materialData_->environmentCoefficient, 0.01f, 0.0f, 1.0f);
//	// ライトの種類を選べるようにする
//	int currentNum = static_cast<int>(materialData_->enableLighting);
//	const char* lights[] = { "None", "lambert", "halfLambert" };
//	if(ImGui::Combo(("ライティング" + label).c_str(), &currentNum, lights, IM_ARRAYSIZE(lights))) {
//		// 選ばれた番号をそのまま enum にキャストして戻せばOK！
//		materialData_->enableLighting = static_cast<LightReflectionModel>(currentNum);
//	}
//	ImGui::Checkbox(("drawSkeleton" + label).c_str(), &drawSkeleton_);
//	ImGui::Separator();
//#endif
//}
//
//void ModelRenderer::SkeletonInit() {
//	// 共有データをロックして有効性をチェック
//	std::shared_ptr<ModelData> data = modelData_.lock();
//	if(!data) {
//		// モデルデータが解放済みなら更新をスキップ
//		return;
//	}
//
//	CreateSkeleton(data->rootNode);
//}
//
//Matrix4x4 ModelRenderer::AnimationUpdate(ModelData* modelData) {
//	// 共有データをロックして有効性をチェック
//	std::shared_ptr<Animation> data = animationData_.lock();
//	if(!data) {
//		// モデルデータが解放済みなら描画をスキップ
//		return Math::MakeIdentity4x4();
//	}
//
//	// Animationの再生
//	animationTime_ += 1.0f / 60.0f;
//	animationTime_ = std::fmod(animationTime_, data->duration);	// 最後まで行ったらリピート
//
//	Vector3 translate = modelData->rootNode.transform.translate;
//	Quaternion rotate = modelData->rootNode.transform.rotate;
//	Vector3 scale = modelData->rootNode.transform.scale;
//
//	if(auto it = data->nodeAnimations.find(modelData->rootNode.name); it != data->nodeAnimations.end()) {
//		const NodeAnimation& rootNodeAnimation = (*it).second;
//		if(!rootNodeAnimation.translate.keyframes.empty()) {
//			translate = CalculateValue(rootNodeAnimation.translate.keyframes, animationTime_);
//		}
//		if(!rootNodeAnimation.rotate.keyframes.empty()) {
//			rotate = CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime_);
//		}
//		if(!rootNodeAnimation.scale.keyframes.empty()) {
//			scale = CalculateValue(rootNodeAnimation.scale.keyframes, animationTime_);
//		}
//	}
//
//	return Math::MakeAffineMatrix(scale, rotate, translate);
//}
//
//Vector3 ModelRenderer::CalculateValue(const std::vector<KeyframeVector3>& keyframes, float time) {
//	//***特殊なケースを除外する***//
//	// キーが無いものは返す値がわからないのでassert
//	assert(!keyframes.empty());
//	// キーが1つか、時刻がキーフレーム前なら最初の値とする
//	if(keyframes.size() == 1 || time <= keyframes[0].time) {
//		return keyframes[0].value;
//	}
//
//	//ここから実際に計算
//	for(size_t index = 0; index < keyframes.size() - 1; ++index) {
//		size_t newIndex = index + 1;
//		// indexとnewIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
//		if(keyframes[index].time <= time && time <= keyframes[newIndex].time) {
//			//範囲内を補間する
//			float t = (time - keyframes[index].time) / (keyframes[newIndex].time - keyframes[index].time);
//			return Math::Lerp(keyframes[index].value, keyframes[newIndex].value, t);
//		}
//	}
//	//	ここまで来た場合は一番後の時刻よりも後ろなので最後の値を返すことにする
//	return (*keyframes.rbegin()).value;
//}
//
//Quaternion ModelRenderer::CalculateValue(const std::vector<KeyframeQuaternion>& keyframes, float time) {
//	//***特殊なケースを除外する***//
//	// キーが無いものは返す値がわからないのでassert
//	assert(!keyframes.empty());
//	// キーが1つか、時刻がキーフレーム前なら最初の値とする
//	if(keyframes.size() == 1 || time <= keyframes[0].time) {
//		return keyframes[0].value;
//	}
//
//	//ここから実際に計算
//	for(size_t index = 0; index < keyframes.size() - 1; ++index) {
//		size_t newIndex = index + 1;
//		// indexとnewIndexの2つのkeyframeを取得して範囲内に時刻があるかを判定
//		if(keyframes[index].time <= time && time <= keyframes[newIndex].time) {
//			//範囲内を補間する
//			float t = (time - keyframes[index].time) / (keyframes[newIndex].time - keyframes[index].time);
//			return Math::Lerp(keyframes[index].value, keyframes[newIndex].value, t);
//		}
//	}
//	//	ここまで来た場合は一番後の時刻よりも後ろなので最後の値を返すことにする
//	return (*keyframes.rbegin()).value;
//}
//
//Skeleton ModelRenderer::CreateSkeleton(const Node& rootNode) {
//	Skeleton skeleton;
//	skeleton.root = CreateJoint(rootNode, {}, skeleton.joints);
//
//	// 名前とindexのマッピングを行いアクセスしやすくする
//	for(const Joint& joint : skeleton.joints) {
//		skeleton.jointMap.emplace(joint.name, joint.index);
//	}
//
//	//作った際に一回だけ呼ぶ
//	SkeletonUpdate(skeleton);
//
//	return skeleton;
//}
//
//int32_t ModelRenderer::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
//	Joint joint;
//	joint.name = node.name;
//	joint.localMatrix = node.localMatrix;
//	joint.skeletonSpaceMatrix = Math::MakeIdentity4x4();
//	joint.transform = node.transform;
//	joint.index = int32_t(joints.size());	// 現在登録されている数をIndexに
//	joint.parent = parent;
//	joints.push_back(joint);	// SkeletonのJoint列に追加
//	for(const Node& child : node.children) {
//		// 子Jointを作成し、そのIndexを登録
//		int32_t childIndex = CreateJoint(child, joint.index, joints);
//		joints[joint.index].children.push_back(childIndex);
//	}
//	// 自身のIndexを返す
//	return joint.index;
//}
//
//void ModelRenderer::SkeletonUpdate(Skeleton& skeleton) {
//	// すべてのJointを更新。親が若いので通常ループで処理可能
//	for(Joint& joint : skeleton.joints) {
//		joint.localMatrix = Math::MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
//		// そのJointに親がいれば親の行列をかけてあげる
//		if(joint.parent) {
//			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton.joints[*joint.parent].skeletonSpaceMatrix;
//		}
//		// 親がいなければlocalMatrixとskeletonSpaceMatrixは一致する
//		else {
//			joint.skeletonSpaceMatrix = joint.localMatrix;
//		}
//	}
//}
//
//void ModelRenderer::ApplyAnimation() {
//	// 共有データをロックして有効性をチェック
//	std::shared_ptr<Animation> data = animationData_.lock();
//	if(!data) {
//		// データが解放済みなら早期リターン
//		return;
//	}
//
//	for(Joint& joint : skeleton_.joints) {
//		// 対象のJointのAnimationがあれば値の適用を行う。下記のif文はC++17から可能になった初期化付きif文
//		if(auto it = data->nodeAnimations.find(joint.name); it != data->nodeAnimations.end()) {
//			const NodeAnimation& rootNodeAnimation = (*it).second;
//			if(!rootNodeAnimation.translate.keyframes.empty()) {
//				joint.transform.translate = CalculateValue(rootNodeAnimation.translate.keyframes, animationTime_);
//			}
//			if(!rootNodeAnimation.rotate.keyframes.empty()) {
//				joint.transform.rotate = CalculateValue(rootNodeAnimation.rotate.keyframes, animationTime_);
//			}
//			if(!rootNodeAnimation.scale.keyframes.empty()) {
//				joint.transform.scale = CalculateValue(rootNodeAnimation.scale.keyframes, animationTime_);
//			}
//		}
//	}
//}
//
//void ModelRenderer::DrawSkeleton() {
//	for(const Joint& joint : skeleton_.joints) {
//		if(joint.parent) { // 親ジョイントが存在するか確認
//			// 親ジョイントの参照
//			const Joint& parentJoint = skeleton_.joints[*joint.parent];
//			// それぞれの行列の平行移動成分（第4行目）からローカル座標系での位置ベクトルを抽出
//			Vector3 myLocalPos = { joint.skeletonSpaceMatrix.m[3][0], joint.skeletonSpaceMatrix.m[3][1], joint.skeletonSpaceMatrix.m[3][2] };
//			Vector3 parentLocalPos = { parentJoint.skeletonSpaceMatrix.m[3][0], parentJoint.skeletonSpaceMatrix.m[3][1], parentJoint.skeletonSpaceMatrix.m[3][2] };
//			// 計算用の関数を使って myLocalPos と parentLocalPos に `world_` を掛ける (localMatrixはJoint内に既に含まれているため二重適用を避ける)
//			// ワールド座標に変換してから、その2点に対してDrawLineする
//			Vector3 myWorldPos = Math::Transform(myLocalPos, world_);
//			Vector3 parentWorldPos = Math::Transform(parentLocalPos, world_);
//
//			Mesh::DrawLine(
//				parentWorldPos.x, parentWorldPos.y, parentWorldPos.z,
//				myWorldPos.x, myWorldPos.y, myWorldPos.z, {1.0f, 1.0f, 1.0f, 1.0f}, vp_);
//		}
//	}
//}
//
//SkinCluster ModelRenderer::CreateSkinCluster(std::shared_ptr<ModelData>& modelData) {
//	SkinCluster skinCluster;
//
//	// palette用のResourceを確保
//	skinCluster.paletteResource = dxCommon_->CreateBufferResource(sizeof(WellForGPU) * skeleton_.joints.size());
//	WellForGPU* mappedPalette = nullptr;
//	skinCluster.paletteResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedPalette));
//	skinCluster.mappedPalette = { mappedPalette, skeleton_.joints.size() };	// std::spanを使ってアクセスするようにする
//	// DescriptorHeapの空きIndexを確保(CPU,GPU)
//	DescriptorFreeIndex_ = SRVManager::GetInstance()->Allocate();
//	skinCluster.paletteSrvHandle.first = SRVManager::GetInstance()->GetCPUDescriptorHandle(DescriptorFreeIndex_);
//	skinCluster.paletteSrvHandle.second = SRVManager::GetInstance()->GetGPUDescriptorHandle(DescriptorFreeIndex_);
//
//	// palette用のsrvを作成
//	D3D12_SHADER_RESOURCE_VIEW_DESC paletteSrvDesc{};
//	paletteSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
//	paletteSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	paletteSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
//	paletteSrvDesc.Buffer.FirstElement = 0;
//	paletteSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
//	paletteSrvDesc.Buffer.NumElements = UINT(skeleton_.joints.size());
//	paletteSrvDesc.Buffer.StructureByteStride = sizeof(WellForGPU);
//	device_->CreateShaderResourceView(skinCluster.paletteResource.Get(), &paletteSrvDesc, skinCluster.paletteSrvHandle.first);
//
//	// influence用のResourceを確保
//	skinCluster.influenceResource = dxCommon_->CreateBufferResource(sizeof(VertexInfluence) * modelData->vertices.size());
//	VertexInfluence* mappedInfluence = nullptr;
//	skinCluster.influenceResource->Map(0, nullptr, reinterpret_cast<void**>(&mappedInfluence));
//	std::memset(mappedInfluence, 0, sizeof(VertexInfluence) * modelData->vertices.size());	// 0埋め。weightを0にしておく
//	skinCluster.mappedInfluence = { mappedInfluence, modelData->vertices.size() };
//
//	// influence用のVBVを作成
//	skinCluster.influenceBufferView.BufferLocation = skinCluster.influenceResource->GetGPUVirtualAddress();
//	skinCluster.influenceBufferView.SizeInBytes = UINT(sizeof(VertexInfluence) * modelData->vertices.size());
//	skinCluster.influenceBufferView.StrideInBytes = sizeof(VertexInfluence);
//
//	// InverseBindPoseMatrixの保存領域を作成
//	skinCluster.inverseBinePoseMatrices.resize(skeleton_.joints.size());
//	std::generate(skinCluster.inverseBinePoseMatrices.begin(), skinCluster.inverseBinePoseMatrices.end(), Math::MakeIdentity4x4);
//
//	// ModelDataのSkinCluster情報を解析してinfluenceの中身を埋める
//	for(const auto& jointWeight : modelData->skinClusterData) {	// ModelのSkinClusterの情報を解析
//		auto it = skeleton_.jointMap.find(jointWeight.first);	// jointWeight.firstはjoint名なので、skeletonに対象となるjointが含まれているか判断
//		if(it == skeleton_.jointMap.end()) {	// そんなな目のjointは存在しないので次に回す。
//			continue;
//		}
//		// (*it).secondにはjointのindexが入っているので、該当のindexのinverseBindPoseMatrixを代入
//		skinCluster.inverseBinePoseMatrices[(*it).second] = jointWeight.second.inverseBindPoseMatrix;
//		for(const auto& vertexWeight : jointWeight.second.vertexWeights) {
//			auto& currentInfluence = skinCluster.mappedInfluence[vertexWeight.vertexIndex];	// 該当のvertexIndexのinfluence情報を参照しておく
//			for(uint32_t index = 0; index < kNumMaxInfluence; ++index) {	// 空いているところに入れる
//				if(currentInfluence.weights[index] == 0.0f) {	// weight==0が空いている状態なので、その場所にweightとjointのindexを代入
//					currentInfluence.weights[index] = vertexWeight.weight;
//					currentInfluence.jointIndices[index] = (*it).second;
//					break;
//				}
//			}
//		}
//	}
//
//	// ボーンに紐づいていない頂点にデフォルトのウェイトを設定
//	for(size_t i = 0; i < modelData->vertices.size(); ++i) {
//		auto& influence = skinCluster.mappedInfluence[i];
//		float totalWeight = 0.0f;
//		for(uint32_t j = 0; j < kNumMaxInfluence; ++j) {
//			totalWeight += influence.weights[j];
//		}
//		if(totalWeight == 0.0f) {
//			// ルートボーン(index 0)に100%のウェイトを割り当て
//			influence.weights[0] = 1.0f;
//			influence.jointIndices[0] = 0;
//		}
//	}
//
//	return skinCluster;
//}
//
//void ModelRenderer::SkinClusterUpdate() {
//	for(size_t jointIndex = 0; jointIndex < skeleton_.joints.size(); ++jointIndex) {
//		assert(jointIndex < skinCluster_.inverseBinePoseMatrices.size());
//		skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix =
//			skinCluster_.inverseBinePoseMatrices[jointIndex] * skeleton_.joints[jointIndex].skeletonSpaceMatrix;
//		skinCluster_.mappedPalette[jointIndex].skeletonSpaceInverseTransposeMatrix =
//			Math::Transpose(Math::Inverse(skinCluster_.mappedPalette[jointIndex].skeletonSpaceMatrix));
//	}
//
//	//for(size_t i = 0; i < skeleton_.joints.size(); ++i) {
//	//	// すべてのボーンに「単位行列」を入れる
//	//	skinCluster_.mappedPalette[i].skeletonSpaceMatrix = Math::MakeIdentity4x4();
//	//	skinCluster_.mappedPalette[i].skeletonSpaceInverseTransposeMatrix = Math::MakeIdentity4x4();
//	//}
//}