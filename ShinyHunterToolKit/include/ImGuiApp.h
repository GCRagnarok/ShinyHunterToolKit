#pragma once

#include <glfw3.h>
#include <thread>
#include <functional>
#include "../ImGui/imgui.h"
#include "ShinyCounter.h"
#include "PhysicalControllerManager.h"
#include "ViGEmManager.h"

class ImGuiApp
{
public:
    ImGuiApp();
    ~ImGuiApp();

	void Init();
    void Run();
	void Clean();

	void SetIsAutomaticButtonActived(bool p_IsAutomaticButtonActivated);
	void StartRepeatedButtonThread(std::function<void()> p_Function);
	void StopRepeatedButtonThread();
	void HandleImGuiRepeatedThreadStop();

	int m_InputEncountersPerReset;

	int m_InputCurrentEncounters;
	bool m_IsFirstInstance;
	std::string m_ResultCurrentEncounters;
	ImVec4 m_ResultsCurrentEncountersColour;

	bool m_IsAutomaticButtonActivated;
	std::atomic<bool> m_IsRepeatedButtonThreadRunning;


private:
	void Render();

	void CenteredText(const std::string& text);
	void CenteredButton(const char* label, std::function<void()> onClick);
	void CenteredCombo(const char* label, int* currentItem, const char* const items[], int itemsCount);
	void CenteredInputInt(const char* label, int* value);

	void GetGenerationInput();
	void GetEncountersPerResetInput();
	void GetCurrentEncountersInput();
	void IncrementEncounters();
	void DisplayEncounters();
	void DisplayControllerStates();
	void RepeatedButtonPress();

    ShinyCounter m_ShinyCounter;
	PhysicalControllerManager* m_PhysicalControllerManager;
	ViGEmManager m_ViGEmManager;
	GLFWwindow* m_Window;

	ImVec4 m_TextColorRed;
	ImVec4 m_TextColorGreen;
	ImVec4 m_TextColorYellow;

	std::thread m_RepeatedButtonThread;
};