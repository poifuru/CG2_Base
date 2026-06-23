#pragma once 
#include <Windows.h>
#include <vector>  

// マウスボタンのインデックスを分かりやすくするために定義しとく
enum MouseButton {
    LEFT = 0,
    RIGHT = 1,
    MIDDLE = 2,
};

class RawInput {  
public:
	RawInput() = default;
	~RawInput() = default;

    void Initialize(HWND hwnd);

    void HandleInputMessage (LPARAM lParam);

    //キーボードの入力関数
    bool Push (unsigned short key) const;
    bool Trigger (unsigned short key)const;
    bool Release (unsigned short key)const;

    //マウスの入力関数
	//0:左, 1:右, 2:中
    bool PushMouse (int button) const;
	//0:左, 1:右, 2:中
    bool TriggerMouse (int button) const;
	//0:左, 1:右, 2:中
    bool ReleaseMouse (int button) const;

    //デバッグ用
    void HandleMessageForDebug (LPARAM lParam);

    void EndFrame ();
    long GetMouseDeltaX() const { return mouseDeltaX_; }  
    long GetMouseDeltaY() const { return mouseDeltaY_; }

private:
	void ParseInputData(const RAWINPUT* raw);

private: // メンバ変数  
    //キーボード
    std::vector<bool> keys_ = std::vector<bool> (256, false);
    std::vector<bool> preKeys_ = std::vector<bool> (256, false);

    //マウス(左、右、ホイール対応で3ボタン)
    std::vector<bool> mouseButtons_ = std::vector<bool> (3, false);
    std::vector<bool> preMouseButtons_ = std::vector<bool> (3, false);

    long mouseDeltaX_ = 0;  
    long mouseDeltaY_ = 0;  

    //バッファ
    std::vector<BYTE> buffer_;
};
