#pragma once  
#include "../../header/WinSupport.h"
#include <vector>  

class RawInput {  
public: // メンバ関数  
    void Initialize(HWND hwnd);
    void Update(LPARAM lParam);  
    bool Push (unsigned short key) const;
    bool Trigger (unsigned short key)const;
    bool Release (unsigned short key)const;
    void EndFrame ();
    long GetMouseDeltaX() const { return mouseDeltaX_; }  
    long GetMouseDeltaY() const { return mouseDeltaY_; }  

private: // メンバ変数  
    std::vector<bool> keys_ = std::vector<bool> (256, false);
    std::vector<bool> preKeys_ = std::vector<bool> (256, false);
    long mouseDeltaX_ = 0;  
    long mouseDeltaY_ = 0;  
};
