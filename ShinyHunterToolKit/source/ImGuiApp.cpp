#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include "../Include/ImGuiApp.h"
#include "../ImGui/imgui.h"
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_impl_glfw.h"
#include "../ImGui/imgui_impl_opengl3.h"

ImGuiApp::ImGuiApp()
    : m_PhysicalControllerManager(nullptr), 
    m_TextColorRed(1.0f, 0.0f, 0.0f, 1.0f), 
    m_TextColorGreen(0.0f, 1.0f, 0.0f, 1.0f), 
    m_TextColorYellow(1.0f, 1.0f, 0.0f, 1.0f),
	m_ShinyCounter(this), 
    m_IsFirstInstance(true), 
    m_InputCurrentEncounters(0), 
    m_ResultCurrentEncounters(""), 
    m_InputEncountersPerReset(1), 
    m_ResultsCurrentEncountersColour(1.0f, 1.0f, 1.0f, 1.0f), 
    m_IsAutomaticButtonActivated(false)
{
    Init();

    if (!m_ViGEmManager.Init())
    {
        throw std::runtime_error("Failed to initialize ViGEm client");
    }

    m_PhysicalControllerManager = new PhysicalControllerManager(m_ViGEmManager, m_ShinyCounter, *this);
    if (!m_PhysicalControllerManager->Init())
    {
        throw std::runtime_error("Failed to initialize Physical Controller Manager");
    }
}

ImGuiApp::~ImGuiApp()
{
	Clean();
}

void ImGuiApp::Init()
{
    // Initialize GLFW
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    // Create window with graphics context
    m_Window = glfwCreateWindow(1280, 720, "Shiny Hunter Tool Kit", NULL, NULL);
    if (m_Window == NULL)
        throw std::runtime_error("Failed to create GLFW window");
    glfwMakeContextCurrent(m_Window);
    glfwSwapInterval(1); // Enable vsync

    // Set minimum window size
    glfwSetWindowSizeLimits(m_Window, 1280, 720, GLFW_DONT_CARE, GLFW_DONT_CARE);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        throw std::runtime_error("Failed to initialize GLEW");

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    //Style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Load custom fonts
	io.Fonts->AddFontFromFileTTF("assets/fonts/dogica/dogicapixel.ttf", 8.0f); // default font
    io.Fonts->AddFontFromFileTTF("assets/fonts/dogica/dogicapixel.ttf", 6.0f); // small font
    io.Fonts->AddFontFromFileTTF("assets/fonts/dogica/dogicapixel.ttf", 32.0f); // large font
    io.Fonts->AddFontFromFileTTF("assets/fonts/dogica/dogicapixel.ttf", 22.0f); // H1 font
    io.Fonts->AddFontFromFileTTF("assets/fonts/dogica/dogicapixel.ttf", 16.0f); // H2 font
    io.Fonts->AddFontFromFileTTF("assets/fonts/dogica/dogicapixel.ttf", 10.0f); // H3 font

    // Change layout properties
    style.WindowPadding = ImVec2(15, 15);
    style.WindowRounding = 0.0f;
    style.FramePadding = ImVec2(5, 5);
    style.FrameRounding = 0.0f;
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 15.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabMinSize = 5.0f;
    style.GrabRounding = 3.0f;

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

void ImGuiApp::Run()
{
    // Start the controller manager thread
    m_PhysicalControllerManager->StartUpdateThread();

    // Main loop
    while (!glfwWindowShouldClose(m_Window))
    {
        // Poll and handle events
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render the GUI
        Render();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(m_Window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_Window);
    }

    // Stop the controller manager thread
    m_PhysicalControllerManager->StopUpdateThread();
}

// ImGui Helper Functions ------------------------------------------------

void ImGuiApp::CenteredText(const std::string& text)
{
    ImVec2 windowSize = ImGui::GetWindowSize();

    ImVec2 textSize = ImGui::CalcTextSize(text.c_str());

    float textPosX = (windowSize.x - textSize.x) * 0.5f;

    if (textPosX < 0.0f)
    {
        textPosX = 0.0f;
    }

    ImGui::SetCursorPosX(textPosX);

    ImGui::Text("%s", text.c_str());
}

void ImGuiApp::CenteredButton(const char* label, std::function<void()> onClick)
{
    ImVec2 windowSize = ImGui::GetWindowSize();

    ImVec2 buttonSize = ImGui::CalcTextSize(label);
    buttonSize.x += ImGui::GetStyle().FramePadding.x * 2.0f;
    buttonSize.y += ImGui::GetStyle().FramePadding.y * 2.0f;

    float buttonPosX = (windowSize.x - buttonSize.x) * 0.5f;

    if (buttonPosX < 0.0f)
    {
        buttonPosX = 0.0f;
    }

    ImGui::SetCursorPosX(buttonPosX);

    if (ImGui::Button(label))
    {
        onClick();
    }
}

void ImGuiApp::CenteredCombo(const char* label, int* currentItem, const char* const items[], int itemsCount)
{
    ImVec2 windowSize = ImGui::GetWindowSize();

    float maxItemWidth = 0.0f;
    for (int i = 0; i < itemsCount; ++i)
    {
        float itemWidth = ImGui::CalcTextSize(items[i]).x;
        if (itemWidth > maxItemWidth)
        {
            maxItemWidth = itemWidth;
        }
    }

    float additionalOffset = 10.0f;
    float comboWidth = maxItemWidth + ImGui::GetStyle().FramePadding.x * 4.0f + additionalOffset;

    ImVec2 comboSize = ImVec2(comboWidth, ImGui::GetTextLineHeightWithSpacing());

    float comboPosX = (windowSize.x - comboSize.x) * 0.5f;

    if (comboPosX < 0.0f)
    {
        comboPosX = 0.0f;
    }

    ImGui::SetCursorPosX(comboPosX);

    ImGui::SetNextItemWidth(comboWidth);

    ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(comboWidth, FLT_MAX));

    ImGui::Combo(label, currentItem, items, itemsCount);
}

