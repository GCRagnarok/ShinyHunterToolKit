#pragma once
#include <windows.h>
#include <iostream>
#include <thread>
#include <xinput.h>
#include <mutex>
#include <condition_variable>
#include <vector>

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
    void CheckRecordCombo(const XINPUT_STATE& p_ControllerState);
    void CheckPlayCombo(const XINPUT_STATE& p_ControllerState);

    void SendInputToVirtualController();

    void Update();

	bool IsRunning() const { return m_IsRunning; }

    bool IsRecordComboPressed(const XINPUT_STATE& p_ControllerState);
    void HandleRecordComboHeld();
    void WaitForUserButtonPress();
    void StartRepeatedButtonPress(WORD p_RepeatedButton);
    void StopRepeatedButtonPress();
    void HandleRepeatedThread();

    bool IsPlayComboPressed(const XINPUT_STATE& p_ControllerState);
    void WaitForUserButtonSequence();
    void StartMacroButtonSequence(const std::vector<std::pair<WORD, std::chrono::milliseconds>>& buttonSequence);
	void StopMacroButtonSequence();
	void HandleRecordMacroThread();
	void HandlePlaybackMacroThread();

	void StartUpdateThread();
	void StopUpdateThread();
    void RunUpdateThread();

    std::atomic<bool> m_IsRepeatedThreadRunning;
    std::atomic<bool> m_WaitingForUserInput = false;


    bool m_IsRecordComboHeld = false;
    int m_RecordComboDelay = 1;
    bool m_controllerInitialEnagage = false;
    std::atomic<bool> m_IsMacroThreadRunning;
    std::atomic<bool> m_WaitingForUserInputSequence = false;
    std::vector<std::pair<WORD, std::chrono::milliseconds>> m_ButtonSequence;

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
    std::chrono::steady_clock::time_point m_RecordComboStartTime;
    std::thread m_MacroThread;

    bool m_IsRunning = false;
	bool m_IsUpdateThreadRunning = false;
    bool m_WasRecordComboPressed;
    bool m_WasResetComboPressed;
};