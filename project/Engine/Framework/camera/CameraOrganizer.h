#pragma once

// 前方宣言
struct CameraData;
class MainCameraComponent;
class VirtualCameraComponent;

class CameraOrganizer {
public:
	static CameraOrganizer* GetInstance () {
		//初めて呼び出されたときに一回だけ初期化
		static CameraOrganizer instance;
		return &instance;
	}
	~CameraOrganizer ();

	//初期化関数
	void Initialize ();

	//アクティブカメラの更新処理
	void Update ();

	// メインカメラの登録
	void RegisterMainCamera(MainCameraComponent* mainCamera) { mainCamera_ = mainCamera; }

	// 仮想カメラの登録
	void RegisterVirtualCamera(VirtualCameraComponent* virtualCamera);

	// 仮想カメラの登録解除
	void UnregisterVirtualCamera(VirtualCameraComponent* virtualCamera);

	// 外部から描画情報をもらうためのインターフェース
	CameraData& GetCameraData();
	float GetActiveFov() const { return currentFov_; }

	// カメラシェイクのリクエスト
	void Shake(float duration = 0.35f, float intensity = 0.4f);

private:
	//コンストラクタを禁止
	CameraOrganizer () = default;
	// コピーコンストラクタと代入演算子を禁止
	CameraOrganizer (const CameraOrganizer&) = delete;
	CameraOrganizer& operator=(const CameraOrganizer&) = delete;
	CameraOrganizer (CameraOrganizer&&) = delete;
	CameraOrganizer& operator=(CameraOrganizer&&) = delete;

private:
	// 最優先の仮想カメラを決定する
	VirtualCameraComponent* FindActiveVirtualCamera();

private:
	MainCameraComponent* mainCamera_ = nullptr;			// メインカメラ
	std::vector<VirtualCameraComponent*> virtualCameras_;	// 仮想カメラのコンテナ

	VirtualCameraComponent* currentVirtualCamera_ = nullptr;	// 現在の仮想カメラ
	VirtualCameraComponent* preVirtualCamera_ = nullptr;	// ひとつ前の仮想カメラ

	// ブレンド（補間）用の変数
	float blendTimer_ = 0.0f;
	float blendDuration_ = 1.5f; // 切り替えかける時間
	bool isBlending_ = false;
	float currentFov_ = 0.45f;

	// カメラシェイク用の変数
	float shakeTimer_ = 0.0f;
	float shakeDuration_ = 0.0f;
	float shakeIntensity_ = 0.0f;
};