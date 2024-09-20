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
    m_WasRepeatedComboPressed(false),
    m_IsRepeatedThreadRunning(false)
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

// Check if the left trigger and right trigger are pressed
bool PhysicalControllerManager::IsRepeatedComboPressed(const XINPUT_STATE& p_ControllerState)
{
    return (p_ControllerState.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) &&
        (p_ControllerState.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
}

void PhysicalControllerManager::HandleRepeatedComboHeld()
{
    if (!m_IsRepeatedComboHeld)
    {
        m_RepeatedComboStartTime = std::chrono::steady_clock::now();
        m_IsRepeatedComboHeld = true;
    }

    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_RepeatedComboStartTime).count();
    if (duration >= m_RepeatedComboDelay)
    {
        HandleRepeatedThread();
    }
}

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
    m_IsRepeatedComboHeld = false;
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
            else if (newButtonState & XINPUT_GAMEPAD_LEFT_THUMB) repeatedButton = XUSB_GAMEPAD_LEFT_THUMB;
            else if (newButtonState & XINPUT_GAMEPAD_RIGHT_THUMB) repeatedButton = XUSB_GAMEPAD_RIGHT_THUMB;

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

        // Check for controller disengagement
        if (IsRepeatedComboPressed(newState))
        {
            if (!m_IsRepeatedComboHeld)
            {
                m_RepeatedComboStartTime = std::chrono::steady_clock::now();
                m_IsRepeatedComboHeld = true;
            }

            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_RepeatedComboStartTime).count();
            if (duration >= m_RepeatedComboDelay)
            {
                shouldExitEarly = true;
                break;
            }
        }
        else
        {
            m_IsRepeatedComboHeld = false;
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

void PhysicalControllerManager::CheckRepeatedCombo(const XINPUT_STATE& p_ControllerState)
{
    bool isRepeatedComboPressed = IsRepeatedComboPressed(p_ControllerState);

    if (isRepeatedComboPressed)
    {
        if (!m_IsRepeatedComboHeld)
        {
            m_RepeatedComboStartTime = std::chrono::steady_clock::now();
            m_IsRepeatedComboHeld = true;
        }
        else
        {
            HandleRepeatedComboHeld();
        }
    }
    else
    {
        m_IsRepeatedComboHeld = false;
    }

    m_WasRepeatedComboPressed = isRepeatedComboPressed;
}

void PhysicalControllerManager::CheckControllerInput(const XINPUT_STATE& p_ControllerState)
{
    CheckResetCombo(p_ControllerState);
    CheckRepeatedCombo(p_ControllerState);
}

void PhysicalControllerManager::SendInputToVirtualController()
{
    XUSB_REPORT report = m_ViGEmManager.ConvertToXUSBReport(m_ControllerState.Gamepad);
    m_ViGEmManager.ReceiveInput(report);
}

void PhysicalControllerManager::Update()
{
	CheckPhysicalControllerState();

    if(m_IsControllerConnected)
    {
        CheckControllerInput(m_ControllerState);
        if (m_ViGEmManager.IsVirtualControllerConnected())
        {
            SendInputToVirtualController();
        }
    }
}