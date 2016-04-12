//
//  SelectionSystem.hpp
//  PocketEditor
//
//  Created by Jeppe Nielsen on 06/04/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "GameWorld.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "EditorSelection.hpp"
#include "Touchable.hpp"
#include "EditorSelection.hpp"
#include "Selectable.hpp"
#include "SelectableFactory.hpp"

using namespace Pocket;

class SelectionSystem : public GameSystem<Transform, Mesh, Material> {
public:
    void ObjectAdded(GameObject* object);
    void ObjectRemoved(GameObject* object);
    
    void SetEditorWorld(GameWorld* editorWorld);
    
private:
    GameWorld* editorWorld;
    SelectableFactory* factory;
};