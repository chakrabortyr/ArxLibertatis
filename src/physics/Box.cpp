/*
 * Copyright 2011-2016 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */
/* Based on:
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
// Code: Cyril Meynier
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved

#include "physics/Box.h"

#include "graphics/Math.h"


// Used to launch an object into the physical world...
void EERIE_PHYSICS_BOX_Launch(EERIE_3DOBJ * obj, const Vec3f & pos, const Anglef & angle, const Vec3f & vect)
{
	arx_assert(obj);
	arx_assert(obj->pbox);
	
	float ratio = obj->pbox->surface * ( 1.0f / 10000 );
	ratio = glm::clamp(ratio, 0.f, 0.8f);
	ratio = 1.f - (ratio * ( 1.0f / 4 ));
	
	for(size_t i = 0; i < obj->pbox->vert.size(); i++) {
		PHYSVERT * pv = &obj->pbox->vert[i];
		pv->pos = pv->initpos;
		pv->pos = VRotateY(pv->pos, angle.getYaw());
		pv->pos = VRotateX(pv->pos, angle.getPitch());
		pv->pos = VRotateZ(pv->pos, angle.getRoll());
		pv->pos += pos;

		pv->force = Vec3f_ZERO;
		pv->velocity = vect * (250.f * ratio);
		pv->mass = 0.4f + ratio * 0.1f;
	}
	
	obj->pbox->active = 1;
	obj->pbox->stopcount = 0;
	obj->pbox->storedtiming = 0;
}

// Checks is a triangle of a physical object is colliding a triangle
bool IsObjectVertexCollidingTriangle(const PHYSICS_BOX_DATA & pbox, Vec3f * verts)
{
	EERIE_TRI t1, t2;
	bool ret = false;
	std::copy(verts, verts + 2, t2.v);

	const boost::array<PHYSVERT, 15> & vert = pbox.vert;

	Vec3f center = (verts[0] + verts[1] + verts[2]) * ( 1.0f / 3 );
	float rad = fdist(center, verts[0]);

	{
		size_t nn = 0;

		for (; nn < pbox.vert.size(); nn++)
		{
			if(!fartherThan(center, vert[nn].pos, std::max(60.0f, rad + 25))) {
				nn = 1000;
			}
		}

		if (nn < 1000)
			return false;
	}
	
	//TOP
	t1.v[0] = vert[1].pos;
	t1.v[1] = vert[2].pos;
	t1.v[2] = vert[3].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	//BOTTOM
	t1.v[0] = vert[10].pos;
	t1.v[1] = vert[9].pos;
	t1.v[2] = vert[11].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	//UP/FRONT
	t1.v[0] = vert[1].pos;
	t1.v[1] = vert[4].pos;
	t1.v[2] = vert[5].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	//DOWN/FRONT
	t1.v[0] = vert[5].pos;
	t1.v[1] = vert[8].pos;
	t1.v[2] = vert[9].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	//UP/BACK
	t1.v[0] = vert[3].pos;
	t1.v[1] = vert[2].pos;
	t1.v[2] = vert[7].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}
	
	//DOWN/BACK
	t1.v[0] = vert[7].pos;
	t1.v[1] = vert[6].pos;
	t1.v[2] = vert[11].pos;
	
	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	//UP/LEFT
	t1.v[0] = vert[6].pos;
	t1.v[1] = vert[2].pos;
	t1.v[2] = vert[1].pos;

	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	//DOWN/LEFT
	t1.v[0] = vert[10].pos;
	t1.v[1] = vert[6].pos;
	t1.v[2] = vert[5].pos;

	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	//UP/RIGHT
	t1.v[0] = vert[4].pos;
	t1.v[1] = vert[3].pos;
	t1.v[2] = vert[7].pos;

	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	//DOWN/RIGHT
	t1.v[0] = vert[8].pos;
	t1.v[1] = vert[7].pos;
	t1.v[2] = vert[11].pos;

	if(Triangles_Intersect(t1, t2)) {
		return true;
	}

	return ret;
}

// Releases physic box data from an object
void EERIE_PHYSICS_BOX_Release(EERIE_3DOBJ * obj) {
	
	if(!obj || !obj->pbox) {
		return;
	}
	
	delete obj->pbox;
	obj->pbox = NULL;
}

// Creation of the physics box... quite cabalistic and extensive func...
// Need to put a (really) smarter algorithm in there...
void EERIE_PHYSICS_BOX_Create(EERIE_3DOBJ * obj)
{
	if (!obj) return;

	EERIE_PHYSICS_BOX_Release(obj);

	if (obj->vertexlist.empty()) return;

	PHYSICS_BOX_DATA * pbox = new PHYSICS_BOX_DATA();
	
	pbox->stopcount = 0;
	
	Vec3f cubmin = Vec3f(std::numeric_limits<float>::max());
	Vec3f cubmax = Vec3f(-std::numeric_limits<float>::max());
	
	for(size_t k = 0; k < obj->vertexlist.size(); k++) {
		if(k != obj->origin) {
			cubmin = glm::min(cubmin, obj->vertexlist[k].v);
			cubmax = glm::max(cubmax, obj->vertexlist[k].v);
		}
	}
	
	pbox->vert[0].pos = cubmin + (cubmax - cubmin) * .5f;
	pbox->vert[13].pos = pbox->vert[0].pos;
	pbox->vert[13].pos.y = cubmin.y;
	pbox->vert[14].pos = pbox->vert[0].pos;
	pbox->vert[14].pos.y = cubmax.y;
	
	for(size_t k = 1; k < pbox->vert.size() - 2; k++)
	{
		pbox->vert[k].pos.x = pbox->vert[0].pos.x;
		pbox->vert[k].pos.z = pbox->vert[0].pos.z;

		if (k < 5)		pbox->vert[k].pos.y = cubmin.y;
		else if (k < 9)	pbox->vert[k].pos.y = pbox->vert[0].pos.y;
		else			pbox->vert[k].pos.y = cubmax.y;
	}

	float diff = cubmax.y - cubmin.y;

	if (diff < 12.f) 
	{
		cubmax.y += 8.f; 
		cubmin.y -= 8.f; 

		for(size_t k = 1; k < pbox->vert.size() - 2; k++)
		{
			pbox->vert[k].pos.x = pbox->vert[0].pos.x;
			pbox->vert[k].pos.z = pbox->vert[0].pos.z;

			if (k < 5)		pbox->vert[k].pos.y = cubmin.y;
			else if (k < 9)	pbox->vert[k].pos.y = pbox->vert[0].pos.y;
			else			pbox->vert[k].pos.y = cubmax.y;
		}

		pbox->vert[14].pos.y = cubmax.y;
		pbox->vert[13].pos.y = cubmin.y;
		float RATI = diff * ( 1.0f / 8 );

		for (size_t k = 0; k < obj->vertexlist.size(); k++)
		{
			if (k == obj->origin) continue;

			Vec3f curr = obj->vertexlist[k].v;
			size_t SEC = 1;
			pbox->vert[SEC].pos.x = std::min(pbox->vert[SEC].pos.x, curr.x);
			pbox->vert[SEC].pos.z = std::min(pbox->vert[SEC].pos.z, curr.z);

			pbox->vert[SEC+1].pos.x = std::min(pbox->vert[SEC+1].pos.x, curr.x);
			pbox->vert[SEC+1].pos.z = std::max(pbox->vert[SEC+1].pos.z, curr.z);

			pbox->vert[SEC+2].pos.x = std::max(pbox->vert[SEC+2].pos.x, curr.x);
			pbox->vert[SEC+2].pos.z = std::max(pbox->vert[SEC+2].pos.z, curr.z);

			pbox->vert[SEC+3].pos.x = std::max(pbox->vert[SEC+3].pos.x, curr.x);
			pbox->vert[SEC+3].pos.z = std::min(pbox->vert[SEC+3].pos.z, curr.z);

			SEC = 5;
			pbox->vert[SEC].pos.x = std::min(pbox->vert[SEC].pos.x, curr.x - RATI);
			pbox->vert[SEC].pos.z = std::min(pbox->vert[SEC].pos.z, curr.z - RATI);

			pbox->vert[SEC+1].pos.x = std::min(pbox->vert[SEC+1].pos.x, curr.x - RATI);
			pbox->vert[SEC+1].pos.z = std::max(pbox->vert[SEC+1].pos.z, curr.z + RATI);

			pbox->vert[SEC+2].pos.x = std::max(pbox->vert[SEC+2].pos.x, curr.x + RATI);
			pbox->vert[SEC+2].pos.z = std::max(pbox->vert[SEC+2].pos.z, curr.z + RATI);

			pbox->vert[SEC+3].pos.x = std::max(pbox->vert[SEC+3].pos.x, curr.x + RATI);
			pbox->vert[SEC+3].pos.z = std::min(pbox->vert[SEC+3].pos.z, curr.z - RATI);


			SEC = 9;
			pbox->vert[SEC].pos.x = std::min(pbox->vert[SEC].pos.x, curr.x);
			pbox->vert[SEC].pos.z = std::min(pbox->vert[SEC].pos.z, curr.z);

			pbox->vert[SEC+1].pos.x = std::min(pbox->vert[SEC+1].pos.x, curr.x);
			pbox->vert[SEC+1].pos.z = std::max(pbox->vert[SEC+1].pos.z, curr.z);

			pbox->vert[SEC+2].pos.x = std::max(pbox->vert[SEC+2].pos.x, curr.x);
			pbox->vert[SEC+2].pos.z = std::max(pbox->vert[SEC+2].pos.z, curr.z);

			pbox->vert[SEC+3].pos.x = std::max(pbox->vert[SEC+3].pos.x, curr.x);
			pbox->vert[SEC+3].pos.z = std::min(pbox->vert[SEC+3].pos.z, curr.z);
		}
	}
	else
	{
		float cut = (cubmax.y - cubmin.y) * ( 1.0f / 3 );
		float ysec2 = cubmin.y + cut * 2.f;
		float ysec1 = cubmin.y + cut;

		for (size_t k = 0; k < obj->vertexlist.size(); k++)
		{
			if (k == obj->origin) continue;

			Vec3f curr = obj->vertexlist[k].v;
			size_t SEC;

			if (curr.y < ysec1)
			{
				SEC = 1;
			}
			else if (curr.y < ysec2)
			{
				SEC = 5;
			}
			else
			{
				SEC = 9;
			}

			pbox->vert[SEC].pos.x = std::min(pbox->vert[SEC].pos.x, curr.x);
			pbox->vert[SEC].pos.z = std::min(pbox->vert[SEC].pos.z, curr.z);

			pbox->vert[SEC+1].pos.x = std::min(pbox->vert[SEC+1].pos.x, curr.x);
			pbox->vert[SEC+1].pos.z = std::max(pbox->vert[SEC+1].pos.z, curr.z);

			pbox->vert[SEC+2].pos.x = std::max(pbox->vert[SEC+2].pos.x, curr.x);
			pbox->vert[SEC+2].pos.z = std::max(pbox->vert[SEC+2].pos.z, curr.z);

			pbox->vert[SEC+3].pos.x = std::max(pbox->vert[SEC+3].pos.x, curr.x);
			pbox->vert[SEC+3].pos.z = std::min(pbox->vert[SEC+3].pos.z, curr.z);
		}
	}

	for (size_t k = 0; k < 4; k++)
	{
		if (glm::abs(pbox->vert[5+k].pos.x - pbox->vert[0].pos.x) < 2.f)
			pbox->vert[5+k].pos.x = (pbox->vert[1+k].pos.x + pbox->vert[9+k].pos.x) * .5f;

		if (glm::abs(pbox->vert[5+k].pos.z - pbox->vert[0].pos.z) < 2.f)
			pbox->vert[5+k].pos.z = (pbox->vert[1+k].pos.z + pbox->vert[9+k].pos.z) * .5f;
	}

	pbox->radius = 0.f;

	for(size_t k = 0; k < pbox->vert.size(); k++) {
		PHYSVERT & pv = pbox->vert[k];
		
		float distt = glm::distance(pv.pos, pbox->vert[0].pos);
		
		if(distt > 20.f) {
			pv.pos.x = (pv.pos.x - pbox->vert[0].pos.x) * 0.5f + pbox->vert[0].pos.x;
			pv.pos.z = (pv.pos.z - pbox->vert[0].pos.z) * 0.5f + pbox->vert[0].pos.z;
		}
		
		pv.initpos = pv.pos;
		
		if(k != 0) {
			float d = glm::distance(pbox->vert[0].pos, pv.pos);
			pbox->radius = std::max(pbox->radius, d);
		}
	}
	
	float surface = 0.f;
	for(size_t i = 0; i < obj->facelist.size(); i++) {
		const Vec3f & p0 = obj->vertexlist[obj->facelist[i].vid[0]].v;
		const Vec3f & p1 = obj->vertexlist[obj->facelist[i].vid[1]].v;
		const Vec3f & p2 = obj->vertexlist[obj->facelist[i].vid[2]].v;
		surface += glm::distance((p0 + p1) * .5f, p2) * glm::distance(p0, p1) * .5f;
	}
	pbox->surface = surface;
	
	obj->pbox = pbox;
}
