#include "Animator.h"
#include "Model.h"
#include "DeltaTime.h"
#include "MathFunction.h"

Animator::Animator(DxCommon* dxCommon) : dxCommon_(dxCommon) {}
Animator::~Animator() {}

void Animator::Initialize(Model* targetModel) {
	targetModel_ = targetModel;
	isSkinning_ = true;

	// targetModel->GetModelData()->rootNode からスケルトン(Jointの階層)を生成する
	CreateSkeleton(targetModel->GetModelData()->rootNode);

	// 2. スキンクラスターの生成（これを呼び出すことで復活するぞ！）
	CreateSkinCluster(targetModel->GetModelData());
}

void Animator::BindAnimation(Animation* animation) {
	animation_ = animation;
	animation_->animationTime = 0.0f;
}

void Animator::Update() {
	if (!animation_) {
		rootAnimationMatrix_ = Math::MakeIdentity4x4();
		return;
	}

	// アニメーションの更新
	AnimationTimeUpdate();
	AnimationUpdate();
}

void Animator::AnimationTimeUpdate() {
	// 時間の更新は、Updateの中で「1フレームに1回だけ」行う
	if(animation_->isAnimation) {
		animation_->animationTime += kDeltaTime;
		animation_->animationTime = std::fmod(animation_->animationTime, animation_->duration);
	}
}

Skeleton Animator::CreateSkeleton(const Node& rootNode) {
	skeleton_.root = CreateJoint(rootNode, {}, skeleton_.joints);

	// 名前とindexのマッピングを行いアクセスしやすくする
	for(const Joint& joint : skeleton_.joints) {
		skeleton_.jointMap.emplace(joint.name, joint.index);
	}

	//作った際に一回だけ呼ぶ
	SkeletonUpdate();

	return skeleton_;
}

int32_t Animator::CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = Math::MakeIdentity4x4();
	joint.transform = node.transform;
	joint.index = int32_t(joints.size());	// 現在登録されている数をIndexに
	joint.parent = parent;
	joints.push_back(joint);	// SkeletonのJoint列に追加
	for(const Node& child : node.children) {
		// 子Jointを作成し、そのIndexを登録
		int32_t childIndex = CreateJoint(child, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}
	// 自身のIndexを返す
	return joint.index;
}

void Animator::AnimationUpdate() {
	// 2. スキニングの有無で分岐
	if (isSkinning_) {
		// スキニングありなら、全ジョイントを評価してスケルトンを更新
		EvaluateAnimation();
		SkeletonUpdate();

		// スキニング時は、ルートジョイントの skeletonSpaceMatrix をルート行列として扱ってもいいし、
		// パレットが骨の計算をすべて担うので、モデル自体の移動用には Identity を返してもOK（シェーダーの仕様による）
		if (!skeleton_.joints.empty()) {
			rootAnimationMatrix_ = Math::MakeIdentity4x4();
		}

		// GPUパレットへ転送
		skinCluster_.UpdatePalette(skeleton_, skinClusterData_.inverseBindMatrices);
	} 
	else {
		// ★スキニングなし（Modelクラスでやってたルートノード単体のアニメーション評価をここに完全再現！）
		ModelData* modelData = targetModel_->GetModelData();
		Vector3 translate = modelData->rootNode.transform.translate;
		Quaternion rotate = modelData->rootNode.transform.rotate;
		Vector3 scale = modelData->rootNode.transform.scale;

		auto it = animation_->nodeAnimations.find(modelData->rootNode.name);
		if (it != animation_->nodeAnimations.end()) {
			const NodeAnimation& rootNodeAnimation = (*it).second;
			if (!rootNodeAnimation.translate.keyframes.empty()) {
				translate = AnimationFunc::CalculateValue(rootNodeAnimation.translate.keyframes, animation_->animationTime);
			}
			if (!rootNodeAnimation.rotate.keyframes.empty()) {
				rotate = AnimationFunc::CalculateValue(rootNodeAnimation.rotate.keyframes, animation_->animationTime);
			}
			if (!rootNodeAnimation.scale.keyframes.empty()) {
				scale = AnimationFunc::CalculateValue(rootNodeAnimation.scale.keyframes, animation_->animationTime);
			}
		}
		// 計算結果を保持しておく
		rootAnimationMatrix_ = Math::MakeAffineMatrix(scale, rotate, translate);
	}
}

void Animator::EvaluateAnimation() {
	// ジョイントごとにアニメーションを評価
	for (Joint& joint : skeleton_.joints) {
		auto it = animation_->nodeAnimations.find(joint.name);
		if (it != animation_->nodeAnimations.end()) {
			const NodeAnimation& nodeAnim = (*it).second;
			if (!nodeAnim.translate.keyframes.empty()) {
				joint.transform.translate = AnimationFunc::CalculateValue(nodeAnim.translate.keyframes, animation_->animationTime);
			}
			if (!nodeAnim.rotate.keyframes.empty()) {
				joint.transform.rotate = AnimationFunc::CalculateValue(nodeAnim.rotate.keyframes, animation_->animationTime);
			}
			if (!nodeAnim.scale.keyframes.empty()) {
				joint.transform.scale = AnimationFunc::CalculateValue(nodeAnim.scale.keyframes, animation_->animationTime);
			}
		}
	}
}

