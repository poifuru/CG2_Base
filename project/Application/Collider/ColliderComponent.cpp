#include "PCH.h"
#include "ColliderComponent.h"
#include "GameObject.h"
#include "CollisionManager.h"
#include "BirdEnemyComponent.h"
#include "FishEnemyComponent.h"

// デフォルトは半径1の球として CollisionObject を初期化
ColliderComponent::ColliderComponent() 
	: CollisionObject(Sphere{{0.0f, 0.0f, 0.0f}, 1.0f}) {}

ColliderComponent::~ColliderComponent() {
	CollisionManager::GetInstance()->UnregisterObject(this);
}
void ColliderComponent::Initialize() {

	// ロード済みならリセット処理をスキップして、マネージャー登録だけ行う
	if (isInitialized_) {
		CollisionManager::GetInstance()->RegisterObject(this);
		return;
	}
	isInitialized_ = true;

	radius_ = 1.0f;

	// コライダーマネージャーに自身を登録！
	CollisionManager::GetInstance()->RegisterObject(this);
}

void ColliderComponent::Update() {
	if (!gameObject_) return;

	// 親GameObjectの座標に合わせて、コライダーの球の中心座標を更新する
	Vector3 myPos = gameObject_->GetTransform().translate;

	// CollisionObject内部の幾何データを更新（強引にキャストして書き換える）
	Sphere& sphere = const_cast<Sphere&>(GetSphere());
	sphere.center = myPos;
	sphere.radius = radius_;
}

void ColliderComponent::ImGui() {
	// エディタ上でコライダーの半径をドラッグ調整できるようにする
	ImGui::DragFloat("Radius", &radius_, 0.1f, 0.01f, 50.0f);
}

void ColliderComponent::Serialize(json& j) const {
	j["type"] = "ColliderComponent";
	j["radius"] = radius_;
}

void ColliderComponent::Deserialize(const json& j) {
	isInitialized_ = true; // ロードしたので初期化済みフラグを立てる！
	if (j.contains("radius")) radius_ = j["radius"];
}

void ColliderComponent::OnCollision(CollisionObject* other) {
	// 衝突相手のコライダーコンポーネントを取得
	auto* otherCollider = dynamic_cast<ColliderComponent*>(other);
	if (!otherCollider) return;

	GameObject* otherObj = otherCollider->GetGameObject();
	GameObject* myObj = GetGameObject();
	if (!otherObj || !myObj) return;

	// 自分が「敵（鳥か魚のコンポーネントを持っているか、または名前がEnemy）」で、相手が「弾」なら消滅
	bool isMyEnemy = (myObj->GetComponent<BirdEnemyComponent>() != nullptr || 
					  myObj->GetComponent<FishEnemyComponent>() != nullptr ||
					  myObj->GetName() == "Enemy");

	// ★【修正】作成した isMyEnemy 変数を使って判定するように変更！
	if (isMyEnemy && otherObj->GetName() == "PlayerBullet") {
		myObj->Destroy();
		otherObj->Destroy();
	}
}