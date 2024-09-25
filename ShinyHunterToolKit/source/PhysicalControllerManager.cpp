#include "../include/PhysicalControllerManager.h"
#include "../include/ViGEmManager.h"
#include "../include/ShinyCounter.h"
#include "../include/ImGuiApp.h"

PhysicalControllerManager::PhysicalControllerManager(ViGEmManager& p_viGEmManager, ShinyCounter& p_ShinyCounter, ImGuiApp& p_ImGuiApp)
    : m_ViGEmManager(p_viGEmManager), 
    m_ShinyCounter(p_ShinyCounter), 
    m_ImGuiApp(p_ImGuiApp),
    m_IsControllerConnected(false),
    m_WasResetComboPressed(false),
    m_WasRecordComboPressed(false),
    m_IsRepeatedThreadRunning(false),
	m_IsMacroThreadRunning(false)
{
    ZeroMemory(&m_ControllerState, sizeof(XINPUT_STATE));
    m_IsRunning = true;
}

PhysicalControllerManager::~PhysicalControllerManager()
{
    if (m_IsRepeatedThreadRunning && m_RepeatedThread.joinable())
    {
        m_RepeatedThread.join();
    }

    StopUpdateThread();
}

bool PhysicalControllerManager::Init()
{
    ZeroMemory(&m_ControllerState, sizeof(XINPUT_STATE));
    return true;
}

void PhysicalControllerManager::StartUpdateThread()
{
	m_IsUpdateThreadRunning = true;
    m_UpdateThread = std::thread(&PhysicalControllerManager::RunUpdateThread, this);
}

void PhysicalControllerManager::StopUpdateThread()
{
    m_IsUpdateThreadRunning = false;
    if (m_UpdateThread.joinable())
    {
        m_UpdateThread.join();
    }
}

