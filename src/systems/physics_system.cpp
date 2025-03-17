// internal
#include "physics_system.hpp"
#include "world_init.hpp"
#include <iostream>


std::vector<vec2> PhysicsSystem::get_transformed_vertices(const Mesh& mesh, const Motion& motion)
{
	std::vector<vec2> transformed_vertices;
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	for (const auto& vertex : mesh.vertices)
	{
		// applies the transformation matrix onto our vertices
		vec3 transformed_vertex = transform.mat * vec3(vertex.position.x, vertex.position.y, 1.f);
		transformed_vertices.push_back(vec2(transformed_vertex.x, transformed_vertex.y));
	}

	return transformed_vertices;
}

vec4 get_bounding_box(const Motion& motion, float width_ratio, float height_ratio)
{
	// gets the full bounding box
	float full_width = abs(motion.scale.x);
	float full_height = abs(motion.scale.y);

	// compute a bottom centered bounding box
	float box_width = full_width * width_ratio;
	float box_height = full_height * height_ratio;

	float box_x = motion.position.x - box_width / 2;				// center the box on x-axis
	float box_y = motion.position.y + full_height / 2 - box_height; // put the box on the bottom

	return { box_x, box_y, box_width, box_height };
}

// this function checks if a point is inside a triangle using the barycentric coordinates (raycasting thing)
bool isPointInTriangle(const vec2& p, const vec2& v1, const vec2& v2, const vec2& v3) {
	// vectors and dot products
	vec2 v0 = v3 - v1;
	vec2 v1v = v2 - v1;
	vec2 v2v = p - v1;

	float dot00 = dot(v0, v0);
	float dot01 = dot(v0, v1v);
	float dot02 = dot(v0, v2v);
	float dot11 = dot(v1v, v1v);
	float dot12 = dot(v1v, v2v);

	// Using barycentric coordinates
	float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
	float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// check if the point is in triangle
	return (u >= 0) && (v >= 0) && (u + v <= 1);
}

