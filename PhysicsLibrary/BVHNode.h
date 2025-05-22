#pragma once
#include "BoundingSphere.h"
#include "Collider.h"
#include <vector>

class BVHNode
{
public:
    BVHNode(BVHNode* parent = nullptr);
    ~BVHNode();

    bool IsLeaf() const;
    void Insert(Collider* newCollider, const BoundingSphere& newVolume);

    void GetPotentialContacts(std::vector<std::pair<Collider*, Collider*>>& contacts) const;
    void RecalculateBoundingVolume();

    //~ Members
    BVHNode* m_Parent;
    BVHNode* m_Left;
    BVHNode* m_Right;
    BoundingSphere m_Volume;
    Collider* m_Collider;
};