// Callback function to limit input length
static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventChar >= '0' && data->EventChar <= '9')
    {
        if (data->BufTextLen >= 6)
        {
            return 1; 
        }
    }
    else
    {
        return 1;
    }
    return 0;
}

void ImGuiApp::CenteredInputInt(const char* label, int* value)
{
    ImVec2 windowSize = ImGui::GetWindowSize();

    float fixedInputWidth = 70.0f;

    float buttonWidth = ImGui::CalcTextSize("-").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    float totalWidth = buttonWidth + fixedInputWidth + buttonWidth + ImGui::GetStyle().ItemSpacing.x * 2.0f;

    float inputPosX = (windowSize.x - totalWidth) * 0.5f;
    if (inputPosX < 0.0f)
        inputPosX = 0.0f;

    ImGui::SetCursorPosX(inputPosX);

    ImGui::PushID((std::string(label) + "minus").c_str());
    if (ImGui::Button("-"))
    {
        (*value)--;
        if (*value < -999999)
            *value = -999999;
    }
    ImGui::PopID();
    ImGui::SameLine();

    char valueBuffer[7];
    snprintf(valueBuffer, sizeof(valueBuffer), "%d", *value);

    ImGui::PushItemWidth(fixedInputWidth);
    if (ImGui::InputText(label, valueBuffer, sizeof(valueBuffer), ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CallbackCharFilter, InputTextCallback))
    {
        *value = atoi(valueBuffer);
        if (*value > 999999)
            *value = 999999;
        else if (*value < -999999)
            *value = -999999;
    }
    ImGui::PopItemWidth();
    ImGui::SameLine();

    ImGui::PushID((std::string(label) + "plus").c_str());
    if (ImGui::Button("+"))
    {
        (*value)++;
        if (*value > 999999)
            *value = 999999;
    }
    ImGui::PopID();
}
// Shiny Counter GUI Functions ------------------------------------------------

