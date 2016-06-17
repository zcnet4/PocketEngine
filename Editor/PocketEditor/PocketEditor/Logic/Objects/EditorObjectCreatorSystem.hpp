//
//  EditorObjectCreatorSystem.hpp
//  PocketEditor
//
//  Created by Jeppe Nielsen on 14/04/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "GameWorld.hpp"
#include "EditorObject.hpp"
#include "Transform.hpp"
#include "Mesh.hpp"
#include "Material.hpp"
#include "Selectable.hpp"

using namespace Pocket;

class EditorObjectCreatorSystem : public GameSystem<EditorObject> {
public:
    void Initialize(GameWorld* world);
    void ObjectAdded(GameObject* object);
    void ObjectRemoved(GameObject* object);
    GameWorld* editorWorld;
    GameObject* gameRoot;
    
private:
    template<typename T>
    struct ComponentSystem : public GameSystem<EditorObject, T> {
        void ObjectAdded(GameObject* object) {
            GameObject* editorObject = (GameObject*)creatorSystem->GetMetaData(object);
            editorObject->AddComponent<T>(object);
        }

        void ObjectRemoved(Pocket::GameObject *object) {
            GameObject* editorObject = (GameObject*)creatorSystem->GetMetaData(object);
            editorObject->RemoveComponent<T>();
        }

        EditorObjectCreatorSystem* creatorSystem;
    };
};