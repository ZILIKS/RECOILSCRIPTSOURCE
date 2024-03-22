#include "gui.h"
#include <iostream>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include <string>
#include <random>
#include "../imgui/simpleSerialCom/Arduino.h"
#include <Windows.h>
#include <chrono>
#include <thread>
#include <cmath>



	Arduino arduino("Arduino Leonardo");





extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); // set click points
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

	return DefWindowProc(window, message, wideParameter, longParameter);
}


void SendDataToArduino(Arduino& arduino, std::atomic_bool& stopThread, const std::atomic_bool& recoilEnabled, const std::atomic_bool& secondaryEnabled, const std::atomic_bool& rapidFire, const std::atomic<int>& sliderValueVertical, const std::atomic<int>& sliderValueHorizontal, const std::atomic<int>& sliderAmountDelay, const std::atomic<int>& sliderValueVerticalSecondary, const std::atomic<int>& sliderValueHorizontalSecondary, const std::atomic<int>& sliderAmountDelaySecondary) {
	while (!stopThread) {
		// Your existing serial data sending logic here
		// Use the thread-safe atomic variables instead of the regular variables

		// ... (your existing logic)
	}
}
void gui::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);
	

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);



}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

//claude ai hot key not on focus
static bool recoilEnabled = false;
static bool secondaryEnabled = false;
static bool rapidFire = false;
//long __stdcall WindowProcess(
//	HWND window,
//	UINT message,
//	WPARAM wideParameter,
//	LPARAM longParameter)
//{

	


void gui::Render() noexcept
{



	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });


	// Apply a dark theme

	// Set window background color and rounding
	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(60, 67, 70, 200));
	ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(60, 67, 70, 200));


	ImGui::Begin(
		"ZILUXE ARDUINO SUIT",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);

	//rounding 
	ImGuiStyle& style = ImGui::GetStyle();

	style.WindowRounding = 2.0f;
	style.FrameRounding = 4.0f;
	style.PopupRounding = 2.0f;
	style.GrabRounding = 3.0f;
	style.TabRounding = 4.0f;
	//all color elements purple and grey
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_FrameBg] = ImVec4(0.56f, 0.58f, 0.60f, 0.30f);
	//colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.12f, 0.65f, 0.19f);

	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.27f, 0.27f, 0.27f, 0.40f);


	colors[ImGuiCol_FrameBgActive] = ImVec4(0.42f, 0.13f, 0.65f, 0.19f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.44f, 0.22f, 0.61f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.32f, 0.25f, 0.64f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.21f, 1.00f, 0.00f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.47f, 0.04f, 0.78f, 1.00f);

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	
	static bool recoilEnabled = false;
	static bool isF1Pressed = false;


	ImGui::Checkbox("Recoil", &recoilEnabled);
#define VK_F1 0x70


	if (GetAsyncKeyState(VK_F1) & 0x8000)
	{
		// If the key was not pressed before
		if (!isF1Pressed)
		{
			recoilEnabled = !recoilEnabled; // Toggle the state of recoilEnabled
			isF1Pressed = true; // Mark the key as pressed
		}
	}
	else
	{
		isF1Pressed = false; // Mark the key as released
	}

	ImGui::SameLine();

	

	static bool secondaryEnabled = false;
	ImGui::Checkbox("Use Secondary", &secondaryEnabled);

#define VK_KEY_1 0x31 // VK code for '1' key
#define VK_KEY_2 0x32 // VK code for '2' key


if (GetAsyncKeyState(VK_KEY_1) & 0x8000)
{
    secondaryEnabled = false;
}

if (GetAsyncKeyState(VK_KEY_2) & 0x8000)
{
	secondaryEnabled = true;
}

		
	
	ImGui::SameLine();

	static bool rapidFire = false;
	ImGui::Checkbox("Rapid Fire", &rapidFire);



	
	
	// Slider values
