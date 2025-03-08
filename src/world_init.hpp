#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "systems/render_system.hpp"

// trees
Entity createTree(RenderSystem* renderer, vec2 position);

// player
Entity createPlayer(RenderSystem* renderer, vec2 position);

// forest bridge
Entity createForestBridge(RenderSystem* renderer, vec2 position);

// forest river
Entity createForestRiver(RenderSystem* renderer, vec2 position);

// create static grotto items
Entity create_grotto_static_entities(RenderSystem* renderer, vec2 position, vec2 scale, float angle, GLuint texture_asset_id, float can_collide);

Entity create_boundary_line(RenderSystem* renderer, vec2 position, vec2 scale);

// grid lines to show tile positions
Entity createGridLine(vec2 start_pos, vec2 end_pos);

// debugging red lines
Entity createLine(vec2 position, vec2 size);

// grotto entrance
Entity createGrottoEntrance(RenderSystem* renderer, vec2 position, int id, std::string name);

// bushes
Entity createBush(RenderSystem* renderer, vec2 position);

// magical fruit
Entity createFruit(RenderSystem* renderer, vec2 position, int id, std::string name, int amount);

// coffee bean
Entity createCoffeeBean(RenderSystem* renderer, vec2 position, int id, std::string name, int amount);

// cauldron
Entity createCauldron(RenderSystem* renderer, vec2 position, vec2 scale, int id, std::string name);

// mortar and pestle
Entity createMortarPestle(RenderSystem* renderer, vec2 position, vec2 scale, int id, std::string name);

// chest
Entity createChest(RenderSystem* renderer, vec2 position, vec2 scale, int id, std::string name);

// recipe book
Entity createRecipeBook(RenderSystem* renderer, vec2 position, vec2 scale, int id, std::string name);

// grotto exit
Entity createGrottoExit(RenderSystem* renderer, vec2 position, int id, std::string name);

// interaction textbox
Entity createTextbox(RenderSystem* renderer, vec2 position, Entity itemEntity);

// render textbox
RenderRequest getTextboxRenderRequest(Textbox& textbox);

// enemy
Entity createEnemy(RenderSystem* renderer, vec2 position);

// render fired ammo
bool createFiredAmmo(RenderSystem* renderer, vec2 target, Entity& item_entity, Entity& player_entity);

// desert entrance
Entity createDesertEntrance(RenderSystem* renderer, vec2 position, int id, std::string name);

// desert exit
Entity createDesertExit(RenderSystem* renderer, vec2 position, int id, std::string name);

// desert tree
Entity createDesertTree(RenderSystem* renderer, vec2 position);

// cactus
Entity createDesertCactus(RenderSystem* renderer, vec2 position);

// cactus
Entity createDesertRiver(RenderSystem* renderer, vec2 position);

// sandpile at desert entrance
Entity createDesertSandPile(RenderSystem* renderer, vec2 position);

// hidden recipe in sandpile
Entity createDesertPage(RenderSystem* renderer, vec2 position);

// desert skull
Entity createDesertSkull(RenderSystem* renderer, vec2 position);