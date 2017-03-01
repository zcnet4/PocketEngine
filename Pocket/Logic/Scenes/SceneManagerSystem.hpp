//
//  SceneLayerSystem.hpp
//  PocketEditor
//
//  Created by Jeppe Nielsen on 25/02/2017.
//  Copyright © 2017 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "SceneManager.hpp"
#include "GameSystem.hpp"

namespace Pocket {
    class SceneManagerSystem : public GameSystem<SceneManager> {
    public:
        void ObjectAdded(GameObject* object);
        void ObjectRemoved(GameObject* object);
    private:
        void ActiveScenesChanged(GameObject* object);
    };
}
