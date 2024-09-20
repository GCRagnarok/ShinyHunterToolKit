#pragma once
#include <string>

class ImGuiApp;

class ShinyCounter
{
public:
	ShinyCounter(ImGuiApp* p_ImGuiApp);

    std::string SetGeneration(int p_Generation);
    std::string SetEncountersPerReset(int p_EncountersPerReset);
    std::string SetCurrentEncounters(int p_CurrentEncounters);

    void Counter();

    int GetCurrentEncounters() const { return m_CurrentEncounters; }

    int m_Generation;
    int m_EncountersPerReset;
    int m_CurrentEncounters;

    ImGuiApp* m_ImGuiApp;
};
