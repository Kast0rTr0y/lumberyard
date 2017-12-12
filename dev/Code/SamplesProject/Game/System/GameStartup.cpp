/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
#include "StdAfx.h"
#include "GameStartup.h"
#include "Core/GameplaySampleGame.h"
#include "Core/EditorGame.h"
#include "platform_impl.h"
#include "IHardwareMouse.h"
#include <IPlayerProfiles.h>
#include <ILevelSystem.h>
#include <CryLibrary.h>

#include <AzCore/Component/ComponentApplicationBus.h>

#define DLL_INITFUNC_CREATEGAME "CreateGameFramework"

using namespace LYGame;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

extern "C"
{
DLL_EXPORT IGameStartup* CreateGameStartup()
{
    return GameStartup::Create();
}

DLL_EXPORT IEditorGame* CreateEditorGame()
{
    return new LYGame::EditorGame();
}
#if defined(AZ_MONOLITHIC_BUILD)
IGameFramework* CreateGameFramework();
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

GameStartup::GameStartup()
    : m_Framework(NULL)
    , m_Game(NULL)
    , m_FrameworkDll(NULL)
    , m_bFrameworkHasInitialized(false)
{
}

GameStartup::~GameStartup()
{
    Shutdown();
}

IGameRef GameStartup::Init(SSystemInitParams& startupParams)
{
    IGameRef gameRef = NULL;
    startupParams.pGameStartup = this;

    if (InitFramework(startupParams))
    {
        ISystem* system = m_Framework->GetISystem();
        startupParams.pSystem = system;

        // register system listeners
        system->GetISystemEventDispatcher()->RegisterListener(this);

        IComponentFactoryRegistry::RegisterAllComponentFactoryNodes(*system->GetIEntitySystem()->GetComponentFactoryRegistry());

        gameRef = Reset(GAME_NAME);

        if (m_Framework->CompleteInit())
        {
            GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_RANDOM_SEED, static_cast<UINT_PTR>(gEnv->pTimer->GetAsyncTime().GetMicroSecondsAsInt64()), 0);

            if (startupParams.bExecuteCommandLine)
            {
                system->ExecuteCommandLine();
            }

            m_bFrameworkHasInitialized = true;
        }
        else
        {
            gameRef->Shutdown();
            gameRef = NULL;
        }

        m_metaStreamTest = MetastreamTest::MetastreamTestHandler::CreateInstance();
    }

    return gameRef;
}

IGameRef GameStartup::Reset(const char* modName)
{
    static char gameBuffer[sizeof(LYGame::GameplaySampleGame)];

    ISystem* system = m_Framework->GetISystem();
    ModuleInitISystem(system, "CryGame");

    m_Game = new(reinterpret_cast<void*>(gameBuffer)) LYGame::GameplaySampleGame();
    const bool initialized = (m_Game && m_Game->Init(m_Framework));

    return initialized ? &m_Game : NULL;
}

void GameStartup::Shutdown()
{
    m_bFrameworkHasInitialized = false;
    
    if (m_Game)
    {
        m_Game->Shutdown();
        m_Game = NULL;
    }

    ShutdownFramework();

    m_metaStreamTest = nullptr;
}

int GameStartup::Update(bool haveFocus, unsigned int updateFlags)
{
    return (m_Game ? m_Game->Update(haveFocus, updateFlags) : 0);
}