void ImGuiApp::GetGenerationInput()
{
    static int selectedGeneration = 0;
    static std::string result;
    const char* generations[] = { "Generation 1 (RBY)",
        "Generation 2 (GSC)",
        "Generation 3 (RSE/FRLG)",
        "Generation 4 (DPPt/HGSS)",
        "Generation 5 (BW/BW2)",
        "Generation 6 (XY/ORAS)",
        "Generation 7 (SM/USUM)" };

    if (selectedGeneration + 1 != m_ShinyCounter.m_Generation)
    {
        result = m_ShinyCounter.SetGeneration(selectedGeneration + 1);
    }

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[5]);
	CenteredText("Generation");
    ImGui::PopFont();

	ImGui::Spacing();

    CenteredCombo("##generationCombo", &selectedGeneration, generations, IM_ARRAYSIZE(generations));

    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorGreen);
    CenteredText(result);
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
}

void ImGuiApp::GetEncountersPerResetInput()
{
    static std::string result;
    ImVec4 resultsColour = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    ImGui::Spacing();

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[5]);
    CenteredText("Encounters Per Reset");
    ImGui::PopFont();

    ImGui::Spacing();

    CenteredInputInt("##encountersPerResetInput", &m_InputEncountersPerReset);

    ImGui::Spacing();

    if (m_InputEncountersPerReset != m_ShinyCounter.m_EncountersPerReset)
    {
        result = m_ShinyCounter.SetEncountersPerReset(m_InputEncountersPerReset);
    }

    if (m_InputEncountersPerReset <= 0)
    {
        resultsColour = m_TextColorRed;
    }
    else
    {
        resultsColour = m_TextColorGreen;
    }

    ImGui::PushStyleColor(ImGuiCol_Text, resultsColour);
    CenteredText(result);
    ImGui::PopStyleColor();

    ImGui::Spacing();
}

void ImGuiApp::GetCurrentEncountersInput()
{
    ImGui::Spacing();

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[5]);
    CenteredText("Current Encounters");
    ImGui::PopFont();

    ImGui::Spacing();

    CenteredInputInt("##currentEncountersInput", &m_InputCurrentEncounters);

    ImGui::Spacing();

    if (m_IsFirstInstance && m_InputCurrentEncounters == 0)
    {
        m_ResultsCurrentEncountersColour = m_TextColorGreen;
        m_ResultCurrentEncounters = "The shiny counter has been set to 0.";
		m_IsFirstInstance = false;
    }

    CenteredButton("Set Current Encounters", [this]() {
        m_ResultCurrentEncounters = m_ShinyCounter.SetCurrentEncounters(m_InputCurrentEncounters);

        if (m_InputCurrentEncounters < 0)
        {
            m_ResultsCurrentEncountersColour = m_TextColorRed;
        }
        else
        {
            m_ResultsCurrentEncountersColour = m_TextColorGreen;
        }

        });

    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Text, m_ResultsCurrentEncountersColour);
    CenteredText(m_ResultCurrentEncounters);
    ImGui::PopStyleColor();

    ImGui::Spacing();
}

void ImGuiApp::IncrementEncounters()
{
    std::string resetCombos[] = {
    "(Generation 1 : START + SELECT + A + B)",
    "(Generation 2 : START + SELECT + A + B)",
    "(Generation 3 : START + SELECT + A + B)",
    "(Generation 4 : START + SELECT + LB + RB)",
    "(Generation 5 : START + SELECT + LB + RB)",
    "(Generation 6 : START/SELECT + LB + RB)",
    "(Generation 7 : START/SELECT + LB + RB)",
    };

    ImGui::Spacing();

    CenteredButton("Manually Increment Encounters", [this]() {
        m_ShinyCounter.Counter();
        });

    if (m_PhysicalControllerManager->m_IsControllerConnected)
    {
        ImGui::Spacing();

		CenteredText("OR");

        ImGui::Spacing();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
        CenteredText("Press the reset combo on the connected controller to automatically increment the shiny counter.");
        std::string comboText = resetCombos[m_ShinyCounter.m_Generation - 1] + ".";
        CenteredText(comboText.c_str());
        ImGui::PopFont();
    }

    ImGui::Spacing();
}

