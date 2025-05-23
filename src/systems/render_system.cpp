
#include <SDL.h>
#include <glm/trigonometric.hpp>
#include <iostream>

// internal
#include "render_system.hpp"
#include "tinyECS/registry.hpp"

void RenderSystem::drawGridLine(Entity entity, const mat3& projection)
{

	GridLine& gridLine = registry.gridLines.get(entity);

	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(gridLine.start_pos);
	transform.scale(gridLine.end_pos);

	assert(registry.renderRequests.has(entity));
	const RenderRequest& render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	gl_has_errors();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	if (render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		gl_has_errors();

		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	// CK: std::cout << "line color: " << color.r << ", " << color.g << ", " << color.b << std::endl;
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawTexturedMesh(Entity entity,
	const mat3& projection)
{
	Motion& motion = registry.motions.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT

	// Basic 2D transformations
	Transform transform;
	transform.translate(motion.position);
	transform.scale(motion.scale);
	transform.rotate(radians(motion.angle));

	assert(registry.renderRequests.has(entity));
	const RenderRequest& render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// texture-mapped entities - use data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		GLint isEnemyOrPlayer_uloc = glGetUniformLocation(program, "is_enemy_or_player");
		// If entity has a damage flash component, pass it into the fragment shader to blend the red tint
		if (registry.damageFlashes.has(entity)) {
			glUniform1f(isEnemyOrPlayer_uloc, true);

			GLint damageFlash_uloc = glGetUniformLocation(program, "damage_flash");
			glUniform1f(damageFlash_uloc, registry.damageFlashes.get(entity).flash_value);
			gl_has_errors();
		}
		else {
			glUniform1f(isEnemyOrPlayer_uloc, false);
		}

	}
	// .obj entities
	else if (render_request.used_effect == EFFECT_ASSET_ID::CHICKEN || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
	gl_has_errors();

	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();

	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
}

void RenderSystem::drawToScreen()
{
	// Setting shaders for the background
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::BACKGROUND]);
	gl_has_errors();

	// Clearing backbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	updateViewport();
	glDepthRange(0, 10);		  // Adjust depth range
	glClearColor(0, 0, 0, 1.0); // Black background for clearing
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
	gl_has_errors();

	// Draw the background texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	gl_has_errors();

	// Set background program
	const GLuint background_program = effects[(GLuint)EFFECT_ASSET_ID::BACKGROUND];
	gl_has_errors();

	// Set vertex position and texture coordinates (both stored in the same VBO)
	GLint in_position_loc = glGetAttribLocation(background_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	// Bind textures (off-screen render and background)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	glActiveTexture(GL_TEXTURE1);
	// Load biome as background texture
	GLuint biome = registry.screenStates.components[0].biome;
	switch (biome) {
	case ((GLuint)BIOME::FOREST):
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::FOREST_BG]); // Background texture
		break;
	case ((GLuint)BIOME::FOREST_EX):
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::FOREST_EX_BG]); // Background texture
		break;
	case ((GLuint)BIOME::GROTTO):
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::GROTTO_BG]); // Background texture
		break;
	case ((GLuint)BIOME::DESERT):
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::DESERT_BG]); // Background texture
		break;
	case ((GLuint)BIOME::MUSHROOM):
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::MUSHROOM_BG]); // Background texture
		break;
	case ((GLuint)BIOME::CRYSTAL):
		glBindTexture(GL_TEXTURE_2D, texture_gl_handles[(GLuint)TEXTURE_ASSET_ID::CRYSTAL_BG]); // Background texture
		break;
	default:
		glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
		break;
	}
	glUniform1i(glGetUniformLocation(background_program, "background_texture"), 1);
	gl_has_errors();

	// Draw background geometry (a triangle, for instance)
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr); // Draw the background
	gl_has_errors();
}

