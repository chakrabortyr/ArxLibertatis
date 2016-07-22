/*
 * Copyright 2016 Arx Libertatis Team (see the AUTHORS file)
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

#include "graphics/particle/Spark.h"

#include "core/Core.h"
#include "core/GameTime.h"
#include "graphics/Color.h"
#include "graphics/GraphicsModes.h"
#include "graphics/Math.h"
#include "graphics/RenderBatcher.h"
#include "platform/profiler/Profiler.h"

struct SparkParticle {
	bool exist;
	Vec3f ov;
	Vec3f move;
	Vec3f oldpos;
	long timcreation;
	u32 tolive;
	Color3f rgb;
	float fparam;
	
	SparkParticle()
		: exist(false)
		, ov(Vec3f_ZERO)
		, move(Vec3f_ZERO)
		, timcreation(0)
		, tolive(0)
		, rgb(Color3f::black)
		, fparam(0.f)
	{ }
};

static const size_t g_sparkParticlesMax = 2200;
static SparkParticle g_sparkParticles[g_sparkParticlesMax];
static long g_sparkParticlesCount = 0;

void ParticleSparkClear() {
	std::fill(g_sparkParticles, g_sparkParticles + g_sparkParticlesMax, SparkParticle());
	g_sparkParticlesCount = 0;
}

long ParticleSparkCount() {
	return g_sparkParticlesCount;
}

SparkParticle * createSparkParticle() {
	
	if(arxtime.is_paused()) {
		return NULL;
	}
	
	for(size_t i = 0; i < g_sparkParticlesMax; i++) {
		
		SparkParticle * pd = &g_sparkParticles[i];
		
		if(pd->exist) {
			continue;
		}
		
		g_sparkParticlesCount++;
		pd->exist = true;
		pd->timcreation = arxtime.now();
		
		pd->rgb = Color3f::white;
		pd->move = Vec3f_ZERO;
		
		return pd;
	}
	
	return NULL;
}

static long SPARK_COUNT = 0;

void ARX_PARTICLES_Spawn_Spark(const Vec3f & pos, unsigned int count, SpawnSparkType type) {
	
	if(SPARK_COUNT < 1000) {
		SPARK_COUNT += count * 25;
	} else {
		SPARK_COUNT -= static_cast<long>(g_framedelay);
		return;
	}
	
	for(unsigned int k = 0; k < count; k++) {
		
		SparkParticle * pd = createSparkParticle();
		if(!pd) {
			return;
		}
		
		pd->oldpos = pd->ov = pos + randomVec(-5.f, 5.f);
		pd->move = randomVec(-6.f, 6.f);
		
		unsigned long len = glm::clamp(static_cast<unsigned long>(count * (1.f / 3)), 3ul, 8ul);
		pd->tolive = len * 90 + count;
		
		switch(type) {
			case SpawnSparkType_Default:
				pd->rgb = Color3f(.3f, .3f, 0.f);
				break;
			case SpawnSparkType_Failed:
				pd->rgb = Color3f(.2f, .2f, .1f);
				break;
			case SpawnSparkType_Success:
				pd->rgb = Color3f(.45f, .1f, 0.f);
				break;
		}
		
		pd->fparam = len + Random::getf() * len; // Spark tail length
	}
}

void ParticleSparkUpdate() {
	
	ARX_PROFILE_FUNC();
	
	if(!ACTIVEBKG) {
		return;
	}
	
	if(g_sparkParticlesCount == 0) {
		return;
	}
	
	EERIE_CAMERA * cam = &subj;
	
	const ArxInstant now = arxtime.now();
	
	long pcc = g_sparkParticlesCount;
	
	for(size_t i = 0; i < g_sparkParticlesMax && pcc > 0; i++) {

		SparkParticle * part = &g_sparkParticles[i];
		if(!part->exist) {
			continue;
		}

		long framediff = part->timcreation + part->tolive - now;
		long framediff2 = now - part->timcreation;
		
		if(framediff2 < 0) {
			continue;
		}
		

		EERIE_BKG_INFO * bkgData = getFastBackgroundData(part->ov.x, part->ov.z);

		if(!bkgData || !bkgData->treat) {
			part->exist = false;
			g_sparkParticlesCount--;
			continue;
		}
		
		if(framediff <= 0) {
			part->exist = false;
			g_sparkParticlesCount--;
			continue;
		}
		
		float val = (part->tolive - framediff) * 0.01f;
		
		Vec3f in = part->ov + part->move * val;
		Vec3f inn = in;
		
		TexturedVertex out;
		EE_RTP(inn, out);
		
		if(out.rhw < 0 || out.p.z > cam->cdepth * fZFogEnd) {
			continue;
		}
		
		Vec3f vect = part->oldpos - in;
		vect = glm::normalize(vect);
		TexturedVertex tv[3];
		tv[0].color = part->rgb.toRGB();
		tv[1].color = Color(102, 102, 102, 255).toRGBA();
		tv[2].color = Color(0, 0, 0, 255).toRGBA();
		tv[0].p = out.p;
		tv[0].rhw = out.rhw;
		Vec3f temp;
		temp = in + Vec3f(Random::getf(0.f, 0.5f), 0.8f, Random::getf(0.f, 0.5f));
		EE_RTP(temp, tv[1]);
		temp = in + vect * part->fparam;
		
		EE_RTP(temp, tv[2]);
		
		RenderMaterial mat;
		mat.setBlendType(RenderMaterial::Additive);
		RenderBatcher::getInstance().add(mat, tv);
		
		if(!arxtime.is_paused()) {
			part->oldpos = in;
		}
	}
}