#include "Sound.h"
#include "../../header/WinSupport.h"

Sound::Sound (IXAudio2* xAudio2, SoundData* data) {
	xAudio2_ = xAudio2;
	sourceVoice_ = std::make_unique<IXAudio2SourceVoice> ();
    data_ = std::make_unique<SoundData>(*data);

	IXAudio2SourceVoice* source = nullptr;
	HRESULT hr = xAudio2_->CreateSourceVoice (&source, &data_->wfex);
	if (SUCCEEDED (hr)) {
		sourceVoice_.reset (source);
	}
}

Sound::~Sound () {

}

void Sound::Play (bool loop) {
	XAUDIO2_BUFFER buf = data_->buffer;
	if (loop) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	else {
		buf.LoopCount = 0;
	}
	sourceVoice_->SubmitSourceBuffer (&buf);
	sourceVoice_->Start (0);
}

void Sound::Stop () {
	sourceVoice_->Stop (0);
	sourceVoice_->FlushSourceBuffers ();
}

void Sound::SetVolume (float vol) {
	sourceVoice_->SetVolume (vol);
}
