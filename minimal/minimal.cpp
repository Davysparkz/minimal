// minimal.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "minimal.h"

using namespace w32;
using namespace dx;

const uint8_t gnumframes{ 3 };						// no. of swapchain backbuffers.
bool gusewarp{ false };									// use WARP adapter.

uint32_t gclientwidth{ 1280 };
uint32_t gclientheigth{ 720 };

bool gisinitialized{ false };

hwnd_t ghwnd;												// window handle.
rect_t gwindowrect;										// window rectangle (used to toggle fullscreen state).

comptr<device2_t> gdevice2;
comptr<commandqueue_t> gcommandqueue;
comptr<swapchain4_t> gswapchain;
comptr<resource_t> gbackbuffers[gnumframes];
comptr<graphicscommandlist_t> gcommandlist;
comptr<commandallocator_t> gcommandallocators[gnumframes];
comptr<descriptorheap_t> grtvdescriptorheap;

uint_t grtvdescriptorsize;
uint_t gcurrentbackbufferindex;

comptr<fence_t> gfence;
uint64_t gfencevalue{ 0 };
uint64_t gframefencevalues[gnumframes];
handle_t gfenceevent;

bool gvsync{ true };										// by default, enabled V-Sync. Can be toggled with the V key.
bool gtearingsupported{ false };

bool gfullscreen{ false };								// by default, use windowed mode. Can be toggled with Alt+Enter or F11

lresult_t CALLBACK    WndProc(hwnd_t, uint_t, wparam_t, lparam_t);

void ParseCommandLineArguments()
{
	int argc;
	wchar_t** argv = ::CommandLineToArgvW(::GetCommandLine(), &argc);
	
	for (int i = 0; i < argc; ++i) {
		if (::wcscmp(argv[i], L"-w") == 0 || (::wcscmp(argv[i], L"--width") == 0)) {
			gclientwidth = ::wcstol(argv[++i], nullptr, 10);
		}
		if (::wcscmp(argv[i], L"-h") == 0 || (::wcscmp(argv[i], L"--height") == 0)) {
			gclientheigth = ::wcstol(argv[++i], nullptr, 10);
		}
		if (::wcscmp(argv[i], L"-warp") == 0 || (::wcscmp(argv[i], L"--warp") == 0)) {
			gusewarp = true;
		}
	}
	

	:: LocalFree(argv);
}

void EnableDebugLayer()
{
#if defined(_DEBUG)
	comptr<debug_t> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(iid_ppv_args(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif
}

void RegisterWindowClass(hinstance_t hInstance, const wchar_t* wndClsName)
{
	wndclassexw_s wcex;

	wcex.cbSize = sizeof(wndclassex_s);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINIMAL));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (hbrush_t)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = wndClsName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	static atom_t atom = ::RegisterClassExW(&wcex);
	assert(atom > 0);
}

hwnd_t CreateWindow(const wchar_t* wndClsName, hinstance_t hInst, const wchar_t*  wndTitle, uint32_t width, uint32_t height)
{
	int scrnW = ::GetSystemMetrics(SM_CXSCREEN);
	int scrnH = ::GetSystemMetrics(SM_CYSCREEN);

	rect_t wndRc{ 0, 0, static_cast<long>(width), static_cast<long>(height) };

	int wndW = wndRc.right - wndRc.left;
	int wndH = wndRc.bottom - wndRc.top;

	int wndX = std::max<int>(0, (scrnW - wndW) / 2);
	int wndY = std::max<int>(0, (scrnH - wndH) / 2);

	hwnd_t hWnd = ::CreateWindowExW(
		0,
		wndClsName,
		wndTitle,
		WS_OVERLAPPEDWINDOW,
		wndX,
		wndY,
		wndW,
		wndH,
		nullptr,
		nullptr,
		hInst,
		nullptr
	);

	assert(hWnd && "Failed to create window");

	return hWnd;
}

comptr<adapter4_t> GetAdapter(bool useWarp)
{
	comptr<factory4_t> dxgiFactory;

	uint_t createFactoryFlags{ 0 };

#if defined (_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, iid_ppv_args(&dxgiFactory)));


	comptr<adapter1_t> dxgiAdapter1;
	comptr<adapter4_t> dxgiAdapter4;

	if (useWarp) {
		ThrowIfFailed(dxgiFactory->EnumWarpAdapter(iid_ppv_args(&dxgiAdapter1)));
		ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
	}
	else {
		size_t maxDedicatedVideoMemory{ 0 };

		for (uint_t i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; i++) {

			dxgiadapterdesc1_s desc;

			dxgiAdapter1->GetDesc1(&desc);

			hresult_t hr = D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(device_t), nullptr);

			if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(hr) && desc.DedicatedVideoMemory > maxDedicatedVideoMemory) {

				maxDedicatedVideoMemory = desc.DedicatedVideoMemory;
				ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
			}
		}
	}

	return dxgiAdapter4;
}