void RenderSystem::fadeScreen()
{
	// Setting shaders for the background
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::FADE]);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gl_has_errors();

	// Clearing backbuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	updateViewport();
	glDepthRange(0, 10);
	gl_has_errors();

	// Draw the background texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	gl_has_errors();

	// Set background program
	const GLuint fade_program = effects[(GLuint)EFFECT_ASSET_ID::FADE];
	gl_has_errors();

	// Set vertex position and texture coordinates (both stored in the same VBO)
	GLint in_position_loc = glGetAttribLocation(fade_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	// Bind textures (off-screen render and background)
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	if (registry.screenStates.components[0].is_switching_biome) {
		glUniform1f(glGetUniformLocation(fade_program, "darken_screen_factor"), registry.screenStates.components[0].darken_screen_factor);
	}
	gl_has_errors();

	// Draw background geometry (a triangle, for instance)
	glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, nullptr); // Draw the background
	gl_has_errors();

	glDisable(GL_BLEND);
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(UISystem* ui_system, float elapsed_ms)
{
	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();

	// clear backbuffer
	updateViewport();
	glDepthRange(0.00001, 10);

	// black background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
	// and alpha blending, one would have to sort
	// sprites back to front
	gl_has_errors();

	mat3 projection_2D = createProjectionMatrix();

	drawToScreen();

	// Textured geometry, ensuring that render layers are respected, and that y-position sorting
	// occurs for terrain and players
	std::vector<Entity> entities = process_render_requests();

	// draw all entities with a render request to the frame buffer
	for (Entity entity : entities)
	{
		// skip invisble entities
		if (registry.renderRequests.has(entity) && !registry.renderRequests.get(entity).is_visible) continue;

		// filter to entities that have a motion component
		if (registry.motions.has(entity))
		{
			// Note, its not very efficient to access elements indirectly via the entity
			// albeit iterating through all Sprites in sequence. A good point to optimize
			drawTexturedMesh(entity, projection_2D);
		}
		// draw grid lines separately, as they do not have motion but need to be rendered
		else if (registry.gridLines.has(entity))
		{
			drawGridLine(entity, projection_2D);
		}
	}

	ScreenState& screen = registry.screenStates.components[0];
	if (screen.biome != (int)BIOME::GROTTO) {
		// Draw fog
		drawFog();
	}

	// Draw water
	if (ui_system->isCauldronOpen()) {
		simulateWater(ui_system->getOpenedCauldron());
	}

	// Render ui system first, so it can be faded out
	ui_system->draw();

	// Render items inside mortars if mortar menu is open
	if (ui_system->isMortarPestleOpen()) {
		for (Entity entity : registry.mortarAndPestles.entities) {
			Inventory& mortarInventory = registry.inventories.get(entity);
			for (Entity item : mortarInventory.items) {
				if (registry.renderRequests.has(item)) {
					drawTexturedMesh(item, projection_2D);
				}
			}
		}
	}
	else {
		// Hide items in mortar when the menu is closed
		for (Entity entity : registry.mortarAndPestles.entities) {
			Inventory& mortarInventory = registry.inventories.get(entity);
			for (Entity item : mortarInventory.items) {
				if (registry.renderRequests.has(item)) {
					registry.renderRequests.get(item).is_visible = false;
				}
			}
		}
	}

	// Fade screen
	if (registry.screenStates.components[0].is_switching_biome) fadeScreen();

	// flicker-free display with a double buffer
	gl_has_errors();

	// Add to time
	iTime += elapsed_ms / 1000.f;
}

void RenderSystem::drawFog()
{
	// Setting vertex and index buffers
	// Reuse the water screen quad for fog as well
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::WATER_QUAD]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(GLuint)GEOMETRY_BUFFER_ID::WATER_QUAD]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	// Draw to fog framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fog_buffer);
	glDisable(GL_BLEND);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	const GLuint program = (GLuint)effects[(int)EFFECT_ASSET_ID::FOG];
	glUseProgram(program);
	gl_has_errors();

	vec2 resolution = vec2(frameBufferWidth, frameBufferHeight);
	glUniform2fv(glGetUniformLocation(program, "iResolution"), 1, (float*)&resolution);
	glUniform1f(glGetUniformLocation(program, "iTime"), iTime);

	ScreenState& screen = registry.screenStates.components[0];
	glUniform1f(glGetUniformLocation(program, "INTENSITY"), screen.fog_intensity);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, fog_texture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

	// Then draw to screen
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_BLEND);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

