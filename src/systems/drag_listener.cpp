#include "drag_listener.hpp"
#include "potion_system.hpp"
#include <iostream>
#include <sstream>
#include <RmlUi/Core/Element.h>

static DragListener drag_listener;
UISystem* DragListener::m_ui_system;

// Registers an element as being a container of draggable elements.
void DragListener::RegisterDraggableElement(Rml::Element* element) {
	element->AddEventListener("dragstart", &drag_listener);
	element->AddEventListener("drag", &drag_listener);
	element->AddEventListener("dragend", &drag_listener);
}

float DragListener::getHeatDegree(Rml::Vector2f coords, float curDegree) {
	if (lastCoords == Rml::Vector2f(0, 0)) {
		return 0;
	}

	float dragFactor = 1.5; // decrease = more sensitive
	float xdiff = (coords.x - lastCoords.x) / dragFactor;
	float ydiff = (coords.y - lastCoords.y) / dragFactor;
	if (curDegree <= 0) {
		ydiff *= -1;
	}
	int res = abs(xdiff) > abs(ydiff) ? curDegree + xdiff : curDegree + ydiff;
	int clippedLow = max(res, -max_degree);
	return min(clippedLow, max_degree);
}

int DragListener::getHeatLevel(float degree) {
	return (int) ((degree + max_degree) / 1.2f);
}

float DragListener::getCurrentDegree(Rml::Element* heatknob) {
	std::string heatTrans = heatknob->GetProperty(Rml::PropertyId::Transform)->Get<Rml::String>();
	std::string rotateStr = heatTrans.substr(heatTrans.find("rotate"));
	return std::stof(rotateStr.substr(7, rotateStr.find("deg")));
}

void DragListener::setHeatDegree(Rml::Element* heatknob, float degree) {
	std::string heatTrans = heatknob->GetProperty(Rml::PropertyId::Transform)->Get<Rml::String>();
	std::string before = heatTrans.substr(0, heatTrans.find("rotate"));
	std::stringstream s;
	s << before << "rotate(" << std::to_string(degree) << "deg)";
	heatknob->SetProperty("transform", s.str());
}

void DragListener::ProcessEvent(Rml::Event& event) {
	Rml::Element* cur = event.GetCurrentElement();
	if (event == "dragstart") {
		if (cur->GetId() == "heat") {
			heatElement = cur;
			lastCoords = event.GetUnprojectedMouseScreenPos();
		}
		return;
	}

	if (event == "drag") {
		if (cur->GetId() == "heat") {
			float curDegree = getCurrentDegree(cur);
			float newDegree = getHeatDegree(event.GetUnprojectedMouseScreenPos(), curDegree);
			setHeatDegree(cur, newDegree);
			lastCoords = event.GetUnprojectedMouseScreenPos();
		}
		return;
	}

	if (event == "dragend") {
		if (cur->GetId() == "heat") {
			float curDegree = getCurrentDegree(cur);
			int heatLevel = getHeatLevel(curDegree);
			PotionSystem::changeHeat(m_ui_system->getOpenedCauldron(), heatLevel);
		}	
		return;
	}
}