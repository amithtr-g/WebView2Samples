// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include "time.h"
#include <wrl.h>
#include <wil/com.h>
// include WebView2 header
#include "WebView2.h"

using namespace Microsoft::WRL;

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");
HBRUSH hbrBlack, hbrGray;
HDC hdc;

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("WebView sample");

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Pointer to WebViewController
static wil::com_ptr<ICoreWebView2Controller> webviewController;

// Pointer to WebView window
static wil::com_ptr<ICoreWebView2> webviewWindow;

bool purchaseCompleted = false;

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	// Store instance handle in our global variable
	hInst = hInstance;

	// The parameters to CreateWindow explained:
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application does not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1200, 900,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd,
		nCmdShow);
	UpdateWindow(hWnd);


	// <-- WebView2 sample code starts here -->
	// Step 3 - Create a single WebView within the parent window
	// Locate the browser and set up the environment for WebView
	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

				// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
				env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller != nullptr) {
							webviewController = controller;
							webviewController->get_CoreWebView2(&webviewWindow);
						}
						
						wil::com_ptr<ICoreWebView2_2> m_webview2;
						ICoreWebView2CookieManager* m_cookieManager;
						webviewWindow->QueryInterface(IID_PPV_ARGS(&m_webview2));
						m_webview2->get_CookieManager(&m_cookieManager);


						std::wstring uri;

						// print the previous cookie
						if (m_cookieManager)
						{
							m_cookieManager->GetCookies(
								uri.c_str(),
								Callback<ICoreWebView2GetCookiesCompletedHandler>(
									[hWnd, uri](HRESULT error_code, ICoreWebView2CookieList* list) -> HRESULT {
								// CHECK_FAILURE(error_code);

								std::wstring result;
								UINT cookie_list_size;
								list->get_Count(&cookie_list_size);

								if (cookie_list_size == 0)
								{
									result += L"No cookies found.";
								}
								else
								{
									result += std::to_wstring(cookie_list_size) + L" cookie(s) found";
									if (!uri.empty())
									{
										result += L" on " + uri;
									}
									result += L"\n\n[";
									for (UINT i = 0; i < cookie_list_size; ++i)
									{
										wil::com_ptr<ICoreWebView2Cookie> cookie;
										list->GetValueAtIndex(i, &cookie);

										if (cookie.get())
										{
											// LPWSTR temp;
											// cookie.get()->get_Name(temp);
											
											// -- coookie print -- //
											wil::unique_cotaskmem_string name;
											cookie.get()->get_Name(&name);
											wil::unique_cotaskmem_string value;
											cookie.get()->get_Value(&value);


											std::wstring cky = L"{";
											//cky += L"\"Name\": " +  std::wstring(name.get()) + L", " + L"\"Value\": " +
												// std::wstring(value.get());

											cky += L"\"Name\": " + std::wstring(name.get()) + L", ";
											cky += L"\"}";

											// -- end cookie print -- //

											result += cky;

											if (i != cookie_list_size - 1)
											{
												result += L",\n";
											}
										}
									}
									result += L"]";
								}
								MessageBox(nullptr, result.c_str(), L"GetCookies Result", MB_OK);
								return S_OK;
							})
								.Get());
						}
						

						// set the cookie
						srand(time(NULL));
						int r = rand() % 26;
						char c = 'a' + r;

						wchar_t buf[100];
						swprintf(buf, 100, L"%s-%c", L"CookieAmith", c);

						wil::com_ptr<ICoreWebView2Cookie> cookie;
						m_cookieManager->CreateCookie(
							buf, L"CookieValueAmith", L".google.com", L"/", &cookie);
						m_cookieManager->AddOrUpdateCookie(cookie.get());



						// Add a few settings for the webview
						// The demo step is redundant since the values are the default settings
						ICoreWebView2Settings* Settings;
						webviewWindow->get_Settings(&Settings);
						Settings->put_IsScriptEnabled(TRUE);
						Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
						Settings->put_IsWebMessageEnabled(TRUE);
						// " Not A;Brand";v="99", "Chromium";v="99", "Google Chrome";v="99"
						// Settings->put_UserAgent('" Not A;Brand"; v = "99", "Chromium"; v = "99", "Google Chrome"; v = "99"');
						// " Not A;Brand";v="99", "Chromium";v="99", "Microsoft Edge";v="99"

						
						ICoreWebView2Settings2* Settings2;
						auto hr = Settings->QueryInterface(IID_PPV_ARGS(&Settings2));
						if (SUCCEEDED(hr)) {
							// Settings2->put_UserAgent(L"\" Not A; Brand\"; v = \"99\", \"Chromium\"; v = \"99\", \"Google Chrome\"; v = \"99\"");
							Settings2->put_UserAgent(L"\" Not A; Brand\";v=\"99\", \"Chromium\";v=\"99\", \"Microsoft Edge\";v=\"99\"");
						}
						


						// Resize WebView to fit the bounds of the parent window
						RECT bounds;
						GetClientRect(hWnd, &bounds);
						
						bounds.left += 100;
						bounds.right -= 100;
						bounds.top += 100;
						bounds.bottom -= 100;

						webviewController->put_Bounds(bounds);

						// Schedule an async task to navigate to Bing
						// webviewWindow->Navigate(L"https://www.bing.com/");
						webviewWindow->Navigate(L"https://play.google.com/games/profile");
						// webviewWindow->Navigate(L"https://play.google.com");

						// webviewWindow->Navigate(L"https://accounts.google.com");


						// Step 4 - Navigation events
						// register an ICoreWebView2NavigationStartingEventHandler to cancel any non-https navigation
						EventRegistrationToken token;
						webviewWindow->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
							[](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
								PWSTR uri;
								args->get_Uri(&uri);
								std::wstring source(uri);
								if (source.find(L"https://play.google.com/web/ncsoft") == 0) {
									purchaseCompleted = true;
								}
								if (source.find(L"https://play.google.com/") == 0) {
									purchaseCompleted = true;
								}
								CoTaskMemFree(uri);
								return S_OK;
							}).Get(), &token);

						webviewWindow->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
							[](ICoreWebView2* webview, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
								if (purchaseCompleted) {
									BOOL isSuccess;
									args->get_IsSuccess(&isSuccess);
									webviewWindow->ExecuteScript(
										//L"while(true) { if (window.document.documentElement.innerText.indexOf(\'successful\') > -1) {window.chrome.webview.postMessage(\'success\'); break; } }",
										//L"window.document.documentElement.addEventListener(\'click\', function() {window.chrome.webview.postMessage(\'success\');} );",
										//L"myTimeout(); function myTimeout() {console.log(\'loop\'); setTimeout(myFunc, 2000);} function myFunc() {if (window.document.documentElement.innerText.indexOf(\'Diamonds\') > -1) {window.chrome.webview.postMessage(\'success\');} else { myTimeout(); }}",
										L"setTimeout(myFunc, 300000); function myFunc() { window.chrome.webview.postMessage(\'success\');}",
										Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
											[](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
												return S_OK;
											}).Get());
								}
								return S_OK;
							}).Get(), &token);

						// Step 5 - Scripting
						// Schedule an async task to add initialization script that freezes the Object object
						webviewWindow->AddScriptToExecuteOnDocumentCreated(L"Object.freeze(Object);", nullptr);
						// Schedule an async task to get the document URL
						webviewWindow->ExecuteScript(L"window.document.URL;", Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
							[](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
								LPCWSTR URL = resultObjectAsJson;
								//doSomethingWithURL(URL);
								return S_OK;
							}).Get());

						// Step 6 - Communication between host and web content
						// Set an event handler for the host to return received message back to the web content
						webviewWindow->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
							[](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
								PWSTR message;
								args->TryGetWebMessageAsString(&message);
								// processMessage(&message);
								webview->PostWebMessageAsString(message);
								std::wstring result(message);
								if (result.find(L"success") != -1) {
									webviewController->Close();
								}
								CoTaskMemFree(message);
								return S_OK;
							}).Get(), &token);

						// Schedule an async task to add initialization script that
						// 1) Add an listener to print message from the host
						// 2) Post document URL to the host
						webviewWindow->AddScriptToExecuteOnDocumentCreated(
							L"window.chrome.webview.addEventListener(\'message\', event => alert(event.data));" \
							L"window.chrome.webview.postMessage(window.document.URL);",
							nullptr);

						return S_OK;
					}).Get());
				return S_OK;
			}).Get());


	
	// <-- WebView2 sample code ends here -->

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR greeting[] = _T("Hello, Windows desktop!");

	switch (message)
	{
	case WM_CREATE:
		hbrBlack = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
		hbrGray = static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH));
		return 0L;

	case WM_ERASEBKGND:
		hdc = (HDC)wParam;
		RECT rc;
		GetClientRect(hWnd, &rc);
		SetMapMode(hdc, MM_ANISOTROPIC);
		SetWindowExtEx(hdc, 100, 100, NULL);
		SetViewportExtEx(hdc, rc.right, rc.bottom, NULL);
		FillRect(hdc, &rc, hbrBlack);

		for (int i = 0; i < 13; i++)
		{
			int x = (i * 40) % 100;
			int y = ((i * 40) / 100) * 20;
			SetRect(&rc, x, y, x + 20, y + 20);
			FillRect(hdc, &rc, hbrGray);
		}
		return 1L;
	case WM_SIZE:
		if (webviewController != nullptr) {
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			bounds.left += 100;
			bounds.right -= 100;
			bounds.top += 100;
			bounds.bottom -= 100;
			webviewController->put_Bounds(bounds);
			// calling close will close the webview
			// webviewController->Close();
		};
		
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}