comptr<device2_t> CreateDevice(comptr<adapter4_t> adapter)
{
	comptr<device2_t> device;
	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, iid_ppv_args(&device)));

#if defined(_DEBUG)
	comptr<infoqueue_t> pInfoQueue;

	if (SUCCEEDED(device.As(&pInfoQueue))) {
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// suppress messages based on their severity level.
		message_severity_s severities[]{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// suppress individual messages by their ID.
		messageid_e denyIds[]{
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE
		};

		infoqueuefilter_s newFilter{};
		newFilter.DenyList.NumSeverities = _countof(severities);
		newFilter.DenyList.pSeverityList = severities;
		newFilter.DenyList.NumIDs = _countof(denyIds);
		newFilter.DenyList.pIDList = denyIds;

		ThrowIfFailed(pInfoQueue->PushStorageFilter(&newFilter));
	}
#endif

	return device;
}

comptr<commandqueue_t> CreateCommandQueue(comptr<device2_t> device, commandlisttype_e type)
{
	comptr<commandqueue_t> cmdQueue;

	commandqueuedesc_s desc{};
	desc.Type = type;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	ThrowIfFailed(device->CreateCommandQueue(&desc, iid_ppv_args(&cmdQueue)));

	return cmdQueue;
}

bool CheckTearingSupport()
{
	bool allowTearing{ false };

	comptr<factory4_t> factory4;

	if (SUCCEEDED(CreateDXGIFactory1(iid_ppv_args(&factory4)))) {
		comptr<factory5_t> factory5;

		if (SUCCEEDED(factory4.As(&factory5))) {
			if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing) ))) {
				allowTearing = false;
			}
		}
	}

	return allowTearing == true;
}

comptr<swapchain4_t> CreateSwapChain(hwnd_t hWnd, comptr<commandqueue_t> cmdQueue, uint32_t width, uint32_t height, uint32_t bufferCount)
{
	comptr<swapchain4_t> swapChain4;
	comptr<factory4_t> factory4;

	uint_t createFactoryFlags{ 0 };

#if defined (_DEBUG)
	createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

	ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, iid_ppv_args(&factory4)));

	dxgiswapchaindesc1_s desc;
	desc.Width = width;
	desc.Height = height;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.Stereo = false;
	desc.SampleDesc = { 1, 0 };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = bufferCount;
	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	comptr<swapchain1_t> swapChain1;

	ThrowIfFailed(factory4->CreateSwapChainForHwnd(cmdQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1));

	// disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen will be handled manually.
	ThrowIfFailed(factory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER));
	ThrowIfFailed(swapChain1.As(&swapChain4));

	return swapChain4;
}

comptr<descriptorheap_t> CreateDescriptorHeap(comptr<device2_t> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors)
{
	comptr<descriptorheap_t> heap;

	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NumDescriptors = numDescriptors;
	desc.Type = type;

	ThrowIfFailed(device->CreateDescriptorHeap(&desc, iid_ppv_args(&heap)));

	return heap;
}

void UpdateRenderTargetViews(comptr<device2_t> device, comptr<swapchain4_t> swapChain, comptr<descriptorheap_t> heap)
{
	uint_t rtvDescriptorSize{ device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV) };

	cpudescriptorhandle_t rtvHandle{ heap->GetCPUDescriptorHandleForHeapStart() };

	for (uint_t i = 0; i < gnumframes; ++i) {
		comptr<resource_t> backBuffer;

		ThrowIfFailed(swapChain->GetBuffer(i, iid_ppv_args(&backBuffer)));

		device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

		gbackbuffers[i] = backBuffer;

		rtvHandle.Offset(rtvDescriptorSize);
	}
}

comptr<commandallocator_t> CreateCommandAllocator(comptr<device2_t> device, D3D12_COMMAND_LIST_TYPE type)
{
	comptr<commandallocator_t> cmdAllocator;

	ThrowIfFailed(device->CreateCommandAllocator(type, iid_ppv_args(&cmdAllocator)));

	return cmdAllocator;
}

comptr<graphicscommandlist_t> CreateCommandList(comptr<device2_t> device, 	comptr<commandallocator_t> cmdAllocator, D3D12_COMMAND_LIST_TYPE type)
{
	comptr<graphicscommandlist_t> cmdList;

	ThrowIfFailed(device->CreateCommandList(0, type, cmdAllocator.Get(), nullptr, iid_ppv_args(&cmdList)));

	ThrowIfFailed(cmdList->Close());

	return cmdList;
}

comptr<fence_t> CreateFence(comptr<device2_t> device)
{
	comptr<fence_t> fence;

	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, iid_ppv_args(&fence)));

	return fence;
}

