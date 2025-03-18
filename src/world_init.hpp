#pragma once

#include "common.hpp"
#include "tinyECS/tiny_ecs.hpp"
#include "systems/render_system.hpp"

Entity createGridLine(vec2 start_pos, vec2 end_pos);
Entity createBoundaryLine(RenderSystem* renderer, vec2 position, vec2 scale);

Entity createWelcomeScreen(RenderSystem* renderer, vec2 position);

Entity createPlayer(RenderSystem* renderer, vec2 position);

// Collectable items and interaction textbox
Entity createCollectableIngredient(RenderSystem* renderer, vec2 position, ItemType type, int amount, bool canRespawn);
Entity createTextbox(RenderSystem* renderer, vec2 position, Entity itemEntity);
RenderRequest getTextboxRenderRequest(Textbox& textbox);

// forest
Entity createTree(RenderSystem* renderer, vec2 position);
Entity createForestBridge(RenderSystem* renderer, vec2 position);
// the bridge top and bottom are for the mesh part of the forest bridge
Entity createForestBridgeTop(RenderSystem* renderer, vec2 position);
Entity createForestBridgeBottom(RenderSystem* renderer, vec2 position);
Entity createForestRiver(RenderSystem* renderer, vec2 position);
Entity createGrottoStaticEntities(RenderSystem* renderer, vec2 position, vec2 scale, float angle, GLuint texture_asset_id, float can_collide);
Entity createBush(RenderSystem* renderer, vec2 position);

// grotto
Entity createCauldron(RenderSystem* renderer, vec2 position, vec2 scale, std::string name, bool create_textbox);
Entity createMortarPestle(RenderSystem* renderer, vec2 position, vec2 scale, std::string name);
Entity createChest(RenderSystem* renderer, vec2 position, vec2 scale, std::string name);
Entity createRecipeBook(RenderSystem* renderer, vec2 position, vec2 scale, std::string name);

// desert
Entity createDesertTree(RenderSystem* renderer, vec2 position);
Entity createDesertCactus(RenderSystem* renderer, vec2 position);
Entity createDesertRiver(RenderSystem* renderer, vec2 position);
Entity createDesertSandPile(RenderSystem* renderer, vec2 position);
Entity createDesertPage(RenderSystem* renderer, vec2 position);
Entity createDesertSkull(RenderSystem* renderer, vec2 position);

// Entering between biomes
Entity createForestToGrotto(RenderSystem* renderer, vec2 position, std::string name);
Entity createGrottoToForest(RenderSystem* renderer, vec2 position, std::string name);

Entity createForestToDesert(RenderSystem* renderer, vec2 position, std::string name);
Entity createDesertToForest(RenderSystem* renderer, vec2 position, std::string name);

Entity createForestToForestEx(RenderSystem* renderer, vec2 position, std::string name);
Entity createForestExToForest(RenderSystem* renderer, vec2 position, std::string name);

Entity createForestToMushroom(RenderSystem* renderer, vec2 position, std::string name);
Entity createMushroomToForest(RenderSystem* renderer, vec2 position, std::string name);

Entity createMushroomToCrystal(RenderSystem* renderer, vec2 position, std::string name);
Entity createCrystalToMushroom(RenderSystem* renderer, vec2 position, std::string name);

Entity createCrystalToForestEx(RenderSystem* renderer, vec2 position, std::string name);
Entity createForestExToCrystal(RenderSystem* renderer, vec2 position, std::string name);

// combat
Entity createEnt(RenderSystem* renderer, vec2 position, int movable, std::string name);
Entity createMummy(RenderSystem* renderer, vec2 position, int movable, std::string name);
bool createFiredAmmo(RenderSystem* renderer, vec2 target, Entity& item_entity, Entity& player_entity);