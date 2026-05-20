#pragma once
#include <memory>
#include <string>

class Model;
class DxCommon;
class LightManager;

class ModelFactory {
public:
	static ModelFactory* GetInstance() {
		//初めて呼び出されたときに一回だけ初期化
		static ModelFactory instance;
		return &instance;
	}

	void Initialize(DxCommon* dxCommon);

	// アセットが完全に紐づいたModelを出力
	std::unique_ptr<Model> CreateModel(
		const std::string& modelName,
		const std::string& textureName
	);

	void SetLightManager(LightManager* lightManager) { lightManager_ = lightManager; }

public:
	//コンストラクタを禁止
	ModelFactory() = default;
	// コピーコンストラクタと代入演算子を禁止
	ModelFactory(const ModelFactory&) = delete;
	ModelFactory& operator=(const ModelFactory&) = delete;
	ModelFactory(ModelFactory&&) = delete;
	ModelFactory& operator=(ModelFactory&&) = delete;

private:
	DxCommon* dxCommon_ = nullptr;
	LightManager* lightManager_ = nullptr;
};