void ImGuiApp::DisplayEncounters()
{
	ImGui::Separator();

    CenteredText("Current Number of Encounters: ");

    ImGui::Spacing();

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
    std::string encounterText = std::to_string(m_ShinyCounter.GetCurrentEncounters());
    CenteredText(encounterText.c_str());
    ImGui::PopFont();

    ImGui::Spacing();
    ImGui::Spacing();

	if (m_ShinyCounter.GetCurrentEncounters() >= 999999)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorRed);
		CenteredText("The shiny counter has reached its maximum value!");
        CenteredText("You are the unluckiest person alive!");
		ImGui::PopStyleColor();
	}
}

// Controller Manager GUI Functions ------------------------------------------------

void ImGuiApp::DisplayControllerStates()
{
    if (m_PhysicalControllerManager != nullptr)
    {
        bool isPhysicalControllerConnected = m_PhysicalControllerManager->m_IsControllerConnected;
        ImVec4 physicalColor = isPhysicalControllerConnected ? m_TextColorGreen : m_TextColorRed;

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[5]);
        CenteredText("Controller Status");
        ImGui::PopFont();

		ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, physicalColor);
        CenteredText("Physical Controller:");
        CenteredText(m_PhysicalControllerManager->CheckPhysicalControllerConnected());
        ImGui::PopStyleColor();

        ImGui::Spacing();

		bool isVirtualControllerConnected = m_ViGEmManager.m_IsVirtualControllerConnected;
        ImVec4 virtualColor = isVirtualControllerConnected ? m_TextColorGreen : m_TextColorRed;

        ImGui::PushStyleColor(ImGuiCol_Text, virtualColor);
        CenteredText("Virtual Controller:");
        CenteredText(m_ViGEmManager.CheckVirtualControllerConnected());
        ImGui::PopStyleColor();

        ImGui::Spacing();
    }
}

void ImGuiApp::SetIsAutomaticButtonActived(bool p_IsAutomaticButtonActivated)
{
	m_IsAutomaticButtonActivated = p_IsAutomaticButtonActivated;
}

void ImGuiApp::StartRepeatedButtonThread(std::function<void()> p_Function)
{
    m_IsAutomaticButtonActivated = true;
    m_IsRepeatedButtonThreadRunning.store(true);
    m_RepeatedButtonThread = std::thread([this, p_Function]()
        {
			p_Function();
        });

	std::cout << "Started repeated button thread\n";
}

void ImGuiApp::StopRepeatedButtonThread()
{
    m_IsAutomaticButtonActivated = false;
    m_IsRepeatedButtonThreadRunning.store(false);
    if (m_RepeatedButtonThread.joinable())
    {
        m_RepeatedButtonThread.join();
        std::cout << "Stopped repeated button thread\n";
    }
}

void ImGuiApp::HandleImGuiRepeatedThreadStop()
{
    if (m_IsRepeatedButtonThreadRunning.load())
    {
        StopRepeatedButtonThread();
    }
    else
    {
        SetIsAutomaticButtonActived(false);
    }
}