handle_t CreateEventHandle()
{
	handle_t fenceEv;

	fenceEv = ::CreateEvent(nullptr, false, false, nullptr);

	assert(fenceEv && " Failed to create fence event.");

	return fenceEv;
}

uint64_t Signal(comptr<commandqueue_t> cmdQueue, comptr<fence_t> fence, uint64_t& fenceValue)
{
	uint64_t fenceValueForSignal{ ++fenceValue };

	ThrowIfFailed(cmdQueue->Signal(fence.Get(), fenceValueForSignal));

	return fenceValueForSignal;
}

void WaitForFenceValue(comptr<fence_t> fence, uint64_t fenceValue, handle_t fenceEvent, std::chrono::milliseconds duration = std::chrono::milliseconds::max())
{
	if (fence->GetCompletedValue() < fenceValue) {
		ThrowIfFailed(fence->SetEventOnCompletion(fenceValue, fenceEvent));

		::WaitForSingleObject(fenceEvent, static_cast<dword_t>(duration.count()));
	}
}

void Flush(comptr<commandqueue_t> cmdQueue, comptr<fence_t> fence, uint64_t& fenceValue, handle_t fenceEvent)
{
	uint64_t fenceValueForSignal{ Signal(cmdQueue, fence, fenceValue) };

	WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
}

void Update()
{
	static uint64_t frameCounter{ 0 };
	static double elapsedSeconds{ 0.0 };
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	frameCounter++;
	
	auto t1 = clock.now();
	auto deltaTime = t1 - t0;
	t0 = t1;

	elapsedSeconds += deltaTime.count()*1e-9;

	if (elapsedSeconds > 1.0) {
		char buffer[500];
		 
		auto fps = frameCounter / elapsedSeconds;

		sprintf_s(buffer, 500, "FPS: %f\n", fps);

		OutputDebugStringA(buffer);

		frameCounter = 0;
		elapsedSeconds = 0.0;
	}
}

void Render()
{
	auto cmdAllocator{ gcommandallocators[gcurrentbackbufferindex] };
	auto backBuffer{ gbackbuffers[gcurrentbackbufferindex] };

	cmdAllocator->Reset();

	gcommandlist->Reset(cmdAllocator.Get(), nullptr);

	{ /* Clear render target view. */
		resourcebarrier_t barrier{
			resourcebarrier_t::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET)
		};

		gcommandlist->ResourceBarrier(1, &barrier);

		float clearColor[]{ 0.4f, 0.6f, 0.9f, 1.0f };

		cpudescriptorhandle_t rtv{ grtvdescriptorheap->GetCPUDescriptorHandleForHeapStart(), static_cast<int>(gcurrentbackbufferindex), grtvdescriptorsize };

		gcommandlist->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	}

	{ /* Present*/
		resourcebarrier_t barrier{
			resourcebarrier_t::Transition(backBuffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT)
		};

		gcommandlist->ResourceBarrier(1, &barrier);

		ThrowIfFailed(gcommandlist->Close());

		commandlist_t* const cmdLists[]{ gcommandlist.Get() };

		gcommandqueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		uint_t syncInterval{ gvsync ? 1u : 0u };
		uint_t presentFlags{ gtearingsupported && !gvsync ? DXGI_PRESENT_ALLOW_TEARING : 0u };

		ThrowIfFailed(gswapchain->Present(syncInterval, presentFlags));

		gframefencevalues[gcurrentbackbufferindex] = Signal(gcommandqueue, gfence, gfencevalue) ;

		gcurrentbackbufferindex = gswapchain->GetCurrentBackBufferIndex();

		WaitForFenceValue(gfence, gframefencevalues[gcurrentbackbufferindex], gfenceevent);
	}
}

void Resize(uint32_t width, uint32_t height)
{
	if (gclientwidth != width || gclientheigth != height) {
		gclientwidth = std::max(1u, width);
		gclientheigth = std::max(1u, height);

		Flush(gcommandqueue, gfence, gfencevalue, gfenceevent);

		for (int i = 0; i < gnumframes; ++i) {
			gbackbuffers[i].Reset();
			gframefencevalues[i] = gframefencevalues[gcurrentbackbufferindex];
		}

		dxgiswapchaindesc_s desc{};

		ThrowIfFailed(gswapchain->GetDesc(&desc));

		ThrowIfFailed(gswapchain->ResizeBuffers(gnumframes, gclientwidth, gclientheigth, desc.BufferDesc.Format, desc.Flags));

		gcurrentbackbufferindex = gswapchain->GetCurrentBackBufferIndex();

		UpdateRenderTargetViews(gdevice2, gswapchain, grtvdescriptorheap);
	}
}

