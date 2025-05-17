#pragma once
#include "BoundingSphere.h"
#include "Collider.h"
#include <vector>

class BVHNode {
public:
    BVHNode* parent;
    BVHNode* left;
    BVHNode* right;
    BoundingSphere volume;
    Collider* collider;

    BVHNode(BVHNode* parent = nullptr);
    ~BVHNode();

    bool isLeaf() const;
    void insert(Collider* newCollider, const BoundingSphere& newVolume);

    void getPotentialContacts(std::vector<std::pair<Collider*, Collider*>>& contacts);
    void recalculateBoundingVolume();
};

