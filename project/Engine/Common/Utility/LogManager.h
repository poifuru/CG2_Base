#pragma once
#include <fstream>

// ログ出力を一括で管理するクラス
class LogManager {
public:	// 外部公開関数
	/// <summary>
	/// 静的なインスタンスを返す関数
	/// </summary>
	/// <returns>インスタンス</returns>
	static LogManager* GetInstance() {
		//　初めて呼び出されたときに一回だけ初期化
		static LogManager instance;
		return &instance;
	}

	/// <summary>
	/// 初期化関数
	/// </summary>
	void Initialize() {
		os_ = Logtext();
	}
	
	/// <summary>
	/// ログを出力する関数
	/// </summary>
	/// <param name="message"></param>
	void Log(const std::string& message);

private: // 内部関数
	/// <summary>
	/// デストラクタ
	/// </summary>
	~LogManager();

	/// <summary>
	/// ログをテキストで出す関数
	/// </summary>
	/// <returns>os_</returns>
	std::ofstream Logtext();

private: // シングルトンにするためにコンストラクタを禁止
	// コンストラクタを禁止
	LogManager() = default;
	// コピーコンストラクタと代入演算子を禁止
	LogManager(const LogManager&) = delete;
	LogManager& operator=(const LogManager&) = delete;
	LogManager(LogManager&&) = delete;
	LogManager& operator=(LogManager&&) = delete;

private: // メンバ変数
	// ログ書き込み用の変数
	std::ofstream os_;
};