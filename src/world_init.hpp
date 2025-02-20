#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "render_system.hpp"

// trees
Entity createTree(RenderSystem *renderer, vec2 position);

// player
Entity createPlayer(RenderSystem *renderer, vec2 position);

// forest bridge
Entity createForestBridge(RenderSystem *renderer, vec2 position);

// forest river
Entity createForestRiver(RenderSystem *renderer, vec2 position);

// create static grotto items
Entity create_grotto_static_entities(RenderSystem *renderer, vec2 position, vec2 scale, float angle, GLuint texture_asset_id, float can_collide);

Entity create_boundary_line(RenderSystem *renderer, vec2 position, vec2 scale);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// debugging red lines
Entity createLine(vec2 position, vec2 size);

// grotto entrance
Entity createGrottoEntrance(RenderSystem *renderer, vec2 position, int id, std::string name);

// bushes
Entity createBush(RenderSystem *renderer, vec2 position);

// magical fruit
Entity createFruit(RenderSystem *renderer, vec2 position, int id, std::string name, int amount);

// coffee bean
Entity createCoffeeBean(RenderSystem *renderer, vec2 position, int id, std::string name, int amount);

// cauldron
Entity create_cauldron(RenderSystem *renderer, vec2 position, vec2 scale, int id, std::string name);

// mortar and pestle
Entity create_mortar_pestle(RenderSystem *renderer, vec2 position, vec2 scale, int id, std::string name);

// chest
Entity create_chest(RenderSystem *renderer, vec2 position, vec2 scale, int id, std::string name);

// recipe book
Entity create_recipe_book(RenderSystem *renderer, vec2 position, vec2 scale, int id, std::string name);

// interaction textbox
Entity createTextbox(RenderSystem *renderer, vec2 position, Entity itemEntity);

// render textbox
RenderRequest getTextboxRenderRequest(Textbox &textbox);