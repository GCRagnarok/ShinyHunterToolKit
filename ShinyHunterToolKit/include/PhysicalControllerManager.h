#pragma once
#include <windows.h>
#include <iostream>
#include <thread>
#include <xinput.h>
#include <mutex>
#include <condition_variable>

class ShinyCounter;
class ViGEmManager;
class ImGuiApp;

class PhysicalControllerManager
{
public:
    PhysicalControllerManager(ViGEmManager& p_viGEmManager, ShinyCounter& p_ShinyCounter, ImGuiApp& p_ImGuiApp);
    ~PhysicalControllerManager();

    bool Init();
    bool IsControllerConnected() const { return m_IsControllerConnected; }

    void CheckPhysicalControllerState();

	std::string CheckPhysicalControllerConnected();

    void CheckControllerInput(const XINPUT_STATE& p_ControllerState);
	void CheckResetCombo(const XINPUT_STATE& p_ControllerState);
	void CheckRepeatedCombo(const XINPUT_STATE& p_ControllerState);

    void SendInputToVirtualController();

    void Update();

	bool IsRunning() const { return m_IsRunning; }

    bool IsRepeatedComboPressed(const XINPUT_STATE& p_ControllerState);
    void HandleRepeatedComboHeld();
    void WaitForUserButtonPress();
    void StartRepeatedButtonPress(WORD p_RepeatedButton);
    void StopRepeatedButtonPress();

	void StartUpdateThread();
	void StopUpdateThread();
    void RunUpdateThread();

    void HandleRepeatedThread();

    bool m_IsRepeatedComboHeld = false;
	int m_RepeatedComboDelay = 1;
    std::atomic<bool> m_IsRepeatedThreadRunning;
    std::atomic<bool> m_WaitingForUserInput = false;

    bool m_controllerInitialEnagage = false;

    bool m_IsControllerConnected;
    XINPUT_STATE m_ControllerState;

    std::condition_variable m_ExitCondition;
    std::mutex m_Mutex;
    bool m_ShouldExit = false;

private:
    ShinyCounter& m_ShinyCounter;
    ViGEmManager& m_ViGEmManager;
	ImGuiApp& m_ImGuiApp;
    std::thread m_RepeatedThread;
	std::thread m_UpdateThread;
    std::chrono::steady_clock::time_point m_RepeatedComboStartTime;

    bool m_IsRunning = false;
	bool m_IsUpdateThreadRunning = false;
    bool m_WasRepeatedComboPressed;
    bool m_WasResetComboPressed;
};