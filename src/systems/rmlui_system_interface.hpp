#pragma once

#include <RmlUi/Core.h>
#include <GLFW/glfw3.h>

class RmlUiSystemInterface : public Rml::SystemInterface {
public:
    RmlUiSystemInterface(GLFWwindow* window);
    virtual ~RmlUiSystemInterface();

    // Get the current time elapsed (in seconds)
    double GetElapsedTime() override;
    
    // Log message to console
    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

private:
    GLFWwindow* m_window;
    double m_start_time;
}; 