void Animator::CreateSkinCluster(ModelData* modelData) {
	// モデルにスキンデータが入っていなければスキニングをオフにする
	if (modelData->skinClusterData.empty() || modelData->vertexCount == 0) {
		isSkinning_ = false;
		return;
	}

	// SkinClusterResourceのInitializeに渡すための、生データを一時的に作る
	SkinClusterData clusterData;

	// 頂点数分の影響度バッファをあらかじめゼロクリアで確保
	clusterData.vertexInfluences.resize(modelData->vertexCount);
	for (size_t i = 0; i < clusterData.vertexInfluences.size(); ++i) {
		for (uint32_t j = 0; j < kNumMaxInfluence; ++j) {
			clusterData.vertexInfluences[i].weights[j] = 0.0f;
			clusterData.vertexInfluences[i].jointIndices[j] = 0;
		}
	}

	// メンバ変数の inverseBindMatrices を一度綺麗にする
	skinClusterData_.inverseBindMatrices.clear();

	// パレットバッファのサイズは「スケルトンのジョイント数」と同じにする必要があるため、
	// あらかじめスケルトンのサイズ分、単位行列などで埋めておくのが一番安全
	skinClusterData_.inverseBindMatrices.resize(skeleton_.joints.size());
	for (size_t i = 0; i < skinClusterData_.inverseBindMatrices.size(); ++i) {
		skinClusterData_.inverseBindMatrices[i] = Math::MakeIdentity4x4();
	}

	// 各ジョイント（ボーン）のウェイト情報を解析して詰め替える
	// modelData->skinClusterData が JointWeightData の vector を持っている想定
	for (const auto& jointWeight : modelData->skinClusterData) {

		// スケルトンマップからジョイントのIndexを探す（ラムダ式は使わない！）
		auto it = skeleton_.jointMap.find(jointWeight.first); // jointWeight.firstがボーン名
		if (it == skeleton_.jointMap.end()) {
			continue; // スケルトンに存在しないボーンのウェイトは無視
		}
		int32_t jointIndex = (*it).second;

		// 該当するジョイントのインデックス位置に、正しい逆バインド行列を格納する
		skinClusterData_.inverseBindMatrices[jointIndex] = jointWeight.second.inverseBindPoseMatrix;

		// このボーンが影響を与える頂点リストをループ
		for (const auto& vertexWeight : jointWeight.second.vertexWeights) {
			uint32_t vIndex = vertexWeight.vertexIndex;

			// 念のため頂点インデックスが範囲内かチェックする防衛策
			if (vIndex >= clusterData.vertexInfluences.size()) {
				continue;
			}

			auto& influence = clusterData.vertexInfluences[vIndex];

			// 空いているスロット（weightが0の場所）を探して代入
			for (uint32_t s = 0; s < kNumMaxInfluence; ++s) {
				if (influence.weights[s] == 0.0f) {
					influence.weights[s] = vertexWeight.weight;
					influence.jointIndices[s] = jointIndex;
					break;
				}
			}
		}
	}

	// ボーンに紐づいていない迷子の頂点にデフォルトウェイトを設定する救済処置
	for (size_t i = 0; i < modelData->vertexCount; ++i) {
		auto& influence = clusterData.vertexInfluences[i];
		float totalWeight = 0.0f;
		for (uint32_t j = 0; j < kNumMaxInfluence; ++j) {
			totalWeight += influence.weights[j];
		}
		if (totalWeight == 0.0f) {
			influence.weights[0] = 1.0f;
			influence.jointIndices[0] = 0; // ルートボーンに縛り付ける
		}
	}

	// ★最終チェック：行列の数が0になっていないか、ここで確定させてからInitializeに挑む！
	assert(!skinClusterData_.inverseBindMatrices.empty() && "ボーンの逆バインド行列が0個です。初期化できません。");

	// データを詰め終わったら、満を持して SkinClusterResource を初期化！
	// inverseBindMatricesもclusterDataに入れて渡す仕様ならSkinCluster.hに合わせて調整してくれ
	clusterData.inverseBindMatrices = skinClusterData_.inverseBindMatrices;
	skinCluster_.Initialize(dxCommon_, clusterData, 0);
}

void Animator::SkeletonUpdate() {
	for (Joint& joint : skeleton_.joints) {
		joint.localMatrix = Math::MakeAffineMatrix(joint.transform.scale, joint.transform.rotate, joint.transform.translate);
		if (joint.parent) {
			joint.skeletonSpaceMatrix = joint.localMatrix * skeleton_.joints[*joint.parent].skeletonSpaceMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}