void SetFullscreen(bool fullscreen)
{
	if (gfullscreen != fullscreen) {
		gfullscreen = fullscreen;

		if (gfullscreen) {

			::GetWindowRect(ghwnd, &gwindowrect);

			uint_t wndStyle{ WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX) };

			::SetWindowLongW(ghwnd, GWL_STYLE, wndStyle);

			HMONITOR hMonitor = ::MonitorFromWindow(ghwnd, MONITOR_DEFAULTTONEAREST);
			MONITORINFOEX monInfo{};
			monInfo.cbSize = sizeof(MONITORINFOEX);

			::GetMonitorInfo(hMonitor, &monInfo);

			::SetWindowPos(ghwnd, HWND_TOP, monInfo.rcMonitor.left, monInfo.rcMonitor.top, monInfo.rcMonitor.right - monInfo.rcMonitor.left, monInfo.rcMonitor.bottom - monInfo.rcMonitor.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(ghwnd, SW_MAXIMIZE);
		} 
		else {

			::SetWindowLongW(ghwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			::SetWindowPos(ghwnd, HWND_NOTOPMOST, gwindowrect.left, gwindowrect.top, gwindowrect.right-gwindowrect.left, gwindowrect.bottom-gwindowrect.top, SWP_FRAMECHANGED | SWP_NOACTIVATE);

			::ShowWindow(ghwnd, SW_NORMAL);
		}
	}
}

int APIENTRY wWinMain(_In_ hinstance_t hInstance,  _In_opt_ hinstance_t hPrevInstance,  _In_ LPWSTR    lpCmdLine, _In_ int  nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	const wchar_t* wndClsName{ L"DX12WindowClass" };

	ParseCommandLineArguments();

	EnableDebugLayer();

	gtearingsupported = CheckTearingSupport();

	RegisterWindowClass(hInstance, wndClsName);

	ghwnd = CreateWindow(wndClsName, hInstance, L"Minimal DirectX 12 Framework", gclientwidth, gclientheigth);

	::GetWindowRect(ghwnd, &gwindowrect);

	comptr<adapter4_t> adapter4{ GetAdapter(gusewarp) };

	gdevice2 = CreateDevice(adapter4);

	gcommandqueue = CreateCommandQueue(gdevice2, D3D12_COMMAND_LIST_TYPE_DIRECT);

	gswapchain = CreateSwapChain(ghwnd, gcommandqueue, gclientwidth, gclientheigth, gnumframes);

	gcurrentbackbufferindex = gswapchain->GetCurrentBackBufferIndex();

	grtvdescriptorheap = CreateDescriptorHeap(gdevice2, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, gnumframes);

	grtvdescriptorsize = gdevice2->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	UpdateRenderTargetViews(gdevice2, gswapchain, grtvdescriptorheap);

	for (int i = 0; i < gnumframes; ++i) {
		gcommandallocators[i] = CreateCommandAllocator(gdevice2, D3D12_COMMAND_LIST_TYPE_DIRECT);
	}

	gcommandlist = CreateCommandList(gdevice2, gcommandallocators[gcurrentbackbufferindex], D3D12_COMMAND_LIST_TYPE_DIRECT);

	gfence = CreateFence(gdevice2);

	gfenceevent = CreateEventHandle();

	gisinitialized = true;

	::ShowWindow(ghwnd, SW_SHOW);

	msg_s msg{};

    // Main message loop:
    while (msg.message != WM_QUIT)
    {
		if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
		 }  
    }

	Flush(gcommandqueue, gfence, gfencevalue, gfenceevent);

	::CloseHandle(gfenceevent);

    return (int) msg.wParam;
}

lresult_t CALLBACK WndProc(hwnd_t hWnd, uint_t message, wparam_t wParam, lparam_t lParam)
{
	if (gisinitialized) {
		switch (message)
		{
		case WM_PAINT:
			Update();
			Render();
			break;
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
		{
			bool alt{ (::GetAsyncKeyState(VK_MENU) & 0x8000) != 0 };

			switch (wParam)
			{
			case 'V':
				gvsync = !gvsync;
				break;
			case VK_ESCAPE:
				::PostQuitMessage(0);
				break;
			case VK_RETURN:
				if (alt) {
			case VK_F11:
				SetFullscreen(!gfullscreen);
				}
				break;
			}
		}
			break;
		case WM_SYSCHAR:
			break;
		case WM_SIZE:
		{
			rect_t clientRc{};

			::GetClientRect(ghwnd, &clientRc);

			int width{ clientRc.right - clientRc.left };
			int height{ clientRc.bottom - clientRc.top };

			Resize(width, height);
		}
		break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			break;
		default:
			return ::DefWindowProcW(hWnd, message, wParam, lParam);
		}
	}
	else {
		return ::DefWindowProcW(hWnd, message, wParam, lParam);
	}

	return 0;
}
