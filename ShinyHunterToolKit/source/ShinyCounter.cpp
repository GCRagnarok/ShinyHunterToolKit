#include "../include/ShinyCounter.h"
#include "../include/ImGuiApp.h"

// Constructor implementation
ShinyCounter::ShinyCounter(ImGuiApp* p_ImGuiApp)
    : m_ImGuiApp(p_ImGuiApp), m_Generation(0), m_EncountersPerReset(0), m_CurrentEncounters(0)
{
}

std::string ShinyCounter::SetGeneration(int p_Generation)
{
    std::string generationNames[] = {
        "(Red/Blue/Yellow)",
        "(Gold/Silver/Crystal)",
        "(Ruby/Sapphire/Emerald/FireRed/LeafGreen)",
        "(Diamond/Pearl/Platinum/HeartGold/SoulSilver)",
        "(Black/White/Black 2/White 2)",
        "(X/Y/Omega Ruby/Alpha Sapphire)",
        "(Sun/Moon/Ultra Sun/Ultra Moon)"
    };

    if (p_Generation < 1 || p_Generation > 7)
    {
        return "Invalid generation. Please enter a number between 1 and 7.";
    }
    else
    {
        m_Generation = p_Generation;
        return generationNames[p_Generation - 1] + ".";
    }
}

std::string ShinyCounter::SetEncountersPerReset(int p_EncountersPerReset)
{
    if (p_EncountersPerReset <= 0)
    {
        m_EncountersPerReset = 0;
        return "Invalid input. Please enter a non-negative whole number greater than 0.";
    }
    else
    {
		if (p_EncountersPerReset >= 999999)
		{
			m_EncountersPerReset = 999999;
			return "The shiny counter will increment by its maximum value " + std::to_string(m_EncountersPerReset) + "!";
		}
        m_EncountersPerReset = p_EncountersPerReset;
        return "Each reset will increment the shiny counter by " + std::to_string(m_EncountersPerReset) + ".";
    }
}

std::string ShinyCounter::SetCurrentEncounters(int p_CurrentEncounters)
{
    if (p_CurrentEncounters < 0)
    {
		m_CurrentEncounters = 0;
        return "Invalid input. Please enter a non-negative whole number.";
    }
    else if (p_CurrentEncounters >= 999999)
    {
        m_CurrentEncounters = 999999;
        return "The shiny counter has been set to its maximum value " + std::to_string(m_CurrentEncounters) + "!";
    }
    else
    {
        m_CurrentEncounters = p_CurrentEncounters;
        return "The shiny counter has been set to " + std::to_string(m_CurrentEncounters) + ".";
    }

}

void ShinyCounter::Counter()
{

    if ((m_CurrentEncounters + m_EncountersPerReset) < 999999)
    {
        m_CurrentEncounters += m_EncountersPerReset;
    }
    else
    {
		m_CurrentEncounters = 999999;
    }
}