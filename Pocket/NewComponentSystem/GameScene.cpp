//
//  GameScene.cpp
//  TestComponentSystem
//
//  Created by Jeppe Nielsen on 18/10/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "GameScene.hpp"
#include "GameWorld.hpp"
#include "StringHelper.hpp"

using namespace Pocket;

GameScene::GameScene() : idCounter(0) { }

GameScene::~GameScene() {
    for (int i=0; i<systemsIndexed.size(); ++i) {
        if (!systemsIndexed[i]) continue;
        if (world->systems[i].deleteFunction) {
            world->systems[i].deleteFunction(systemsIndexed[i]);
        } else {
            delete systemsIndexed[i];
        }
    }
}

GameScene::GameScene(const GameScene& other) {
    activeSystems.clear();
    systemsIndexed.resize(other.world->systems.size(), 0);
    delayedActions.clear();
    
    for (int i=0; i<other.world->systems.size(); ++i) {
        if (other.world->systems[i].createFunction) {
            systemsIndexed[i] = other.world->systems[i].createFunction(root);
            systemsIndexed[i]->Initialize();
        } 
    }
}

void GameScene::DestroySystems() {
    for (int i=0; i<world->systems.size(); ++i) {
        if (systemsIndexed[i]) {
            systemsIndexed[i]->Destroy();
        }
    }
}

void GameScene::DoActions(Actions &actions) {
    for(int i=0; i<actions.size();++i) {
        actions[i]();
    }
    actions.clear();
}

void GameScene::Update(float dt) {
    DoActions(delayedActions);
    for(auto system : activeSystems) {
        system->Update(dt);
    }
}

void GameScene::Render() {
    for(auto system : activeSystems) {
        system->Render();
    }
}

GameObject* GameScene::FindObject(int objectId) {
    std::vector<GameObject*> nodesToVisit;
    nodesToVisit.push_back(root);
    
    while (!nodesToVisit.empty()) {
        GameObject* current = nodesToVisit.back();
        if (current->rootId == objectId) return current;
        nodesToVisit.pop_back();
        nodesToVisit.insert(nodesToVisit.begin(), current->Children().begin(), current->Children().end());
    }
    
    return 0;
}