#pragma once  
#include "../../header/WinSupport.h"
#include <vector>  

class RawInput {  
public: // メンバ関数  
    void Initialize(HWND hwnd);
    void Update(LPARAM lParam);  
    bool IsKeyDown(unsigned short key) const;  
    long GetMouseDeltaX() const { return mouseDeltaX_; }  
    long GetMouseDeltaY() const { return mouseDeltaY_; }  

private: // メンバ変数  
    std::vector<bool> keyState_{ 256, false };
    long mouseDeltaX_ = 0;
    long mouseDeltaY_ = 0;
};
