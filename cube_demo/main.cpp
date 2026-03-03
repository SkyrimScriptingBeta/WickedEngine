#include "WickedEngine.h"

// Orbital camera state
static float cam_yaw = 0.0f;
static float cam_pitch = 0.3f;
static float cam_distance = 5.0f;
static bool mouse_dragging = false;
static POINT mouse_last = {};

class CubeDemo : public wi::Application
{
	wi::RenderPath3D renderer;
	bool scene_initialized = false;

public:
	void Initialize() override
	{
		wi::Application::Initialize();

		infoDisplay.active = true;
		infoDisplay.watermark = true;
		infoDisplay.resolution = true;
		infoDisplay.fpsinfo = true;

		renderer.init(canvas);
		renderer.Load();
		ActivatePath(&renderer);
	}

	void Update(float dt) override
	{
		// Create scene objects once after first frame
		if (!scene_initialized)
		{
			scene_initialized = true;
			auto& scene = wi::scene::GetScene();

			scene.Entity_CreateCube("cube");

			scene.Entity_CreateLight(
				"light",
				XMFLOAT3(3.0f, 5.0f, -3.0f),
				XMFLOAT3(1.0f, 1.0f, 1.0f),
				8.0f,   // intensity
				20.0f   // range
			);

			// Add ambient so cube isn't black on unlit sides
			scene.weather.ambient = XMFLOAT3(0.15f, 0.15f, 0.15f);
		}

		// Orbital camera controls
		if (wi::input::Down(wi::input::MOUSE_BUTTON_LEFT))
		{
			XMFLOAT4 ptr = wi::input::GetPointer();
			if (!mouse_dragging)
			{
				mouse_dragging = true;
				mouse_last.x = (LONG)ptr.x;
				mouse_last.y = (LONG)ptr.y;
			}
			else
			{
				float dx = (ptr.x - (float)mouse_last.x) * 0.005f;
				float dy = (ptr.y - (float)mouse_last.y) * 0.005f;
				cam_yaw += dx;
				cam_pitch += dy;
				cam_pitch = std::max(-1.5f, std::min(1.5f, cam_pitch));
				mouse_last.x = (LONG)ptr.x;
				mouse_last.y = (LONG)ptr.y;
			}
		}
		else
		{
			mouse_dragging = false;
		}

		// Scroll to zoom (GetPointer().z = scroll delta)
		float scroll = wi::input::GetPointer().z;
		if (scroll != 0)
		{
			cam_distance -= scroll * 0.3f;
			cam_distance = std::max(1.5f, std::min(20.0f, cam_distance));
		}

		// Compute camera position from spherical coordinates
		float eye_x = cam_distance * cosf(cam_pitch) * sinf(cam_yaw);
		float eye_y = cam_distance * sinf(cam_pitch);
		float eye_z = cam_distance * cosf(cam_pitch) * cosf(cam_yaw);

		XMVECTOR eye = XMVectorSet(eye_x, eye_y + 1.0f, eye_z, 0.0f);
		XMVECTOR target = XMVectorSet(0.0f, 0.5f, 0.0f, 0.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMMATRIX view = XMMatrixLookAtLH(eye, target, up);
		XMMATRIX world = XMMatrixInverse(nullptr, view);

		auto& camera = wi::scene::GetCamera();
		camera.TransformCamera(world);
		camera.UpdateCamera();

		wi::Application::Update(dt);
	}
};

static CubeDemo app;

int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine,
	_In_ int nCmdShow)
{
	static auto WndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		switch (message)
		{
		case WM_SIZE:
		case WM_DPICHANGED:
			if (app.is_window_active)
				app.SetWindow(hWnd);
			break;
		case WM_CHAR:
			switch (wParam)
			{
			case VK_BACK:
				wi::gui::TextInputField::DeleteFromInput();
				break;
			case VK_RETURN:
				break;
			default:
				wi::gui::TextInputField::AddInput((const wchar_t)wParam);
				break;
			}
			break;
		case WM_INPUT:
			wi::input::rawinput::ParseMessage((void*)lParam);
			break;
		case WM_KILLFOCUS:
			app.is_window_active = false;
			break;
		case WM_SETFOCUS:
			app.is_window_active = true;
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return 0;
	};

	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"CubeDemo";
	RegisterClassExW(&wcex);

	HWND hWnd = CreateWindowW(
		wcex.lpszClassName, L"Wicked Engine - Cube Demo",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1280, 720,
		nullptr, nullptr, hInstance, nullptr);
	ShowWindow(hWnd, SW_SHOWDEFAULT);

	app.SetWindow(hWnd);
	wi::arguments::Parse(lpCmdLine);

	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			app.Run();
		}
	}

	wi::jobsystem::ShutDown();
	return (int)msg.wParam;
}
