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

	// Ladle functions

	// Idea: Note start pos in polar coords. For each drag movement note down
	// the polar coords as well and store in a vector. Whenever the mouse passes
	// the initial angle whether forward or backwards check if we can record
	// a stir. We need a point in each quadrant who's r is at least some value.
	// If a stir is recorded, clear the vector and go again
	// Center coord: 625, 285

	// Need to ref back to UI system to get the cauldron
	static UISystem* m_ui_system;

	// Only one drag element should be active at a time
	Rml::Element* heatElement = nullptr;
	Rml::Element* stirElement = nullptr;
	Rml::Vector2f lastCoords = Rml::Vector2f(0, 0);

	const int max_degree = 60;
};

#endif