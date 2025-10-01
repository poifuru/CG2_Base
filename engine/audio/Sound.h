#pragma once
#include "../../header/ComPtr.h"
#include "../../header/struct.h"
#include <xaudio2.h>
#pragma comment(lib, "xaudio2.h")


class Sound{
public: //メンバ関数
	Sound (IXAudio2* xAudio2, SoundData* data);
	~Sound ();

	void Play (bool loop = false);
	void Stop ();
	void SetVolume (float vol);

private://メンバ変数
	ComPtr<IXAudio2> xAudio2_ = nullptr;
	std::unique_ptr<IXAudio2SourceVoice> sourceVoice_;
	std::unique_ptr<SoundData> data_;
};