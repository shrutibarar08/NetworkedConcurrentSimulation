#pragma once
#include "BoundingSphere.h"
#include "Collider.h"

class BVHNode
{
public:
    BVHNode(BVHNode* parent, const BoundingSphere& volume, Collider* collider = nullptr);

    bool isLeaf() const;
    void insert(Collider* newCollider, const BoundingSphere& newVolume);

    const BoundingSphere& getVolume() const;
    Collider* getCollider() const;
    BVHNode* getChild(int index) const;

private:
    BVHNode* parent;
    BVHNode* children[2];

    BoundingSphere volume;
    Collider* collider;
};

