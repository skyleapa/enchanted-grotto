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
	static void RegisterDragDropElement(Rml::Element* element);
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
	void setHeatDegree(float degree);

	// Ladle functions

	// Idea: Note start pos in polar coords. For each drag movement note down
	// the polar coords as well and store in a vector. Whenever the mouse passes
	// the initial angle whether forward or backwards check if we can record
	// a stir. We need a point in each quadrant who's r is at least some value.
	// If a stir is recorded, clear the vector and go again
	std::pair<float, float> getPolarCoordinates(Rml::Vector2f input, Rml::Vector2f center);
	void checkCompletedStir();
	void endStir(Rml::Element* e);

	void checkGrindingMotion();

	// Need to ref back to UI system to get the cauldron
	static UISystem* m_ui_system;

	// Stir coords, in terms of SQUARED magnitude and angle in RADIANS
	std::vector<std::pair<float, float>> stirCoords;

	// Pestle motion coords, also with squared magnitude and radians
	std::vector<std::pair<float, float>> pestleCoords;

	int curGrinds = 0;
	
	// The last heat knob coords
	Rml::Vector2f heatCoords = Rml::Vector2f(0, 0);

	// The center to calculate polar coords from
	// = center cauldron coords + an offset to account for the ladle size
	const Rml::Vector2f CAULDRON_CENTER = Rml::Vector2f(625, 285) + Rml::Vector2f(25, -55);

	// The min and max squared magnitudes to consider for stir coords
	const float MIN_STIR_RADIUS = 50 * 50;
	const float MAX_STIR_RADIUS = 170 * 170;

	bool is_heat_changing = false; // is the dial click playing

	// Area where ingredients are considered
	const Rml::Vector2f MORTAR_CENTER = Rml::Vector2f(625, 420);
	const float INGREDIENT_RADIUS = 150 * 150;
	const float MIN_GRIND_RADIUS = 200 * 200;
	const float MIN_GRIND_ANGLE = M_PI / 6;
	const float MAX_GRIND_ANGLE = 5 * M_PI / 6;
	const int REQUIRED_GRINDS = 5;
};

#endif