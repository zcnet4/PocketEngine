//
//  ParticleEmitter.cpp
//  PocketEngine
//
//  Created by Jeppe Nielsen on 9/14/13.
//  Copyright (c) 2013 Jeppe Nielsen. All rights reserved.
//

#include "ParticleEmitter.hpp"

using namespace Pocket;

void ParticleEmitter::Reset() {
    UseWorldSpace = false;
    Playing = false;
    Age = 0;
    frequencyTime = 0;
}

void ParticleEmitter::Clone(const Pocket::ParticleEmitter &source) {
    UseWorldSpace = source.UseWorldSpace;
    Playing = source.Playing;
    Age = source.Age;
    frequencyTime = source.frequencyTime;
}

ParticleEmitter::ParticleEmitter() : UseWorldSpace(this), Playing(this), Age(this) {}

void ParticleEmitter::CreateParticles(int amount) {
    particles.resize(amount);
    for (size_t i=0; i<particles.size(); ++i) {
        inactiveParticles.push_back(&particles[i]);
    }
}
