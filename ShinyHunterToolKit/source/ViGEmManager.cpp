#include <iostream>
#include <chrono>
#include <thread>
#include "../include/ViGEmManager.h"
#include "../include/PhysicalControllerManager.h"

ViGEmManager::ViGEmManager() : m_Client(nullptr), m_VirtualController(nullptr), m_IsVirtualControllerConnected(false), m_PreviousButtonState(0) {}

ViGEmManager::~ViGEmManager()
{
    Clean();
}

bool ViGEmManager::Init()
{
    m_Client = vigem_alloc();
    if (m_Client == nullptr)
    {
        std::cerr << "Failed to allocate ViGEm client." << std::endl;
        return false;
    }

    const auto connectResult = vigem_connect(m_Client);
    if (!VIGEM_SUCCESS(connectResult))
    {
        std::cerr << "Failed to connect to ViGEmBus. Error code: " << connectResult << std::endl;
        vigem_free(m_Client);
        m_Client = nullptr;
        return false;
    }

    return true;
}

void ViGEmManager::Clean()
{
    if (m_VirtualController)
    {
        vigem_target_remove(m_Client, m_VirtualController);
        vigem_target_free(m_VirtualController);
        m_VirtualController = nullptr;
    }

    if (m_Client)
    {
        vigem_disconnect(m_Client);
        vigem_free(m_Client);
        m_Client = nullptr;
    }
}

bool ViGEmManager::ConnectController()
{
    if (m_Client == nullptr)
    {
        std::cerr << "ViGEm client is not initialized." << std::endl;
        return false;
    }

    m_VirtualController = vigem_target_x360_alloc();
    if (m_VirtualController == nullptr)
    {
        std::cerr << "Failed to allocate Xbox 360 controller target." << std::endl;
        return false;
    }

    const auto addResult = vigem_target_add(m_Client, m_VirtualController);
    if (!VIGEM_SUCCESS(addResult))
    {
        std::cerr << "Failed to add target to ViGEmBus. Error code: " << addResult << std::endl;
        vigem_target_free(m_VirtualController);
        m_VirtualController = nullptr;
        return false;
    }

    m_IsVirtualControllerConnected = true;
    return true;
}

bool ViGEmManager::DisconnectController()
{
    vigem_target_remove(m_Client, m_VirtualController);
    vigem_target_free(m_VirtualController);
    m_VirtualController = nullptr;

    m_IsVirtualControllerConnected = false;
    return true;
}

std::string ViGEmManager::CheckVirtualControllerConnected() const
{
    return m_IsVirtualControllerConnected ? "Connected!" : "Waiting for Physical Controller...";
}

XUSB_REPORT ViGEmManager::ConvertToXUSBReport(const XINPUT_GAMEPAD& p_Gamepad)
{
    XUSB_REPORT report;
    report.wButtons = p_Gamepad.wButtons;
    report.bLeftTrigger = p_Gamepad.bLeftTrigger;
    report.bRightTrigger = p_Gamepad.bRightTrigger;
    report.sThumbLX = p_Gamepad.sThumbLX;
    report.sThumbLY = p_Gamepad.sThumbLY;
    report.sThumbRX = p_Gamepad.sThumbRX;
    report.sThumbRY = p_Gamepad.sThumbRY;
    return report;
}

bool ViGEmManager::ReceiveInput(XUSB_REPORT p_Report)
{
    if (m_Client == nullptr || m_VirtualController == nullptr)
    {
        std::cerr << "ViGEm client or target is not initialized." << std::endl;
        return false;
    }

    if (!VIGEM_SUCCESS(vigem_target_x360_update(m_Client, m_VirtualController, p_Report)))
    {
        std::cerr << "Failed to update Xbox 360 controller state." << std::endl;
        return false;
    }

    //PrintNewInput(p_Report);

    return true;
}

void ViGEmManager::PrintNewInput(XUSB_REPORT p_Report)
{
    if (p_Report.wButtons != m_PreviousButtonState && p_Report.wButtons != 0)
    {
        std::cout << "\nNew button input received: " << GetButtonName(p_Report.wButtons) << std::endl;

        m_PreviousButtonState = p_Report.wButtons;
    }
    else if (p_Report.wButtons == 0)
    {
        m_PreviousButtonState = 0;
    }
}

void ViGEmManager::PressUserButtonRepeatedly(WORD p_Button)
{
    m_StopPressingUserButton = false; 

    while (!m_StopPressingUserButton)
    {
        XUSB_REPORT report = {};
        report.wButtons = p_Button;
        ReceiveInput(report);

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    XUSB_REPORT report = {};
    report.wButtons = 0;
    ReceiveInput(report);
}

void ViGEmManager::StopPressingUserButton()
{
    m_StopPressingUserButton = true;
}

void ViGEmManager::PressUserMacroRepeatedly(const std::vector<std::pair<WORD, std::chrono::milliseconds>>& buttonSequence)
{
    m_StopUserMacro = false;

    while (!m_StopUserMacro)
    {
        for (size_t i = 0; i < buttonSequence.size(); ++i)
        {
            if (m_StopUserMacro) break;

            WORD button = buttonSequence[i].first;
            auto delay = buttonSequence[i].second;

            // Wait for the specified delay before pressing the button
            std::this_thread::sleep_for(delay);

            XUSB_REPORT report = {};
            report.wButtons = button;

            // Send the button press input
            ReceiveInput(report);

            // If this is a press event, hold the button for the specified duration
            if (button != 0)
            {
                // Calculate the hold duration by checking the next event's delay
                auto holdDuration = (i + 1 < buttonSequence.size()) ? buttonSequence[i + 1].second : std::chrono::milliseconds(0);
                auto start = std::chrono::steady_clock::now();

                // Continuously send the button press input during the hold duration
                while (std::chrono::steady_clock::now() - start < holdDuration)
                {
                    ReceiveInput(report);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Adjust the interval as needed
                }

                // Send the button release input
                report.wButtons = 0;
                ReceiveInput(report);
            }
        }

        // Add a delay between the end and the start of the sequence
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Adjust the delay as needed
    }

    // Ensure all buttons are released when stopping
    XUSB_REPORT report = {};
    report.wButtons = 0;
    ReceiveInput(report);
}

void ViGEmManager::StopUserMacro()
{
    m_StopUserMacro = true;
}