// checks if a line segment intersects with another line segment
bool doLinesIntersect(const vec2& p1, const vec2& p2, const vec2& p3, const vec2& p4) {
	vec2 r = p2 - p1;
	vec2 s = p4 - p3;

	float rxs = r.x * s.y - r.y * s.x;

	// this calculates the cross product, if 0 then the lines are parallel or collinear (NOT INTERSECTING)
	if (fabs(rxs) < 1e-6f) {
		return false;
	}

	// calculate the parameters for the intersection point
	vec2 qp = p3 - p1;
	float t = (qp.x * s.y - qp.y * s.x) / rxs;
	float u = (qp.x * r.y - qp.y * r.x) / rxs;

	// check if the intersection point is within both line segments
	return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

bool triangleBoxOverlap(const vec2& v1, const vec2& v2, const vec2& v3, const vec4& box) {
	vec2 box_tl = { box.x, box.y };
	vec2 box_tr = { box.x + box.z, box.y };
	vec2 box_bl = { box.x, box.y + box.w };
	vec2 box_br = { box.x + box.z, box.y + box.w };

	// is any box corner is inside the triangle
	if (isPointInTriangle(box_tl, v1, v2, v3) ||
		isPointInTriangle(box_tr, v1, v2, v3) ||
		isPointInTriangle(box_bl, v1, v2, v3) ||
		isPointInTriangle(box_br, v1, v2, v3)) {
		return true;
	}

	// is any triangle vertex is inside the box
	if ((v1.x >= box.x && v1.x <= box.x + box.z && v1.y >= box.y && v1.y <= box.y + box.w) ||
		(v2.x >= box.x && v2.x <= box.x + box.z && v2.y >= box.y && v2.y <= box.y + box.w) ||
		(v3.x >= box.x && v3.x <= box.x + box.z && v3.y >= box.y && v3.y <= box.y + box.w)) {
		return true;
	}

	// is a triangle edge intersecting with any box edge
	// Triangle edges
	if (doLinesIntersect(v1, v2, box_tl, box_tr) ||
		doLinesIntersect(v1, v2, box_tr, box_br) ||
		doLinesIntersect(v1, v2, box_br, box_bl) ||
		doLinesIntersect(v1, v2, box_bl, box_tl) ||
		doLinesIntersect(v2, v3, box_tl, box_tr) ||
		doLinesIntersect(v2, v3, box_tr, box_br) ||
		doLinesIntersect(v2, v3, box_br, box_bl) ||
		doLinesIntersect(v2, v3, box_bl, box_tl) ||
		doLinesIntersect(v3, v1, box_tl, box_tr) ||
		doLinesIntersect(v3, v1, box_tr, box_br) ||
		doLinesIntersect(v3, v1, box_br, box_bl) ||
		doLinesIntersect(v3, v1, box_bl, box_tl)) {
		return true;
	}

	return false;
}

bool collides_with_mesh(const Mesh* mesh, const Motion& player_motion, const Motion& terrain_motion, const vec4& overlapBox) {
	vec4 player_box = get_bounding_box(player_motion, 0.7f, 0.3f);

	std::vector<vec2> transformed_vertices = PhysicsSystem::get_transformed_vertices(*mesh, terrain_motion);

	// Check each triangle in the mesh (vertex indices stores the indices for each triangle!!!)
	// ex. 30 vertices and 84 vertex indices, indices 0, 1, 2 are for triangle one, 3, 4, 5 for next triangle and so on
	// allows us to keep track of what indices in "vertex" make up each of the triangles
	for (size_t i = 0; i < mesh->vertex_indices.size(); i += 3) {
		// get the vertices of the triangle
		vec2 p1 = transformed_vertices[mesh->vertex_indices[i]];
		vec2 p2 = transformed_vertices[mesh->vertex_indices[i + 1]];
		vec2 p3 = transformed_vertices[mesh->vertex_indices[i + 2]];

		// check if the triangle overlaps with the player's box
		if (triangleBoxOverlap(p1, p2, p3, player_box)) {
			return true;
		}
	}

	return false;
}


bool PhysicsSystem::collides(const Motion& player_motion, const Motion& terrain_motion, const Terrain* terrain, Entity terrain_entity)
{
	float player_width_ratio = 0.7f, player_height_ratio = 0.3f;
	float terrain_width_ratio = 1.0f, terrain_height_ratio = 1.0f;

	// terrain should have normal collision of AABB
	if (terrain && terrain->collision_setting == 0.0f)
	{
		terrain_width_ratio = terrain->width_ratio;
		terrain_height_ratio = terrain->height_ratio;
	}

	// terrain should have no collision at all (the texture for a mesh collision entity)
	if (terrain && terrain->collision_setting == 2.0f)
	{
		return false;
	}

	vec4 player_box = get_bounding_box(player_motion, player_width_ratio, player_height_ratio);
	vec4 terrain_box = get_bounding_box(terrain_motion, terrain_width_ratio, terrain_height_ratio);

	// terrain is a mesh collision!
	if (terrain && terrain->collision_setting == 3.0f)
	{
		auto terrain_mesh_ptr = registry.meshPtrs.get(terrain_entity);
		if (terrain_mesh_ptr && collides_with_mesh(terrain_mesh_ptr, player_motion, terrain_motion, terrain_box))
		{
			return true;
		}
		return false;
	}

	// calculate our AABB overlapping bounding boxes for non mesh collisions
	bool overlap_x = (player_box.x < terrain_box.x + terrain_box.z) && (player_box.x + player_box.z > terrain_box.x);
	bool overlap_y = (player_box.y < terrain_box.y + terrain_box.w) && (player_box.y + player_box.w > terrain_box.y);

	return overlap_x && overlap_y;
}

bool genericCollides(const Motion& motion, const Motion& other_motion)
{
	// gets our bounding boxes for the player and terrain
	vec4 box = get_bounding_box(motion, 1.f, 1.f);
	vec4 other_box = get_bounding_box(other_motion, 1.f, 1.f);

	// calculate our AABB overlapping bounding boxes
	bool overlap_x = (box.x < other_box.x + other_box.z) && (box.x + other_box.z > other_box.x);
	bool overlap_y = (box.y < other_box.y + other_box.w) && (box.y + other_box.w > other_box.y);

	return overlap_x && overlap_y;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// first update the flash value of any enemies who took damage
	for (Entity entity : registry.damageFlashes.entities) {
		DamageFlash& flash = registry.damageFlashes.get(entity);
		flash.flash_value -= elapsed_ms * TIME_UPDATE_FACTOR; // change this for speed of flash
		if (flash.flash_value <= 0) {
			registry.damageFlashes.remove(entity); // remove once flash value goes to 0
		}
	}

	// get our one player
	if (registry.players.entities.empty())
		return;

	Entity player_entity = registry.players.entities[0];
	if (!registry.motions.has(player_entity))
		return;

	Motion& player_motion = registry.motions.get(player_entity);

	// Leave this out for now - apply to health bar in the future
	// // if player's health is below 20, keep flashing red to indicate that they're close to death
	// if (registry.players.components[0].health <= PLAYER_DYING) {
	// 	if (!registry.damageFlashes.has(player_entity)) registry.damageFlashes.emplace(player_entity);
	// }

	for (Entity terrain_entity : registry.terrains.entities)
	{
		if (!registry.motions.has(terrain_entity))
			continue;

		Motion& terrain_motion = registry.motions.get(terrain_entity);
		Terrain& terrain = registry.terrains.get(terrain_entity);
		// Collision Detection: only check collisions if one is a player and the other is terrain
		if (collides(player_motion, terrain_motion, &terrain, terrain_entity))
		{
			registry.collisions.emplace_with_duplicates(player_entity, terrain_entity);
		}

		// also check ammo-terrain detection with ammo_stopping_entities
		for (Entity ammo_entity : registry.ammo.entities) {
			if (!registry.motions.has(ammo_entity)) continue;
			Motion& ammo_motion = registry.motions.get(ammo_entity);
			if (!registry.ammo.get(ammo_entity).is_fired) continue;

			if (genericCollides(ammo_motion, terrain_motion)) {
				if (registry.renderRequests.has(terrain_entity) &&
					std::find(ammo_stopping_entities.begin(), ammo_stopping_entities.end(),
						(int)registry.renderRequests.get(terrain_entity).used_texture) != ammo_stopping_entities.end()) {
					registry.collisions.emplace_with_duplicates(ammo_entity, terrain_entity);
				}
			}
		}
	}

	// Check enemy collisions
	for (Entity enemy : registry.enemies.entities) {
		if (!registry.motions.has(enemy)) continue;
		Motion& enemy_motion = registry.motions.get(enemy);

		// with enemy
		for (Entity ammo_entity : registry.ammo.entities) {
			if (!registry.motions.has(ammo_entity)) continue;
			Motion& ammo_motion = registry.motions.get(ammo_entity);

			if (!registry.ammo.get(ammo_entity).is_fired) continue;

			if (genericCollides(ammo_motion, enemy_motion)) {
				registry.collisions.emplace_with_duplicates(ammo_entity, enemy);
				// enemy flashes red
				if (!registry.damageFlashes.has(enemy)) registry.damageFlashes.emplace(enemy);
			}
		}

		// with player
		if (genericCollides(player_motion, enemy_motion)) {
			registry.collisions.emplace_with_duplicates(player_entity, enemy);
		}
	}

}
