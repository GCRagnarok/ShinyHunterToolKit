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

	void StartPlaybackButtonThread(std::function<void()> p_Function);
	void StopPlaybackButtonThread();

	void SetIsRecordMacroButtonActived(bool p_IsRecordMacroButtonActived);
	void SetIsPlaybackMacroButtonActived(bool p_IsPlaybackMacroButtonActived);
	void HandleImGuiPlaybackThreadStop();

	int m_InputEncountersPerReset;

	int m_InputCurrentEncounters;
	bool m_IsFirstInstance;
	std::string m_ResultCurrentEncounters;
	ImVec4 m_ResultsCurrentEncountersColour;

	bool m_IsAutomaticButtonActivated;
	std::atomic<bool> m_IsRepeatedButtonThreadRunning;

	bool m_IsRecordMacroButtonActivated;
	bool m_IsPlaybackMacroButtonActivated;
	std::atomic<bool> m_IsPlaybackButtonThreadRunning;


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
	void Macros();

    ShinyCounter m_ShinyCounter;
	PhysicalControllerManager* m_PhysicalControllerManager;
	ViGEmManager m_ViGEmManager;
	GLFWwindow* m_Window;

	ImVec4 m_TextColorRed;
	ImVec4 m_TextColorGreen;
	ImVec4 m_TextColorYellow;

	std::thread m_RepeatedButtonThread;
	std::thread m_PlaybackButtonThread;
};