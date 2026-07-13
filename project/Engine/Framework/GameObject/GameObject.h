#pragma once
using json = nlohmann::json;

class Component;
struct SceneContext;

namespace MyEngine::Rendering {
	class Renderer;
}

class GameObject {
public:
	GameObject(SceneContext* context, const std::string& name = "GameObject");
	~GameObject();

	void Initialize();
	void Update();
	void Draw(MyEngine::Rendering::Renderer* renderer);
	void ImGui();

	// シリアライズ用
	json Serialize() const;
	void Deserialize(const json& j);

	// アクセッサ
	const std::string& GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }

	EulerTransform& GetTransform() { return transform_; }
	const EulerTransform& GetTransform() const { return transform_; }

	SceneContext* GetContext() const { return context_; }

	// 全てのコンポーネントにデバッグモードを伝える
	void SetIsDebugMode(bool flag);

	bool IsDead() { return isDead_; }
	void Destroy() { isDead_ = true; } // 死ぬときに呼ぶ

	void SetSerializable(bool flag) { isSerializable_ = flag; }
	bool IsSerializable() const { return isSerializable_; }

	// --- コンポーネント操作のテンプレート関数 --- //
	// コンポーネントの追加
	template <typename T, typename... Args>
	T* AddComponent(Args&&... args) {
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		component->SetGameObject(this);
		T* ptr = component.get();
		components_.push_back(std::move(component));
		return ptr;
	}

	// コンポーネントの取得
	template <typename T>
	T* GetComponent() {
		for (auto& component : components_) {
			T* ptr = dynamic_cast<T*>(component.get());
			if (ptr != nullptr) {
				return ptr;
			}
		}
		return nullptr;
	}

	// コンポーネントの削除
	template <typename T>
	bool RemoveComponent() {
		for (auto it = components_.begin(); it != components_.end(); ++it) {
			if (dynamic_cast<T*>(it->get()) != nullptr) {
				components_.erase(it);
				return true;
			}
		}
		return false;
	}

private:
	std::string name_;
	EulerTransform transform_;
	std::vector<std::unique_ptr<Component>> components_;
	bool isDead_ = false;
	bool isSerializable_ = true;	// シリアライズするかのフラグ

	SceneContext* context_ = nullptr;
};