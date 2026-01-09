#pragma once
#include "struct.h"
#include "ParticleRenderer.h"

struct ParticleCPUData {
	Transform transform;
	Vector3 velocity;
	Vector3 accerelation;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

class BaseParticle {
public:
	//コンストラクタ、デストラクタ
	BaseParticle() = default;
	virtual ~BaseParticle() = 0;

	/// <summary>
	/// 初期化処理
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="deltaTime">デルタタイム</param>
	virtual void Update(float deltaTime) = 0;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// 寿命が尽きているか
	/// </summary>
	/// <returns>存在時間が寿命よりも長いか</returns>
	virtual bool IsDead() const { return data_.currentTime >= data_.lifeTime; }

	/// <summary>
	/// ParticleDataをセット(Emitter用)
	/// </summary>
	/// <param name="data">ParticleData</param>
	void SetData(const ParticleCPUData data) { data_ = data; }

	/// <summary>
	/// ParticleDataを取得
	/// </summary>
	/// <returns>ParticleData</returns>
	const ParticleCPUData& GetData() const { return data_; }

	/// <summary>
	/// 寿命がどれだけ進んでいるか
	/// </summary>
	/// <returns>残り寿命の割合</returns>
	float GetLifeRatio() const { return (data_.lifeTime > 0.0f) ? (data_.currentTime / data_.lifeTime) : 1.0f; }
		
protected:
	//パーティクルの挙動計算用のデータ
	ParticleCPUData data_ = {};

	//パーティクル描画クラス
	ParticleRenderer* renderer_ = nullptr;
};