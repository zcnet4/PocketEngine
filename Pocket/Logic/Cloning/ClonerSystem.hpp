//
//  ClonerSystem.hpp
//  PocketEditor
//
//  Created by Jeppe Nielsen on 17/09/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "GameSystem.hpp"
#include "Cloner.hpp"

namespace Pocket {
    class ClonerSystem : public GameSystem<Cloner> {
    protected:
        void ObjectAdded(GameObject* object);
        void ObjectRemoved(GameObject* object);
    private:
        void ClonePathChanged(GameObject* object);
    };
}