void ImGuiApp::RepeatedButtonPress()
{
    const char* buttonLabel = m_IsAutomaticButtonActivated ? "Disengage Automatic Button Press" : "Engage Automatic Button Press";

    bool canDisplayTriggersMessage = true;
    bool isDisplayingTriggersMessage = false;

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[5]);
    CenteredText("Automatic Button Press");
    ImGui::PopFont();

    ImGui::Spacing();

    if (m_PhysicalControllerManager->m_IsControllerConnected)
    {
        CenteredButton(buttonLabel, [this]()
            {
                if (m_PhysicalControllerManager == nullptr) {
                    throw std::runtime_error("PhysicalControllerManager is not initialized");
                }

                // stop when activated by controller
                if (!m_IsRepeatedButtonThreadRunning.load() && m_PhysicalControllerManager->m_IsRepeatedThreadRunning.load())
                {
                    m_PhysicalControllerManager->StopRepeatedButtonPress();
                    SetIsAutomaticButtonActived(false);
                }
                // stop when activated by ImGui Button
                else if (m_IsRepeatedButtonThreadRunning.load() || m_PhysicalControllerManager->m_WaitingForUserInput.load())
                {
                    m_PhysicalControllerManager->StopRepeatedButtonPress();
                    StopRepeatedButtonThread();
                }
                // start when activated by ImGui Button
                else if (!m_IsRepeatedButtonThreadRunning.load() && !m_PhysicalControllerManager->m_IsRepeatedThreadRunning.load())
                {
                    StartRepeatedButtonThread([this]()
                        {
                            m_PhysicalControllerManager->HandleRepeatedThread();
                        });
                }
            });
    }

    ImGui::Spacing();

    if (!m_PhysicalControllerManager->IsControllerConnected())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorRed);
        CenteredText("Please connect a controller to use this feature.");
        ImGui::PopStyleColor();
        return;
    }
    else if (!m_PhysicalControllerManager->m_IsRepeatedThreadRunning.load())
    {
        if (m_PhysicalControllerManager->m_WaitingForUserInput.load())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorYellow);
            CenteredText("Press any button to start automatic button presses.");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorRed);
            CenteredText("automatic button press disengaged.");
            ImGui::PopStyleColor();
        }
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorGreen);
        CenteredText("automatic button press engaged!");
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
}

void ImGuiApp::SetIsRecordMacroButtonActived(bool p_IsRecordMacroButtonActived)
{
    m_IsRecordMacroButtonActivated = p_IsRecordMacroButtonActived;
}

void ImGuiApp::SetIsPlaybackMacroButtonActived(bool p_IsPlaybackMacroButtonActived)
{
    m_IsPlaybackMacroButtonActivated = p_IsPlaybackMacroButtonActived;
}

void ImGuiApp::StartPlaybackButtonThread(std::function<void()> p_Function)
{
    m_IsPlaybackMacroButtonActivated = true;
    m_IsPlaybackButtonThreadRunning.store(true);
    m_PlaybackButtonThread = std::thread([this, p_Function]()
        {
            p_Function();
        });

    std::cout << "Started repeated button thread\n";
}

void ImGuiApp::StopPlaybackButtonThread()
{
    m_IsPlaybackMacroButtonActivated = false;
    m_IsPlaybackButtonThreadRunning.store(false);
    if (m_PlaybackButtonThread.joinable())
    {

        m_PlaybackButtonThread.join();
    }
}

void ImGuiApp::HandleImGuiPlaybackThreadStop()
{
    if (m_IsPlaybackButtonThreadRunning.load())
    {
        StopPlaybackButtonThread();
    }
    else
    {
        SetIsPlaybackMacroButtonActived(false);
    }
}