void RenderSystem::simulateWater(Entity cauldron)
{
	// FULL CREDIT TO https://www.shadertoy.com/view/tt3yzn
	// for the Navier-Stokes simulation shader

	// Water parameters
	float dx = 0.8f;            // smaller seems to make it more accurate
	float dt = 1.0f;            // how fast time progresses
	float dyeScale = scale * 2; // how fat the dye is

	// Resolution based on window size
	vec2 resolution = vec2(frameBufferWidth, frameBufferHeight);
	vec2 cauldronCenter = vec2(viewport_sizex, viewport_sizey) * CAULDRON_WATER_POS;
	cauldronCenter.x += viewport_x;
	cauldronCenter.y += viewport_y;

	// Cauldron center coords to define inner bounds
	float cauldronR = CAULDRON_D * scale / 2;
	float cauldronOuterR = (CAULDRON_D + 50.f) * scale / 2; // outer bound to improve performance

	// Calm dye flow from top if no heat and no mouse drag
	vec4 iMouse = iMouseCauldron;
	if (!isCauldronDrag) {
		vec2 bottom = vec2(cauldronCenter.x, cauldronCenter.y + scale * (CAULDRON_D / 2 - 5));
		iMouse = vec4(bottom, bottom.x - 5, bottom.y + 5);
	}

	// First draw cauldron water texture underneath fluid sim
	Cauldron& cc = registry.cauldrons.get(cauldron);
	vec4 color = vec4(cc.color / 255.f, 1.f);
	float underScale = 2.0f;
	vec4 underColor = vec4(color.x / underScale, color.y / underScale, color.z / underScale, 1.f);
	registry.colors.remove(cc.water);
	registry.colors.insert(cc.water, underColor);

	// Hacky method of drawing without creating component, curtesy of Steph
	// No renderrequest.isVisible check here, straight up draw the mesh
	drawTexturedMesh(cc.water, createProjectionMatrix());

	// Adjust dt and dye location based on heat level
	// Range 1.5 - 3.5
	if (cc.heatLevel > 0) {
		dt = (cc.heatLevel / 100.f) * 2.0f + 1.5f;
		if (!isCauldronDrag) {
			iMouse = vec4(cauldronCenter, cauldronCenter.x + 5, cauldronCenter.y + 5);
		}
	}

	// Normalize water sim speed to FPS
	dt *= WATER_FPS / m_fps;

	// Stir flash color. Kinda janky calculation but whatever
	float flash = max((float)cc.stirFlash / STIR_FLASH_DURATION, 0.f);
	color.w += flash * 0.5f;

	// Setup effect
	GLuint curEffect = (GLuint)EFFECT_ASSET_ID::WATER_A;
	bool b = true;

	// Disable blending to use multipass
	glDisable(GL_BLEND);

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::WATER_QUAD]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffers[(GLuint)GEOMETRY_BUFFER_ID::WATER_QUAD]);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	// 2 Jacobi iterations seems to be the sweet spot
	// Yes curJIterations initializes at 1 don't touch that
	const int jacobiIterations = 2;
	int curJIterations = 1;
	while (curEffect <= (GLuint)EFFECT_ASSET_ID::WATER_FINAL) {
		if (curEffect == (GLuint)EFFECT_ASSET_ID::WATER_FINAL) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glEnable(GL_BLEND);
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, b ? water_buffer_one : water_buffer_two);
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}

		const GLuint program = (GLuint)effects[curEffect];
		glUseProgram(program);
		gl_has_errors();

		glUniform1i(glGetUniformLocation(program, "iChannel0"), 2);
		glUniform1f(glGetUniformLocation(program, "maxSqm"), cauldronOuterR * cauldronOuterR);
		glUniform2fv(glGetUniformLocation(program, "cauldronCoords"), 1, (float*)&cauldronCenter);

		if (curEffect == (GLuint)EFFECT_ASSET_ID::WATER_A) {
			glUniform2fv(glGetUniformLocation(program, "iResolution"), 1, (float*)&resolution);
			glUniform4fv(glGetUniformLocation(program, "iMouse"), 1, (float*)&iMouse);
			glUniform1f(glGetUniformLocation(program, "dt"), dt);
			glUniform1f(glGetUniformLocation(program, "crSq"), cauldronR * cauldronR);
			glUniform1f(glGetUniformLocation(program, "scale"), dyeScale);
		}
		else if (curEffect == (GLuint)EFFECT_ASSET_ID::WATER_FINAL) {
			glUniform2fv(glGetUniformLocation(program, "iResolution"), 1, (float*)&resolution);
			glUniform4fv(glGetUniformLocation(program, "color"), 1, (float*)&color);
		}
		else {
			glUniform1f(glGetUniformLocation(program, "dx"), dx);
		}
		gl_has_errors();

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, b ? water_texture_two : water_texture_one);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		gl_has_errors();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Keep doing jacobi iterations 
		if (curEffect == (GLuint)EFFECT_ASSET_ID::WATER_B && curJIterations < jacobiIterations) {
			curJIterations++;
		}
		else {
			curEffect++;
		}

		b = !b;
	}
}

