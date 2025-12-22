// scop.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "pch.h"
#include "framework.h"
#include "scop.h"


#include "Terrain.h"
#include "GameTime.h"
#include "TestCam.h"
#include "DeferredGraphics.h"
#include "World/Entity/Entity.h"
#include "CompositeRenderer/CompositeRenderer.h"
#include "GUIManager.h"
#include "StartScene.h"
#include "BlockTextureArray.h"

//#include "FmodSound.h"

#include "json.hpp"

#define MAX_LOADSTRING 100

shared_ptr<DeferredGraphics> d_graphic;
shared_ptr<TestCam> cam;
shared_ptr<Entity> entity;
Terrain* p_terrain = nullptr;
HWND hWnd;
bool under_water = false; // 물 속에 있는 지
bool in_water = false;
float delta_time = 0;
float sun_radian = XMConvertToRadians(15);

vec3 dir;
int w_width = 800;
int w_height = 650;
bool lb_flag = false;
bool rb_flag = false;
bool fix_flag = false;
bool move_check = false;
bool correct_mouse = false;
bool first_view = false;
bool item_ui = false;
int block_type = 1;
bool start_scene_flag = true;

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    d_graphic = make_shared<DeferredGraphics>(hWnd, 800, 650);
    cam = make_shared<TestCam>(800, 650, 60, 0.1, 1000);
    
    RECT client_rect;
    GetClientRect(hWnd, &client_rect);
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCOP));

    Terrain terrain(12, 12, hWnd, w_width, w_height, 1, 10); // 짝수 단위로만
    CompositeRenderer composite_renderer;
    BlockTextureArray::Init();
    GUIManager gui_manager;
    StartScene start_scene;

    p_terrain = &terrain;
    entity = make_shared<Entity>();

    vec3 sp = vec3(-7.58921, 17, 18.8209);
    entity->setCharacter(sp, vec3(0, XMConvertToRadians(0), 0));
    cam->update(entity->getCharacterPos(), entity->getCharacterDir());
    entity->update(cam->getDir());

    terrain.setSightChunk(1);
    // test code

    MSG msg = {};
    // 기본 메시지 루프입니다:
    Time::initialize();
    cam->setCursorInClient(hWnd);
    move_check = true;
    bool click_check = false;

	//FmodSound::init();
	//FmodSound::playBGM();
    while (msg.message != WM_QUIT)
    {
        //FmodSound::playNextBGM();
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            /*if (msg.message == WM_KEYDOWN) {
            }
            DispatchMessage(&msg);*/
        }
        else
        {
            gui_manager.selectInventoryItem(block_type - 1);
            if (lb_flag) 
            {
                if (start_scene_flag) 
                {
                    if (start_scene.checkClickStartButton()) 
                    {
                        start_scene_flag = false;
                        fix_flag = true;
                        start_scene.~StartScene();
                        //FmodSound::playSelectedSound();
                    }
                    else if (start_scene.checkClickExitButton()) 
                    {
                        //FmodSound::release();
                        exit(0);
                    }
                }
                int b_type = gui_manager.getInventoryBlock(block_type - 1);
                if (item_ui == false && b_type) 
                {
                    terrain.putBlock(cam->getPos(), cam->getDir(), b_type);
                    entity->setCharacterLeftArmAnimation();
                }
                click_check = true;
                lb_flag = false;
            }
            if (rb_flag) 
            {
                if (item_ui == false) 
                {
                    terrain.deleteBlock(cam->getPos(), cam->getDir());
                    entity->setCharacterLeftArmAnimation();
                }
                rb_flag = false;
            }

            Time::update();
            delta_time = Time::DeltaTime();
            if (start_scene_flag) 
            {
                SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                start_scene.render();
            }
            else 
            {
                entity->update(cam->getDir());
                cam->update(entity->getCharacterPos(), entity->getCharacterDir());
                terrain.userPositionCheck(cam->getPos().x, cam->getPos().z);
                entity->checkEntityNowPos(); // 프레임 떨어졌을 경우 위치 조정용

                //cam->TestUpdate(); // test 용 카메라
                terrain.Render();
                if (item_ui) 
                {
                    gui_manager.render(GUITexture::TAB_ITEMS, click_check);
                    click_check = false;
                }
                else
                    gui_manager.render();
                composite_renderer.render(terrain.getSRV(), gui_manager.getSRV());
            }
        }
    }
    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCOP));
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SCOP);
    wcex.lpszClassName  = L"SCOP";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hCursor = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR2));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.
    RECT windowRect = { 0, 0, w_width, w_height };
    hWnd = CreateWindowW(L"SCOP", L"board", 
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
      CW_USEDEFAULT, 0, windowRect.right - windowRect.left, 
        windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   // 항상 화면이 위에 뜨게함
   //SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_KEYDOWN:
        {
            if (wParam == 27) {
                // ESC 키가 눌렸을 때 프로그램 종료
                if (item_ui)
                    item_ui = false;
                else {
                    if (start_scene_flag == false)
                        p_terrain->saveGame();
                    //FmodSound::release();
                    DestroyWindow(hWnd);
                }
            }
            else if (wParam == 13) {
                fix_flag ^= 1;
            }
            else if (wParam == 0x70) {
                first_view ^= 1;
            }
            else if (wParam >= 0x31 && wParam <= 0x33)
            {
                block_type = wParam - 0x30;
            }
            else if (wParam == 0x49)
            {
                item_ui ^= 1;
            }
        }
        break;
    case WM_LBUTTONDOWN:
        {
            lb_flag = true;
        }
        break;
    case WM_RBUTTONDOWN:
        {
            rb_flag = true;
        }
        break;
    case WM_MOUSEMOVE:
        {
        if (move_check && fix_flag && item_ui == false) {
            if (correct_mouse == false) {
                correct_mouse = true;
                break;
            }
            cam->onMouseMove(hWnd);
            cam->setCursorInClient(hWnd);
            correct_mouse = false;
        }
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
