#pragma once
#include "header.h"

class Sound{
public:	//メンバ変数
	ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_;
};