void RenderSystem::updateCauldronMouseLoc(double mouseX, double mouseY)
{
	// Adjust mouse coords to framebuffer coords on mac
	mouseX *= retina_scale;
	mouseY *= retina_scale;

	// Flip y mouse coords cause of opengl
	// and then add ladle offsets
	mouseY = frameBufferHeight - mouseY;
	mouseX -= LADLE_OFFSET.x * scale;
	mouseY += LADLE_OFFSET.y * scale;

	iMouseCauldron.z = iMouseCauldron.x;
	iMouseCauldron.w = iMouseCauldron.y;
	iMouseCauldron.x = mouseX;
	iMouseCauldron.y = mouseY;
}

void RenderSystem::swap_buffers()
{
	glfwSwapBuffers(window);
	gl_has_errors();
}

std::vector<Entity> RenderSystem::process_render_requests() {
	std::vector<Entity> entities = registry.renderRequests.entities;

	// don't render entities with no motion (position)
	entities.erase(std::remove_if(entities.begin(), entities.end(), [](Entity e) {
		return !registry.motions.has(e);
		}), entities.end());

	/*
		Rendering order is specified in components.hpp where background < terrain < structure < player
		Note: Terrain and Player is y-position sorted, so that players can go behind and in front of trees ect.
		Examples:
		-	Terrain: Trees, rocks, bushes
		-	Structure: Bridge, river
	*/
	std::sort(entities.begin(), entities.end(), [](Entity a, Entity b) {
		RenderRequest& renderA = registry.renderRequests.get(a);
		RenderRequest& renderB = registry.renderRequests.get(b);

		// UI always renders above everything
		if (renderA.layer == RENDER_LAYER::UI) return false;
		if (renderB.layer == RENDER_LAYER::UI) return true;

		// ITEM always renders above everything except UI
		if (renderA.layer == RENDER_LAYER::ITEM) return false;
		if (renderB.layer == RENDER_LAYER::ITEM) return true;

		// background always renders first
		if (renderA.layer == RENDER_LAYER::BACKGROUND) return true;
		if (renderB.layer == RENDER_LAYER::BACKGROUND) return false;

		// ensure structure renders above terrain
		if (renderA.layer == RENDER_LAYER::TERRAIN && renderB.layer == RENDER_LAYER::STRUCTURE) return false;
		if (renderA.layer == RENDER_LAYER::STRUCTURE && renderB.layer == RENDER_LAYER::TERRAIN) return true;

		// player should always be above structures
		if (renderA.layer == RENDER_LAYER::PLAYER && renderB.layer == RENDER_LAYER::STRUCTURE) return false;
		if (renderA.layer == RENDER_LAYER::STRUCTURE && renderB.layer == RENDER_LAYER::PLAYER) return true;

		// sort structures by sub-layer (bridges on top)
		if (renderA.layer == RENDER_LAYER::STRUCTURE && renderB.layer == RENDER_LAYER::STRUCTURE) {
			return renderA.render_sub_layer > renderB.render_sub_layer;
		}

		// Include y-position sorting between terrain and player, so that players can move behind terrain
		if (renderA.layer == RENDER_LAYER::TERRAIN || renderA.layer == RENDER_LAYER::PLAYER) {
			Motion& motionA = registry.motions.get(a);
			Motion& motionB = registry.motions.get(b);
			float bottomA = motionA.position.y + (motionA.scale.y / 2);
			float bottomB = motionB.position.y + (motionB.scale.y / 2);
			return bottomA < bottomB;
		}

		return false;
		});

	return entities;
}

mat3 RenderSystem::createProjectionMatrix()
{
	// fake projection matrix, scaled to window coordinates
	float left = 0.f;
	float top = 0.f;
	float right = (float)WINDOW_WIDTH_PX;
	float bottom = (float)WINDOW_HEIGHT_PX;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);

	return {
		{sx, 0.f, 0.f},
		{0.f, sy, 0.f},
		{tx, ty, 1.f} };
}

void RenderSystem::setViewportCoords(int x, int y, int sizex, int sizey) {
	scale = (float)sizex / WINDOW_WIDTH_PX;
	frameBufferWidth = 2 * x + sizex;
	frameBufferHeight = 2 * y + sizey;
	viewport_x = x, viewport_y = y, viewport_sizex = sizex, viewport_sizey = sizey;
}