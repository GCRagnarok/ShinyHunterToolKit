#pragma once

#include <windows.h>
#include <ViGEm/Client.h>
#include <xinput.h>
#include <iostream>
#include <string>
#include <vector>

class ViGEmManager
{
public:
    ViGEmManager();
    ~ViGEmManager();

    bool Init();
    void Clean();

    bool ConnectController();
    bool DisconnectController();
    std::string CheckVirtualControllerConnected() const;
    bool IsVirtualControllerConnected() const { return m_IsVirtualControllerConnected; }

    XUSB_REPORT ConvertToXUSBReport(const XINPUT_GAMEPAD& p_Gamepad);
    bool ReceiveInput(XUSB_REPORT p_Report);
    void PrintNewInput(XUSB_REPORT p_Report);

    void PressUserButtonRepeatedly(WORD p_Button);
    void StopPressingUserButton();

    void PressUserMacroRepeatedly(const std::vector<std::pair<WORD, std::chrono::milliseconds>>& buttonSequence);
	void StopUserMacro();

    bool m_IsVirtualControllerConnected;

    std::string GetButtonName(WORD p_Button)
    {
        switch (p_Button)
        {
        case XUSB_GAMEPAD_A: return "A";
        case XUSB_GAMEPAD_B: return "B";
        case XUSB_GAMEPAD_X: return "X";
        case XUSB_GAMEPAD_Y: return "Y";
        case XUSB_GAMEPAD_DPAD_UP: return "DPAD_UP";
        case XUSB_GAMEPAD_DPAD_DOWN: return "DPAD_DOWN";
        case XUSB_GAMEPAD_DPAD_LEFT: return "DPAD_LEFT";
        case XUSB_GAMEPAD_DPAD_RIGHT: return "DPAD_RIGHT";
        case XUSB_GAMEPAD_START: return "START";
        case XUSB_GAMEPAD_BACK: return "BACK";
        case XUSB_GAMEPAD_LEFT_SHOULDER: return "LEFT_SHOULDER";
        case XUSB_GAMEPAD_RIGHT_SHOULDER: return "RIGHT_SHOULDER";
        default: return "NULL";
        }
    }
private:
    PVIGEM_CLIENT m_Client;
    PVIGEM_TARGET m_VirtualController;
    WORD m_PreviousButtonState;
    std::atomic<bool> m_StopPressingUserButton;
    std::atomic<bool> m_StopUserMacro;
};
