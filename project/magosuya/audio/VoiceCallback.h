#pragma once
#include <xaudio2.h>
#include <stdint.h>

class VoiceCallback : public IXAudio2VoiceCallback {
public:
	void STDMETHODCALLTYPE OnBufferEnd(void* pBufferContext) override {
		// 中身を見ようとせず、ポインタの値をそのまま数値に戻す
		uint32_t id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pBufferContext));
	}

	// 残りの純粋仮想関数は空の定義が必要
	void STDMETHODCALLTYPE OnVoiceProcessingPassStart(UINT32) override {}
	void STDMETHODCALLTYPE OnVoiceProcessingPassEnd() override {}
	void STDMETHODCALLTYPE OnStreamEnd() override {}
	void STDMETHODCALLTYPE OnBufferStart(void*) override {}
	void STDMETHODCALLTYPE OnLoopEnd(void*) override {}
	void STDMETHODCALLTYPE OnVoiceError(void*, HRESULT) override {}
};