#include "SoundManager.h"

void SoundManager::Initialize () {
    HRESULT hr = XAudio2Create(&xAudio2_, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr)) throw std::runtime_error("XAudio2初期化失敗");

    IXAudio2MasteringVoice* tempMasterVoice = nullptr;
    hr = xAudio2_->CreateMasteringVoice(&tempMasterVoice);
    if (FAILED(hr)) throw std::runtime_error("MasteringVoice作成失敗");

    masterVoice_.reset(tempMasterVoice); // Use unique_ptr to manage the IXAudio2MasteringVoice
}

void SoundManager::Finalize () {
    // 読み込んだサウンドを全部解放
    for (auto& [name, sound] : sounds_) {
        sound->Unload ();
    }
    sounds_.clear ();

    if (masterVoice_) {
        masterVoice_->DestroyVoice ();
        masterVoice_=nullptr;
    }
    if (xAudio2_) {
        xAudio2_ = nullptr;
    }
}

SoundData* SoundManager::Load (const std::string& name, const std::string& filepath) {

	return nullptr;
}

std::unique_ptr<Sound> SoundManager::CreateSound (const std::string& name) {

	return std::unique_ptr<Sound> ();
}