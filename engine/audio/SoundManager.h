#pragma once
#include "../../header/ComPtr.h"
#include "../../header/struct.h"
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.h")
#include <unordered_map>
#include "Sound.h"


class SoundManager {
public:	//メンバ関数
	//シングルトンでひとつだけ存在させる
	static SoundManager* GetInstance () {
		static SoundManager instance;
		return &instance;
	}

	void Initialize ();
	void Finalize ();
	SoundData* Load (const std::string& name, const std::string& filepath);
	std::unique_ptr<Sound> CreateSound (const std::string& name);

private://メンバ変数
	SoundManager () = default;

	ComPtr<IXAudio2> xAudio2_ = nullptr;
	std::unique_ptr<IXAudio2MasteringVoice> masterVoice_;
	std::unordered_map<std::string, std::unique_ptr<SoundData>> sounds_;
};

