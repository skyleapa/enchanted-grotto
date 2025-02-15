#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// trees
Entity createTree(RenderSystem* renderer, vec2 position);

// player
Entity createPlayer(RenderSystem* renderer, vec2 position);

// forest bridge
Entity createForestBridge(RenderSystem* renderer, vec2 position);

// forest river
Entity createForestRiver(RenderSystem* renderer, vec2 position);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// debugging red lines
Entity createLine(vec2 position, vec2 size);