bool GameStartup::InitFramework(SSystemInitParams& startupParams)
{
#if !defined(AZ_MONOLITHIC_BUILD)
    m_FrameworkDll = CryLoadLibrary(GAME_FRAMEWORK_FILENAME);

    if (!m_FrameworkDll)
    {
        return false;
    }

    IGameFramework::TEntryFunction CreateGameFramework = reinterpret_cast<IGameFramework::TEntryFunction>(CryGetProcAddress(m_FrameworkDll, DLL_INITFUNC_CREATEGAME));

    if (!CreateGameFramework)
    {
        return false;
    }
#endif // !AZ_MONOLITHIC_BUILD

    m_Framework = CreateGameFramework();

    if (!m_Framework)
    {
        return false;
    }

    // Initialize the engine.
    if (!m_Framework->Init(startupParams))
    {
        return false;
    }

    ISystem* system = m_Framework->GetISystem();
    ModuleInitISystem(system, "CryGame");

#ifdef WIN32
    if (gEnv->pRenderer)
    {
        SetWindowLongPtr(reinterpret_cast<HWND>(gEnv->pRenderer->
                                                    GetHWND()), GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    }
#endif

    return true;
}

void GameStartup::ShutdownFramework()
{
    if (m_Framework)
    {
        m_Framework->Shutdown();
        m_Framework = NULL;
#ifndef AZ_MONOLITHIC_BUILD
        CryFreeLibrary(m_FrameworkDll);
#endif
        m_FrameworkDll = NULL;
    }
}

int GameStartup::Run(const char* autoStartLevelName)
{
    gEnv->pConsole->ExecuteString("exec autoexec.cfg");

#ifdef WIN32
    while (true)
    {
        ISystem* pSystem = gEnv ? gEnv->pSystem : nullptr;
        if (!pSystem)
        {
            break;
        }

        if (pSystem->PumpWindowMessage(false) == -1)
        {
            break;
        }

        if (!Update(true, 0))
        {
            // need to clean the message loop (WM_QUIT might cause problems in the case of a restart)
            // another message loop might have WM_QUIT already so we cannot rely only on this
            pSystem->PumpWindowMessage(true);
            break;
        }
    }
#else
    while (true)
    {
        if (!Update(true, 0))
        {
            break;
        }
    }
#endif // WIN32

    return 0;
}

void GameStartup::OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam)
{
    switch (event)
    {
    case ESYSTEM_EVENT_RANDOM_SEED:
        cry_random_seed(gEnv->bNoRandomSeed ? 0 : (uint32)wparam);
        break;
    case ESYSTEM_EVENT_CHANGE_FOCUS:
    {
        // 3.8.1 - disable / enable Sticky Keys!
        GetISystem()->GetISystemEventDispatcher()->OnSystemEvent(ESYSTEM_EVENT_RANDOM_SEED, (UINT_PTR)gEnv->pTimer->GetAsyncTime().GetMicroSecondsAsInt64(), 0);
    }
    break;
    case ESYSTEM_EVENT_LEVEL_LOAD_START:
        break;

    case ESYSTEM_EVENT_LEVEL_POST_UNLOAD:
        STLALLOCATOR_CLEANUP;
        break;

    case ESYSTEM_EVENT_FAST_SHUTDOWN:
        break;
    }
}



//////////////////////////////////////////////////////////////////////////
#ifdef WIN32

bool GameStartup::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
    switch (msg)
    {
    case WM_SYSCHAR:     // Prevent ALT + key combinations from creating 'ding' sounds
    {
        const bool bAlt = (lParam & (1 << 29)) != 0;
        if (bAlt && wParam == VK_F4)
        {
            return false;     // Pass though ALT+F4
        }

        *pResult = 0;
        return true;
    }
    break;

    case WM_SIZE:
        break;

    case WM_SETFOCUS:
        // 3.8.1 - set a hasWindowFocus CVar to true
        break;

    case WM_KILLFOCUS:
        // 3.8.1 - set a hasWindowFocus CVar to false
        break;

    case WM_SETCURSOR:
    {
        // This is sample code to change the displayed cursor for Windows applications.
        // Note that this sample loads a texture (ie, .TIF or .DDS), not a .ICO or .CUR resource.
        const char* const cDefaultCursor = "EngineAssets/Textures/Cursor_Green.tif";
        IHardwareMouse* const pMouse = gEnv ? gEnv->pHardwareMouse : NULL;
        assert(pMouse && "HWMouse should be initialized before window is shown, check engine initialization order");
        const bool bResult = pMouse ? pMouse->SetCursor(cDefaultCursor) : false;
        if (!bResult)
        {
            GameWarning("Unable to load cursor %s, does this file exist?", cDefaultCursor);
        }
        *pResult = bResult ? TRUE : FALSE;
        return bResult;
    }
    break;
    }
    return false;
}
#endif // WIN32


GameStartup* GameStartup::Create()
{
    static char buffer[sizeof(GameStartup)];
    return new(buffer)GameStartup();
}
