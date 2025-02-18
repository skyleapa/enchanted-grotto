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

// grotto entrance in forest
Entity createGrottoEntrance(RenderSystem* renderer, vec2 position);

// create static grotto items
Entity create_grotto_non_interactive_entities(RenderSystem *renderer, vec2 position, vec2 scale, float angle, GLuint texture_asset_id,
    float width_ratio, float height_ratio, float can_collide);

Entity create_boundary_line(RenderSystem *renderer, vec2 position, vec2 scale);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// debugging red lines
Entity createLine(vec2 position, vec2 size);