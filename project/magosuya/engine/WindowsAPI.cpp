#include "WindowsAPI.h"
#pragma comment (lib, "winmm.lib")
#include "imgui_impl_win32.h"
#include "function.h"
#include "InputManager.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

void WindowsAPI::Initialize(InputManager* inputManager) {
	//システムタイマーの分解能を上げる
	timeBeginPeriod(1);

	//ウィンドウプロシージャ
	windowClass_.lpfnWndProc = WindowProc;
	//ウィンドウクラス名
	windowClass_.lpszClassName = L"LE2B_22_マスヤ_ゴウ";
	//インスタンスハンドル
	windowClass_.hInstance = GetModuleHandle(nullptr);
	//カーソル
	windowClass_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスを登録する
	RegisterClass(&windowClass_);

	//ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, kClientWidth, kClientHeight };

	//クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウィンドウを生成
	hwnd_ = CreateWindow(
		windowClass_.lpszClassName,	//利用するクラス名
		L"LE2B_24_マスヤ_ゴウ_ピコピコ",	//タイトルバーの文字
		WS_OVERLAPPEDWINDOW,		//よく見るウィンドウスタイル
		CW_USEDEFAULT,				//表示x座標(Windowsに任せる)
		CW_USEDEFAULT,				//表示y座標(WindowsOSに任せる)
		wrc.right - wrc.left,		//ウィンドウ横幅
		wrc.bottom - wrc.top,		//ウィンドウ縦幅
		nullptr,					//親ウィンドウハンドル
		nullptr,					//メニューハンドル
		windowClass_.hInstance,		//インスタンスハンドル
		nullptr						//オプション
	);

	// ウィンドウに this ポインタを保存
	SetWindowLongPtr(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	//ウィンドウを表示
	ShowWindow(hwnd_, SW_SHOW);

	//インプットマネージャーのポインタ
	inputManager_ = inputManager;
}

LRESULT WindowsAPI::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	//メッセージに応じてゲーム固有の処理を行う
	switch(msg) {
		//入力を検知した
	case WM_INPUT:
	{
		WindowsAPI* pThis = reinterpret_cast<WindowsAPI*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if(pThis && pThis->inputManager_) {
			pThis->inputManager_->Update(lparam);
		}
		break;
	}

	// Alt + Enter によるフルスクリーン切り替え
	case WM_SYSKEYDOWN:
		if (wparam == VK_RETURN && (lparam & (1 << 29))) { // Alt + Enter
			WindowsAPI* pThis = reinterpret_cast<WindowsAPI*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			if (pThis) {
				pThis->SetFullscreen(!pThis->IsFullscreen());
			}
			return 0;
		}
		break;

	//ウィンドウが破棄された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	if(ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

bool WindowsAPI::ProcessMessage() {
	MSG msg{};

	//Windowにメッセージが来てたら最優先で処理させる
	while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		//終了するかの判断をここでしてboolで返す
		if(msg.message == WM_QUIT) {
			return true;
		}
	}

	return false;
}

void WindowsAPI::Finalize() {
	DestroyWindow(hwnd_);
}

std::string FileUtils::OpenFileDialog() {
	char filePath[MAX_PATH] = "";

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = WindowsAPI::GetInstance()->WindowsAPI::GetHwnd(); // 必要ならゲームのウィンドウハンドル(HWND)を渡してね
	ofn.lpstrFilter = "PNG Files (*.png)\0*.png\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Select Particle Texture";
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; // カレントディレクトリを勝手に変えないフラグ

	if (GetOpenFileNameA(&ofn)) {
		return std::string(filePath); // 選択された絶対パスを返す
	}
	return ""; // キャンセルされたら空文字
}

std::string FileUtils::GetRelativePath(const std::string& absolutePath) {
	if (absolutePath.empty()) return "";

	// "Resources" という文字列がどこにあるか探す
	size_t pos = absolutePath.find("Resources");
	if (pos != std::string::npos) {
		// "Resources" 以降の文字列を切り出す (例: "Resources/Particle/circle2.png")
		return absolutePath.substr(pos);
	}

	// 見つからなければそのまま返す
	return absolutePath;
}

void WindowsAPI::SetFullscreen(bool fullscreen) {
	if (isFullscreen_ == fullscreen) return;

	DWORD style = GetWindowLong(hwnd_, GWL_STYLE);

	if (fullscreen) {
		// ウィンドウモードの位置を記憶
		GetWindowPlacement(hwnd_, &windowPlacement_);

		// スタイルをポップアップ（枠なし）に変更
		style &= ~WS_OVERLAPPEDWINDOW;
		style |= WS_POPUP;
		SetWindowLong(hwnd_, GWL_STYLE, style);

		// モニターの情報を取得
		HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
		MONITORINFO monitorInfo = { sizeof(monitorInfo) };
		GetMonitorInfo(monitor, &monitorInfo);

		// モニター全体にリサイズ
		SetWindowPos(hwnd_, HWND_TOP,
			monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.top,
			monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
			monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
			SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	} else {
		// 元のスタイルに戻す
		style &= ~WS_POPUP;
		style |= WS_OVERLAPPEDWINDOW;
		SetWindowLong(hwnd_, GWL_STYLE, style);

		// 元の位置に戻す
		SetWindowPlacement(hwnd_, &windowPlacement_);
		SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
	}

	isFullscreen_ = fullscreen;
}