// Slider values for primary weapon
	static int sliderValueVertical = 1;
	static int sliderValueHorizontal= 0;
	static int sliderAmountDelay = 0;

	// Slider values for secondary weapon
	static int sliderValueVerticalSecondary = 1;
	static int sliderValueHorizontalSecondary = 0;
	static int sliderAmountDelaySecondary = 0;



	std::atomic_bool stopThread = false;
	std::atomic_bool recoilEnabled = false;
	std::atomic_bool secondaryEnabled = false;
	std::atomic_bool rapidFire = false;
	std::atomic<int> sliderValueVertical = 1;
	std::atomic<int> sliderValueHorizontal = 0;
	std::atomic<int> sliderAmountDelay = 0;
	std::atomic<int> sliderValueVerticalSecondary = 1;
	std::atomic<int> sliderValueHorizontalSecondary = 0;
	std::atomic<int> sliderAmountDelaySecondary = 0;





	if (secondaryEnabled) {
		ImGui::SliderInt("Secondary Vertical", &sliderValueVerticalSecondary, 0, 25);
		ImGui::SliderInt("Secondary Horizontal", &sliderValueHorizontalSecondary, -10, 10);
		ImGui::SliderInt("Secondary Delay", &sliderAmountDelaySecondary, 0, 20);
	}
	else {
		ImGui::SliderInt("Primary Vertical", &sliderValueVertical, 0, 25);
		ImGui::SliderInt("Primary Horizont", &sliderValueHorizontal, -10, 10);
		ImGui::SliderInt("Primary Delay", &sliderAmountDelay, 0, 20);
	}



	//ImGui::ShowDemoWindow();
	//ImGui::ShowStyleEditor();
	// Inside your ImGui render loop:


	// Other ImGui widgets and logic...

	// Dropdown box
	const char* primary[] = { "None","Warden", "G-36", "AR", "Lesion" };
	static int primary_current = 0; // Change the default value to 0
	ImGui::Combo("Weapon/Operator", &primary_current, primary, IM_ARRAYSIZE(primary));


	const char* secondary[] = { "None","smg12", "smg11", "br9", "mp9" };
	static int secondary_current = 0; // Change the default value to 0
	ImGui::Combo("Secondary", &secondary_current, secondary, IM_ARRAYSIZE(secondary));

	switch (primary_current) {

	case 1: // warden
		sliderValueVertical = 6;
		sliderValueHorizontal = 0;
		sliderAmountDelay = 4;
		break;
	case 2: // g-36
		sliderValueVertical = 8;
		sliderValueHorizontal = 1;
		sliderAmountDelay = 8;
		break;
	case 3: // AR
		sliderValueVertical = 7;
		sliderValueHorizontal = 0;
		sliderAmountDelay = 9;
		break;
	case 4: // AR
		sliderValueVertical = 6;
		sliderValueHorizontal = 0;
		sliderAmountDelay = 8;
		break;
		// Add cases for other primary options if needed
	}

	switch (secondary_current) {

	case 1: // smg12
		sliderValueVerticalSecondary = 6;
		sliderValueHorizontalSecondary = 1;
		sliderAmountDelaySecondary = 1;
		break;
	case 2: // smg11
		sliderValueVerticalSecondary = 5;
		sliderValueHorizontalSecondary = 0;
		sliderAmountDelaySecondary = 2;
		break;
	case 3: // br9
		sliderValueVerticalSecondary = 5;
		sliderValueHorizontalSecondary = 0;
		sliderAmountDelaySecondary = 1;
		break;
		// Add cases for other primary options if needed
	}

	static float hue = 1.0f; // Hue value for the rainbow color
	static const float hueIncrement = 0.01f; // Increment value for the hue

	//ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(184, 92, 255, 255));
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));

	ImGui::Text(R"(
 ,______________________________________       
|_________________,----------._______   ""-,__  __....-----=====
 |_____________(_(||||||||||||)___________/   ""                |
                  `----------' BOSG. 12.2[/)) ""-,              |
                                       ""    `,  _,--....___    |
                                              `/           """"
)");


	ImGui::PopStyleColor();

	ImVec2 windowSize = ImGui::GetIO().DisplaySize;
	ImVec2 textPos = ImVec2(windowSize.x - 120, windowSize.y - 30);

	ImGui::SetCursorPos(textPos);

	// Calculate the RGB color based on the hue value
	float r = std::sin(hue * 2.0f * 3.14159f) * 0.5f + 0.5f;
	float g = std::sin((hue + 0.33333f) * 2.0f * 3.14159f) * 0.5f + 0.5f;
	float b = std::sin((hue + 0.66666f) * 2.0f * 3.14159f) * 0.5f + 0.5f;

	// Push the rainbow color for the "Made By ZILIKS" text
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(static_cast<int>(r * 255), static_cast<int>(g * 255), static_cast<int>(b * 255), 200));

	
	ImGui::Text("MADE BY ZILIKS");
	

	// Revert to the default font

	ImGui::PopStyleColor(); // Revert to the default text color

	// Increment the hue value for the next frame
	hue += hueIncrement;
	if (hue > 1.0f)
		hue -= 1.0f;
		
	//utilize else if structe to insure only 1 serial code is being sent at a time 
	//this is for the general recoil with rapid fire and secondary off

	 

		if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (GetAsyncKeyState(VK_RBUTTON) & 0x8000) && (recoilEnabled) && (!rapidFire) && (!secondaryEnabled))
		{
			arduino.send_data(("holding_down_recoil") + to_string(sliderValueHorizontal) + "," + to_string(sliderValueVertical) + "," + to_string(sliderAmountDelay));
			Sleep(sliderAmountDelay);
			

		}
		else if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (GetAsyncKeyState(VK_RBUTTON) & 0x8000) && (recoilEnabled) && (!rapidFire) && (secondaryEnabled))
		{
			arduino.send_data(("holding_down_secondary") + to_string(sliderValueHorizontalSecondary) + "," + to_string(sliderValueVerticalSecondary) + "," + to_string(sliderAmountDelaySecondary));
			Sleep(sliderAmountDelaySecondary);
		}
		else if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (recoilEnabled) && (rapidFire)) //this is to control recoil and do rapid fire and rapid fire no recoil if not zoomed
		{
			if (GetAsyncKeyState(VK_RBUTTON) & 0x8000) {
				arduino.send_data(("holding_rapid_recoil") + to_string(sliderValueHorizontal) + "," + to_string(sliderValueVertical) + "," + to_string(sliderAmountDelay));
				Sleep(sliderAmountDelay);
			}
			else if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (!GetAsyncKeyState(VK_RBUTTON) & 0x8000)) {
				arduino.send_data(("holding_down_rapid") + to_string(sliderValueHorizontal) + "," + to_string(sliderValueVertical) + "," + to_string(sliderAmountDelay));
				Sleep(20);
			}
			;
		}

		else if ((rapidFire) && (GetAsyncKeyState(VK_LBUTTON) & 0x8000)) //this is to just use rapid fire with no recoil at all
		{

			arduino.send_data(("holding_down_rapid") + to_string(sliderValueHorizontal) + "," + to_string(sliderValueVertical) + "," + to_string(sliderAmountDelay));

			Sleep(20);
		}
	
	


	//	ImGui::Text("Arduino is not connected!");

	


	

	ImGui::End();

}