void PhysicalControllerManager::RunUpdateThread()
{
    while (m_IsUpdateThreadRunning)
    {
        Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Controller Status ----------------------------------------------------------

void PhysicalControllerManager::CheckPhysicalControllerState()
{
    ZeroMemory(&m_ControllerState, sizeof(XINPUT_STATE));

    bool currentConnectionState = (XInputGetState(0, &m_ControllerState) == ERROR_SUCCESS);

    if (currentConnectionState != m_IsControllerConnected)
    {
        m_IsControllerConnected = currentConnectionState;

        if (m_IsControllerConnected)
        {
            if (!m_ViGEmManager.ConnectController())
            {
                throw std::runtime_error("failed to connect to virtual controller.");
            }
        }
        else
        {
            m_ViGEmManager.DisconnectController();

			if (m_IsRepeatedThreadRunning)
			{
				StopRepeatedButtonPress();
                m_ImGuiApp.HandleImGuiRepeatedThreadStop();
			}
        }
    }
}

std::string PhysicalControllerManager::CheckPhysicalControllerConnected()
{
	if (m_IsControllerConnected)
	{
        return "Connected!";
	}
	else
	{
        return "Disconnected!";
	}
}

// Controller Reset Combo ------------------------------------------------------

void PhysicalControllerManager::CheckResetCombo(const XINPUT_STATE& p_ControllerState)
{
    WORD currentButtonState = p_ControllerState.Gamepad.wButtons;
    WORD resetButtonCombo = 0;
    bool isResetComboPressed = false;

    switch (m_ShinyCounter.m_Generation)
    {
    case 1:
    case 2:
    case 3:
        // Check if the START, BACK, A, and B buttons are held
        resetButtonCombo = XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_BACK | XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_B;
        isResetComboPressed = (currentButtonState & resetButtonCombo) == resetButtonCombo;
        break;
    case 4:
    case 5:
        // Check if the START, BACK, LB, and RB buttons are held
        resetButtonCombo = XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_BACK | XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_RIGHT_SHOULDER;
        isResetComboPressed = (currentButtonState & resetButtonCombo) == resetButtonCombo;
        break;
    case 6:
    case 7:
        // Check if either START or BACK is held with LB and RB
        isResetComboPressed = ((currentButtonState & (XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_RIGHT_SHOULDER)) == (XINPUT_GAMEPAD_START | XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_RIGHT_SHOULDER)) ||
            ((currentButtonState & (XINPUT_GAMEPAD_BACK | XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_RIGHT_SHOULDER)) == (XINPUT_GAMEPAD_BACK | XINPUT_GAMEPAD_LEFT_SHOULDER | XINPUT_GAMEPAD_RIGHT_SHOULDER));
        break;
    default:
        break;
    }

    if (m_WasResetComboPressed && !isResetComboPressed)
    {
        m_ShinyCounter.Counter();
    }

    m_WasResetComboPressed = isResetComboPressed;
}

// Repeated Button Press -------------------------------------------------------

void PhysicalControllerManager::HandleRepeatedThread()
{
    if (m_IsRepeatedThreadRunning.load() || m_WaitingForUserInput.load())
    {
        StopRepeatedButtonPress();
        m_ImGuiApp.HandleImGuiRepeatedThreadStop();
    }
    else
    {
        if (!m_ImGuiApp.m_IsAutomaticButtonActivated)
        {
            m_ImGuiApp.SetIsAutomaticButtonActived(true);
        }

        std::cout << "Please press another button on the controller to initiate repeated presses:\n";
        std::thread waitForUserButtonPressThread(&PhysicalControllerManager::WaitForUserButtonPress, this);
        waitForUserButtonPressThread.detach();

    }
}

void PhysicalControllerManager::WaitForUserButtonPress()
{
    WORD repeatedButton = 0;
    m_WaitingForUserInput.store(true);
    bool shouldExitEarly = false;

    while (true)
    {
        XINPUT_STATE newState;
        ZeroMemory(&newState, sizeof(XINPUT_STATE));
        if (XInputGetState(0, &newState) == ERROR_SUCCESS)
        {
            WORD newButtonState = newState.Gamepad.wButtons;
            if (newButtonState & XINPUT_GAMEPAD_A) repeatedButton = XUSB_GAMEPAD_A;
            else if (newButtonState & XINPUT_GAMEPAD_B) repeatedButton = XUSB_GAMEPAD_B;
            else if (newButtonState & XINPUT_GAMEPAD_X) repeatedButton = XUSB_GAMEPAD_X;
            else if (newButtonState & XINPUT_GAMEPAD_Y) repeatedButton = XUSB_GAMEPAD_Y;
            else if (newButtonState & XINPUT_GAMEPAD_START) repeatedButton = XUSB_GAMEPAD_START;
            else if (newButtonState & XINPUT_GAMEPAD_BACK) repeatedButton = XUSB_GAMEPAD_BACK;
            else if (newButtonState & XINPUT_GAMEPAD_DPAD_UP) repeatedButton = XUSB_GAMEPAD_DPAD_UP;
            else if (newButtonState & XINPUT_GAMEPAD_DPAD_DOWN) repeatedButton = XUSB_GAMEPAD_DPAD_DOWN;
            else if (newButtonState & XINPUT_GAMEPAD_DPAD_LEFT) repeatedButton = XUSB_GAMEPAD_DPAD_LEFT;
            else if (newButtonState & XINPUT_GAMEPAD_DPAD_RIGHT) repeatedButton = XUSB_GAMEPAD_DPAD_RIGHT;
            else if (newButtonState & XINPUT_GAMEPAD_LEFT_SHOULDER) repeatedButton = XUSB_GAMEPAD_LEFT_SHOULDER;
            else if (newButtonState & XINPUT_GAMEPAD_RIGHT_SHOULDER) repeatedButton = XUSB_GAMEPAD_RIGHT_SHOULDER;

            if (repeatedButton != 0)
            {
                StartRepeatedButtonPress(repeatedButton);
                break;
            }
        }

        if (!m_IsControllerConnected)
		{
			shouldExitEarly = true;
			break;
		}

        // GUI disengagement
        if (!m_ImGuiApp.m_IsAutomaticButtonActivated)
        {
            shouldExitEarly = true;
            break;
        }

        // Periodically check if the thread should exit
        std::unique_lock<std::mutex> lock(m_Mutex);
        if (m_ExitCondition.wait_for(lock, std::chrono::milliseconds(100), [this] { return m_ShouldExit; }))
        {
            std::cout << "Thread exit condition met\n";
            shouldExitEarly = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    m_WaitingForUserInput.store(false);

    if (shouldExitEarly)
    {
        m_ImGuiApp.HandleImGuiRepeatedThreadStop();
    }
}

void PhysicalControllerManager::StartRepeatedButtonPress(WORD p_RepeatedButton)
{
    m_RepeatedThread = std::thread(&ViGEmManager::PressUserButtonRepeatedly, &m_ViGEmManager, p_RepeatedButton);
    m_IsRepeatedThreadRunning.store(true);

    std::cout << "\nRepeated " << m_ViGEmManager.GetButtonName(p_RepeatedButton) << " button press started." << std::endl;
    std::cout << "\nHold the left trigger and right triggers for 2 seconds to stop." << std::endl;
}

void PhysicalControllerManager::StopRepeatedButtonPress()
{
    m_ViGEmManager.StopPressingUserButton();
    if (m_RepeatedThread.joinable())
    {
        m_RepeatedThread.join();
    }
    m_IsRepeatedThreadRunning.store(false);

    std::cout << "\nRepeated button press stopped." << std::endl;
}

// Record Macro ----------------------------------------------------------------

// Check if the left trigger and right trigger are pressed
bool PhysicalControllerManager::IsRecordComboPressed(const XINPUT_STATE& p_ControllerState)
{
    return (p_ControllerState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) &&
        (p_ControllerState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
}

void PhysicalControllerManager::CheckRecordCombo(const XINPUT_STATE& p_ControllerState)
{
    bool isRecordComboPressed = IsRecordComboPressed(p_ControllerState);

    if (isRecordComboPressed)
    {
        if (!m_IsRecordComboHeld)
        {
            m_RecordComboStartTime = std::chrono::steady_clock::now();
            m_IsRecordComboHeld = true;
        }
        else
        {
            HandleRecordComboHeld();
        }
    }
    else
    {
        m_IsRecordComboHeld = false;
    }

    m_WasRecordComboPressed = isRecordComboPressed;
}

void PhysicalControllerManager::HandleRecordComboHeld()
{
    if (!m_IsRecordComboHeld)
    {
        m_RecordComboStartTime = std::chrono::steady_clock::now();
        m_IsRecordComboHeld = true;
    }

    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_RecordComboStartTime).count();
    if (duration >= m_RecordComboDelay)
    {
        HandleRecordMacroThread();
    }
}

void PhysicalControllerManager::HandleRecordMacroThread()
{
    if (!m_ImGuiApp.m_IsPlaybackButtonThreadRunning)
    {
        if (m_WaitingForUserInputSequence.load())
        {
            m_WaitingForUserInputSequence.store(false);
            m_ImGuiApp.SetIsRecordMacroButtonActived(false);
        }
        else
        {
            if (!m_ImGuiApp.m_IsRecordMacroButtonActivated)
            {
                m_ImGuiApp.SetIsRecordMacroButtonActived(true);
            }

            std::cout << "Please input your button sequence and press the GUI button or record combo when sequence is complete:\n";
            std::thread waitForUserButtonSequenceThread(&PhysicalControllerManager::WaitForUserButtonSequence, this);
            waitForUserButtonSequenceThread.detach();
        }
    }
    m_IsRecordComboHeld = false;
}

void PhysicalControllerManager::WaitForUserButtonSequence()
{
    m_ButtonSequence.clear();

    m_WaitingForUserInputSequence.store(true); // Set to true while waiting for user input
    auto startTime = std::chrono::steady_clock::now();
    WORD lastButtons = 0;
    bool shouldExitEarly = false;

    while (true)
    {
        // Capture the current button state
        XINPUT_STATE state;
        if (XInputGetState(0, &state) == ERROR_SUCCESS)
        {
            WORD buttons = state.Gamepad.wButtons;

            // End button sequence recording when GUI button is pressed
            if (!m_ImGuiApp.m_IsRecordMacroButtonActivated)
            {
                if (m_ButtonSequence.empty())
                {
                    std::cout << "No button sequence recorded." << std::endl;
                    break;
                }
                else
                {
                    std::cout << "Button sequence input complete." << std::endl;
                    m_ImGuiApp.SetIsRecordMacroButtonActived(false);
                    break;
                }
            }

            // Calculate the time elapsed since the last button press
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime);

            // Record button press and release events
            if (buttons != lastButtons)
            {
                // Record button press
                if (buttons != 0)
                {
                    m_ButtonSequence.emplace_back(buttons, elapsedTime);
                }
                // Record button release
                else if (lastButtons != 0)
                {
                    m_ButtonSequence.emplace_back(0, elapsedTime);
                }

                startTime = currentTime;
                lastButtons = buttons;
            }

            if (m_WaitingForUserInputSequence.load() == false)
            {
                m_ImGuiApp.SetIsRecordMacroButtonActived(false);
                break;
            }
        }

        // Check for controller disconnection
        if (!m_IsControllerConnected)
        {
            shouldExitEarly = true;
            break;
        }

        // Check for controller disengagement
        if (IsRecordComboPressed(state))
        {
            if (!m_IsRecordComboHeld)
            {
                m_RecordComboStartTime = std::chrono::steady_clock::now();
                m_IsRecordComboHeld = true;
            }

            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_RecordComboStartTime).count();
            if (duration >= m_RecordComboDelay)
            {
                shouldExitEarly = true;
                break;
            }
        }
        else
        {
            m_IsRecordComboHeld = false;
        }

        // GUI disengagement
        if (!m_ImGuiApp.m_IsRecordMacroButtonActivated)
        {
            shouldExitEarly = true;
            break;
        }

        // Periodically check if the thread should exit
        std::unique_lock<std::mutex> lock(m_Mutex);
        if (m_ExitCondition.wait_for(lock, std::chrono::milliseconds(100), [this] { return m_ShouldExit; }))
        {
            std::cout << "Thread exit condition met\n";
            shouldExitEarly = true;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    m_WaitingForUserInputSequence.store(false);

    if (shouldExitEarly)
    {
        m_ImGuiApp.SetIsRecordMacroButtonActived(false);
    }
}

// Playback Macro --------------------------------------------------------------

// Check if the left thumbstick (L3) and right thumbstick (R3) are pressed
bool PhysicalControllerManager::IsPlayComboPressed(const XINPUT_STATE& p_ControllerState)
{
    return (p_ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) &&
        (p_ControllerState.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
}

void PhysicalControllerManager::CheckPlayCombo(const XINPUT_STATE& p_ControllerState)
{
    static bool wasPlayComboPressed = false;
    bool isPlayComboPressed = IsPlayComboPressed(p_ControllerState);

    if (isPlayComboPressed && !wasPlayComboPressed)
    {
        HandlePlaybackMacroThread();
    }

    wasPlayComboPressed = isPlayComboPressed;
}

void PhysicalControllerManager::HandlePlaybackMacroThread()
{
    if (m_IsMacroThreadRunning.load())
    {
        StopMacroButtonSequence();
        m_ImGuiApp.HandleImGuiPlaybackThreadStop();
    }
    else
    {
        if (!m_ImGuiApp.m_IsPlaybackMacroButtonActivated)
        {
            m_ImGuiApp.SetIsPlaybackMacroButtonActived(true);
        }

        StartMacroButtonSequence(m_ButtonSequence);
    }
}

void PhysicalControllerManager::StartMacroButtonSequence(const std::vector<std::pair<WORD, std::chrono::milliseconds>>& buttonSequence)
{
    m_MacroThread = std::thread(&ViGEmManager::PressUserMacroRepeatedly, &m_ViGEmManager, buttonSequence);
    m_IsMacroThreadRunning.store(true);

    std::cout << "\nMacro button sequence started." << std::endl;
}

void PhysicalControllerManager::StopMacroButtonSequence()
{
    m_ViGEmManager.StopUserMacro();
    if (m_MacroThread.joinable())
    {
        m_MacroThread.join();
        std::cout << "Macro thread joined\n";
    }
    m_IsMacroThreadRunning.store(false);

    std::cout << "\nMacro stopped." << std::endl;
}

// Controller Updates ---------------------------------------------------------

void PhysicalControllerManager::CheckControllerInput(const XINPUT_STATE& p_ControllerState)
{
    CheckResetCombo(p_ControllerState);
    CheckRecordCombo(p_ControllerState);
    CheckPlayCombo(p_ControllerState);
}

void PhysicalControllerManager::SendInputToVirtualController()
{
    XUSB_REPORT report = m_ViGEmManager.ConvertToXUSBReport(m_ControllerState.Gamepad);
    m_ViGEmManager.ReceiveInput(report);
}

void PhysicalControllerManager::Update()
{
    CheckPhysicalControllerState();

    if (m_IsControllerConnected)
    {
        CheckControllerInput(m_ControllerState);
        if (m_ViGEmManager.IsVirtualControllerConnected())
        {
            SendInputToVirtualController();
        }
    }
}