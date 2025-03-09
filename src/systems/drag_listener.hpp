#ifndef DRAGLISTENER_H
#define DRAGLISTENER_H

#include <RmlUi/Core/EventListener.h>
#include <RmlUi/Core/Types.h>
#include "ui_system.hpp"

class DragListener : public Rml::EventListener
{
public:
	// Registers an elemenet as being a container of draggable elements.
	static void RegisterDraggableElement(Rml::Element* element);
	static void LinkUISystem(UISystem* ui_system) { m_ui_system = ui_system; }

protected:
	virtual void ProcessEvent(Rml::Event& event);

private:
	// Heatknob functions

	// Gets the heatknob degree output according to start and cur coordinate.
	// The range is based on the x difference and ranges from -60 to 60
	float getHeatDegree(Rml::Vector2f coords, float curDegree);
	int getHeatLevel(float degree);
	float getCurrentDegree(Rml::Element* heatknob);
	void setHeatDegree(Rml::Element* heatknob, float degree);

	// Need to ref back to UI system to get the cauldron
	static UISystem* m_ui_system;

	// Only one drag element should be active at a time
	Rml::Element* heatElement = nullptr;
	Rml::Element* stirElement = nullptr;
	Rml::Vector2f lastCoords = Rml::Vector2f(0, 0);

	const int max_degree = 60;
};

#endif