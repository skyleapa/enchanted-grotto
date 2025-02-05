#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// invaders
Entity createInvader(RenderSystem* renderer, vec2 position);

// towers
Entity createTower(RenderSystem* renderer, vec2 position);
void removeTower(vec2 position);

// projectile
Entity createProjectile(vec2 pos, vec2 size, vec2 velocity);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// debugging red lines
Entity createLine(vec2 position, vec2 size);

// legacy
// the player
Entity createChicken(RenderSystem* renderer, vec2 position);