void ImGuiApp::Macros()
{
    const char* recordButtonLabel = m_IsRecordMacroButtonActivated ? "Stop Recording Macro" : "Start Recording Macro";
    const char* playbackButtonLabel = m_IsPlaybackMacroButtonActivated ? "Stop Macro Playback" : "Start Macro Playback";

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[5]);
    CenteredText("Macros");
    ImGui::PopFont();

    ImGui::Spacing();

	// record macro

    if (m_PhysicalControllerManager->m_IsControllerConnected && !m_PhysicalControllerManager->m_IsMacroThreadRunning.load())
    {
        CenteredButton(recordButtonLabel, [this]()
            {
                m_PhysicalControllerManager->HandleRecordMacroThread();
            });

        if (!m_IsRecordMacroButtonActivated)
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            CenteredText("(Or hold both triggers for 1 second begin recording!)");
            ImGui::PopFont();
        }
        else
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            CenteredText("(Or hold both triggers for 1 second to stop recording!)");
            ImGui::PopFont();
        }

        ImGui::Spacing();
    }

    if (!m_PhysicalControllerManager->IsControllerConnected())
    {
        ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorRed);
        CenteredText("Please connect a controller to use this feature.");
        ImGui::PopStyleColor();
        return;
    }
	else if (!m_PhysicalControllerManager->m_IsMacroThreadRunning.load())
    {
        if (m_PhysicalControllerManager->m_WaitingForUserInputSequence.load())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorYellow);
            CenteredText("Recording macro...");
            ImGui::PopStyleColor();
        }
		else if (m_PhysicalControllerManager->m_ButtonSequence.empty())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorRed);
			CenteredText("No macro recorded.");
			ImGui::PopStyleColor();
		}
		else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorGreen);
            CenteredText("Macro ready for playback.");
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();
    }

	// playback macro

    if (m_PhysicalControllerManager->m_IsControllerConnected && !m_PhysicalControllerManager->m_ButtonSequence.empty() && !m_PhysicalControllerManager->m_WaitingForUserInputSequence.load())
    {
        CenteredButton(playbackButtonLabel, [this]()
            {
                // Stop when disengaged by controller
                if (!m_IsPlaybackButtonThreadRunning.load() && m_PhysicalControllerManager->m_IsMacroThreadRunning.load())
                {
                    m_PhysicalControllerManager->StopMacroButtonSequence();
					SetIsPlaybackMacroButtonActived(false);
                }
				// Stop when disengaged by ImGui Button
				else if (m_IsPlaybackButtonThreadRunning.load() || m_PhysicalControllerManager->m_WaitingForUserInputSequence.load())
				{
					m_PhysicalControllerManager->StopMacroButtonSequence();
					HandleImGuiPlaybackThreadStop();
				}
				// start when activated by ImGui Button
                else
                {
                    StartPlaybackButtonThread([this]()
                        {
                            m_PhysicalControllerManager->HandlePlaybackMacroThread();
                        });
                }
            });

        if (!m_IsPlaybackMacroButtonActivated)
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            CenteredText("(Or press L3 + R3 to begin playback!)");
            ImGui::PopFont();
        }
        else
        {
            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
            CenteredText("(Or press L3 + R3 to stop playback!)");
            ImGui::PopFont();
        }

        ImGui::Spacing();

        if (!m_PhysicalControllerManager->m_IsMacroThreadRunning.load())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorRed);
            CenteredText("Macro disengaged.");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, m_TextColorGreen);
            CenteredText("Macro engaged!");
            ImGui::PopStyleColor();
        }
    }

}

// ImGui Render/Clean Functions ------------------------------------------------

void ImGuiApp::Render()
{
    // Get the size of the GLFW window
    int display_w, display_h;
    glfwGetFramebufferSize(m_Window, &display_w, &display_h);

	float titleWindow_h = 50.0f;

    // Title window
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)display_w, titleWindow_h));
    ImGui::Begin("Shiny Hunter Tool Kit", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
    CenteredText("Shiny Hunter Tool Kit");
    ImGui::PopFont();
	ImGui::Spacing();
    ImGui::End();

    // Shiny Counter window
    ImGui::SetNextWindowPos(ImVec2(0, titleWindow_h));
    ImGui::SetNextWindowSize(ImVec2((float)display_w * 0.5f, (float)display_h - titleWindow_h));
    ImGui::Begin("Shiny Counter", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[4]);
    CenteredText("Shiny Counter");
    ImGui::PopFont();
    ImGui::Spacing();
    ImGui::Separator();

    GetGenerationInput();
    ImGui::Separator();

    GetEncountersPerResetInput();
    ImGui::Separator();

    GetCurrentEncountersInput();
    ImGui::Separator();

    IncrementEncounters();
    DisplayEncounters();

    ImGui::End();

    // Controller Manager
    ImGui::SetNextWindowPos(ImVec2((float)display_w * 0.5f, titleWindow_h));
    ImGui::SetNextWindowSize(ImVec2((float)display_w * 0.5f, (float)display_h - titleWindow_h));
    ImGui::Begin("Controller Manager", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[4]);
    CenteredText("Controller Manager");
    ImGui::PopFont();
    ImGui::Spacing();
    ImGui::Separator();

    DisplayControllerStates();
    ImGui::Separator();

    RepeatedButtonPress();
    ImGui::Separator();

	Macros();

    ImGui::End();
}

void ImGuiApp::Clean()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();

    delete m_PhysicalControllerManager;
    m_PhysicalControllerManager = nullptr;

	m_ViGEmManager.Clean();

	HandleImGuiRepeatedThreadStop();
}