
#include "raylib.h"
#include "math_toolbox.h"

#include "enums.h"
#include "defines.h"

#include "game.h"
#include "floating_mine.h"
#include "bullet_move.h"
#include "concave_collider.h"

static convex_polygon* getFloatingConvexs(float x, float y, float m_size)
{
	float nearDist = 23.f, farDist = 75.f;

	float2* pts1 = new (float2[4]){ {x, y}, {x - (sinf(M_PI / 3.f) * nearDist) * m_size, y - (cosf(M_PI / 3.f) * nearDist) * m_size}, {x - (0 * farDist) * m_size, y - (1 * farDist) * m_size}, {x - (sinf(-M_PI / 3.f) * nearDist) * m_size, y - (cosf(-M_PI / 3.f) * nearDist) * m_size } };
	float2* pts2 = new (float2[4]){ {x, y}, {x - (sinf(M_PI / 3.f) * nearDist) * m_size, y - (cosf(M_PI / 3.f) * nearDist) * m_size}, {x - (sinf(2.f * M_PI / 3.f) * farDist) * m_size, y - (cosf(2.f * M_PI / 3.f) * farDist) * m_size}, {x - (sinf(3.f * M_PI / 3.f) * nearDist) * m_size, y - (cosf(3.f * M_PI / 3.f) * nearDist) * m_size } };
	float2* pts3 = new (float2[4]){ {x, y}, {x - (sinf(-3.f * M_PI / 3.f) * nearDist) * m_size, y - (cosf(-3.f * M_PI / 3.f) * nearDist) * m_size}, {x - (sinf(-2.f * M_PI / 3.f) * farDist) * m_size, y - (cosf(-2.f * M_PI / 3.f) * farDist) * m_size}, {x - (sinf(-M_PI / 3.f) * nearDist) * m_size, y - (cosf(-M_PI / 3.f) * nearDist) * m_size } };

    convex_polygon* convexs = new (convex_polygon[3]){
		{pts1, 4}, {pts2, 4}, {pts3, 4}
	};

	return convexs;
}

FloatingMine::FloatingMine(float2 _pos, Texture2D* texture, Sound explosion, int rank)
	:Mine(_pos, texture, explosion, Mines::FLOATING, rank)
{
//move component
	addComponent(&m_move, true);
	m_move->setSpeed((float)m_rank * 30.f);

	float dx = rand() % 2 == 1 ? -1.f : 1.f, dy = rand() % 2 == 1 ? -1.f : 1.f;
	float2 dir = { dx * (float)(rand() % 100) + 0.1f, dy * (float)(rand() % 100) + 0.1f };

	m_move->setDirection(dir.normalize());
	m_move->setMaxDistance(0.f);

//scoring
	int scoring = rank == 1 ? 100 : 135;
	if (rank >= 3) scoring = 200;
	m_scoring = scoring;

//collider || we allocate memory, and it will be delete in the destructor of concaveCollider

	setSize();

	addComponent(&m_collider, true);
	m_collider->setCollider({ getFloatingConvexs(pos().x, pos().y, m_size), 3 });
	m_collider->setTagAndMask(u64 ColliderTag::MINE, u64 ColliderTag::PLAYER + u64 ColliderTag::PLAYER_FIREBALL);

	m_colliderToAdd = m_collider;
}

void FloatingMine::collisionHappens(Collider& other)
{
	if (other.getTag() == u64 ColliderTag::PLAYER_FIREBALL)
		mineExplose();
}

void FloatingMine::mineExplose()
{
	Mine::mineExplose();
	m_collider->m_isActivate = false;
	m_move->m_isActivate = false;
}

void FloatingMine::draw()
{
	if (m_explosionTimer->m_isActivate)
	{
		Mine::draw();
		return;
	}

	DrawTexturePro(*m_texture, { 0, ATLAS_SPRITE_SIZE, ATLAS_SPRITE_SIZE, ATLAS_SPRITE_SIZE },
		{ pos().x, pos().y, ATLAS_SPRITE_SIZE * m_size, ATLAS_SPRITE_SIZE * m_size },
		{ (ATLAS_SPRITE_SIZE / 2 ) * m_size, (ATLAS_SPRITE_SIZE / 2) * m_size }, rotation(), WHITE);

	rotate(0.1f);

	//AABB and referential
	if (m_showAABB)
	{
		box rect = m_collider->m_AABB;
		DrawRectangleLinesEx({ rect.center.x - rect.width_ha, rect.center.y - rect.height_ha, rect.width_ha * 2, rect.height_ha * 2 }, 1, DARKBLUE);
		drawReferential();

		//draw polygons
		auto& concave = m_collider->m_concave;
		for (int convex = 0; convex < 3; ++convex)
		{
			for (int pt = 0; pt < 3; ++pt)
			{
				DrawLineEx(concave.polygons[convex].pts[pt], concave.polygons[convex].pts[pt + 1], 2.f, DARKBLUE);
			}
		}
	}

}