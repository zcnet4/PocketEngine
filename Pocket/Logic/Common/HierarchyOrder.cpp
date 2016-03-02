//
//  HierarchyOrder.h
//  PocketEngine
//
//  Created by Jeppe Nielsen on 10/12/13.
//  Copyright (c) 2013 Jeppe Nielsen. All rights reserved.
//

#include "HierarchyOrder.hpp"

using namespace Pocket;

void HierarchyOrder::Initialize(IGameWorld *world) {
    this->world = world;
}

void HierarchyOrder::ObjectAdded(GameObject *object) {
    object->Parent.Changed.Bind(this, &HierarchyOrder::SetDirty);
    object->Order.Changed.Bind(this, &HierarchyOrder::SetDirty);
    orderIsDirty = true;
}

void HierarchyOrder::ObjectRemoved(GameObject *object) {
   object->Parent.Changed.Unbind(this, &HierarchyOrder::SetDirty);
   object->Order.Changed.Unbind(this, &HierarchyOrder::SetDirty);
}

void HierarchyOrder::SetDirty() {
    orderIsDirty = true;
}

void HierarchyOrder::Update(float dt) {
    if (!orderIsDirty) return;
    orderIsDirty = false;

    int order = 0;
    for (int i=0; i<world->ObjectCount(); ++i) {
        GameObject* object = world->GetObject(i);
        if (object->Parent) continue;
        CalculateOrder(order, object);
    }
}

void HierarchyOrder::CalculateOrder(int& orderOffset, GameObject *object) {
    
    orderOffset++;
    
    Orderable* orderable = object->template GetComponent<Orderable>();
    if (orderable) orderable->Order = orderOffset;
    
    const auto& children = object->Children();
    if (children.empty()) return;
    
    auto sortedChildren = children;
    std::sort(sortedChildren.begin(), sortedChildren.end(), [] (GameObject* a, GameObject* b) {
        return a->Order()<b->Order();
    });
    
    for(auto child : sortedChildren) {
        CalculateOrder(orderOffset, child);
    }
}
