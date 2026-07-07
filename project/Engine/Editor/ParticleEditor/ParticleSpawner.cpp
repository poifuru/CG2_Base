#include "PCH.h"
#include "ParticleSpawner.h"
#include "GameObject.h"
#include "MeshRendererComponent.h"
#include "ParticleComponent.h"
#include "BaseScene.h"
#include <cstdlib>
#include <cmath>

void ParticleSpawner::SpawnExplosion(SceneContext* context, const Vector3& position, int count) {
	if (!context || !context->gameObjects) return;

	for (int i = 0; i < count; ++i) {
		auto partObj = std::make_unique<GameObject>(context, "Particle");
		partObj->SetSerializable(false); // セーブ対象から除外

		auto* mesh = partObj->AddComponent<MeshRendererComponent>();
		auto* partComp = partObj->AddComponent<ParticleComponent>();

		partObj->GetTransform().translate = position;
		// スケールを大きくする（0.3f 〜 0.7f）
		float scaleSize = 0.3f + static_cast<float>(rand()) / RAND_MAX * 0.4f;
		partObj->GetTransform().scale = { scaleSize, scaleSize, scaleSize };

		// 先に初期化する（これでデフォルト値が一度セットされる）
		partObj->Initialize();

		// 初期化された後に、モデルや色、ライティング設定を上書きする！（リセット防止）
		mesh->SetModel("Resources/plane/plane.obj");
		mesh->SetTexture("Resources/ParticleAssets/circle2.png");
		mesh->SetEnableLighting(false);
		mesh->SetBlendMode(BlendModeType::Additive);
		mesh->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });

		float theta = static_cast<float>(rand()) / RAND_MAX * 3.14159265f * 2.0f;
		float phi = static_cast<float>(rand()) / RAND_MAX * 3.14159265f;
		float speed = 2.0f + static_cast<float>(rand()) / RAND_MAX * 6.0f;

		Vector3 velocity = {
			speed * std::sin(phi) * std::cos(theta),
			speed * std::cos(phi),
			speed * std::sin(phi) * std::sin(theta)
		};

		partComp->SetVelocity(velocity);
		partComp->SetGravity(9.8f);
		partComp->SetLifeTime(0.3f + static_cast<float>(rand()) / RAND_MAX * 0.4f);
		partComp->SetColorFade({ 1.0f, 0.6f, 0.1f, 1.0f }, { 1.0f, 0.0f, 0.0f, 0.0f });

		// 生成された最初のフレームの描画前に確実にGPUにデータを送るため、手動でUpdateを走らせる！
		partObj->Update();

		context->gameObjects->push_back(std::move(partObj));
	}
}

void ParticleSpawner::SpawnTrail(SceneContext* context, const Vector3& position, const Vector3& direction) {
	if (!context || !context->gameObjects) return;

	auto partObj = std::make_unique<GameObject>(context, "Particle");
	partObj->SetSerializable(false);

	auto* mesh = partObj->AddComponent<MeshRendererComponent>();
	auto* partComp = partObj->AddComponent<ParticleComponent>();

	partObj->GetTransform().translate = position;
	// スケールを大きくする（0.3f）
	partObj->GetTransform().scale = { 0.3f, 0.3f, 0.3f };

	// 先に初期化する
	partObj->Initialize();

	// 初期化された後に、モデルや色、ライティング設定を上書きする！（リセット防止）
	mesh->SetModel("Resources/plane/plane.obj");
	mesh->SetTexture("Resources/ParticleAssets/circle2.png"); // テスト用の circle2.png のままにしています
	mesh->SetEnableLighting(false);
	mesh->SetBlendMode(BlendModeType::Alpha);
	mesh->SetColor({ 1.0f, 1.0f, 1.0f, 0.5f });

	float randX = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.5f;
	float randY = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.5f;
	float randZ = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 1.5f;
	Vector3 velocity = {
		-direction.x * 2.0f + randX,
		-direction.y * 2.0f + randY,
		-direction.z * 2.0f + randZ
	};

	partComp->SetVelocity(velocity);
	partComp->SetGravity(0.0f);
	partComp->SetLifeTime(0.3f);
	partComp->SetColorFade({ 1.0f, 1.0f, 1.0f, 0.5f }, { 1.0f, 1.0f, 1.0f, 0.0f });

	// 生成された最初のフレームの描画前に確実にGPUにデータを送るため、手動でUpdateを走らせる！
	partObj->Update();

	context->gameObjects->push_back(std::move(partObj));
}
