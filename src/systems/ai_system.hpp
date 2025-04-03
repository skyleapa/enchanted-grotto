#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include "tinyECS/registry.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);
	void setUISystem(UISystem* ui_system) { m_ui_system = ui_system; }

private:
	void updateEnemyAI(float elapsed_ms, Entity enemy_entity, Entity player_entity);
	void moveEnemyTowardsPlayer(Motion& enemy_motion, Motion& player_motion, float elapsed_ms);
	void moveEnemyRandomly(Motion& enemy_motion, float elapsed_ms);
	void moveEnemyTowardsSpawn(Motion& enemy_motion, glm::vec2 spawn_position, float elapsed_ms);

	glm::vec2 handleCollision(const Motion& entity_motion, glm::vec2 next_position, glm::vec2 direction, float elapsed_ms);
	bool isCollision(const Motion& entity_motion);
	vec4 get_bounding_box(const Motion& motion, float width_ratio, float height_ratio);
	bool collides(const Motion& player_motion, const Motion& terrain_motion, const Terrain* terrain);

	UISystem* m_ui_system = nullptr;
};