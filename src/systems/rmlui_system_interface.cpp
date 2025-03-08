#include "rmlui_system_interface.hpp"
#include <iostream>

RmlUiSystemInterface::RmlUiSystemInterface(GLFWwindow* window)
    : m_window(window)
{
    m_start_time = glfwGetTime();
}

RmlUiSystemInterface::~RmlUiSystemInterface()
{
}

double RmlUiSystemInterface::GetElapsedTime()
{
    return glfwGetTime() - m_start_time;
}

bool RmlUiSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
    std::string type_str;
    switch (type)
    {
    case Rml::Log::LT_ALWAYS:
        type_str = "[Always]";
        break;
    case Rml::Log::LT_ERROR:
        type_str = "[Error]";
        break;
    case Rml::Log::LT_ASSERT:
        type_str = "[Assert]";
        break;
    case Rml::Log::LT_WARNING:
        type_str = "[Warning]";
        break;
    case Rml::Log::LT_INFO:
        type_str = "[Info]";
        break;
    case Rml::Log::LT_DEBUG:
        type_str = "[Debug]";
        break;
    default:
        type_str = "[Unknown]";
        break;
    }
    
    std::cout << "RmlUi " << type_str << ": " << message << std::endl;
    return true;
} 