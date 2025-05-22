#include "pch.h"
#include "BVHNode.h"

BVHNode::BVHNode(BVHNode* parent)
    : m_Parent(parent), m_Left(nullptr),
	m_Right(nullptr), m_Collider(nullptr)
{}

BVHNode::~BVHNode()
{
    delete m_Left;
    delete m_Right;
}

bool BVHNode::IsLeaf() const
{
    return (m_Collider != nullptr);
}

void BVHNode::Insert(Collider* newCollider,
    const BoundingSphere& newVolume)
{
    if (IsLeaf())
    {
        BVHNode* oldLeaf = new BVHNode(this);
        oldLeaf->m_Volume = this->m_Volume;
        oldLeaf->m_Collider = this->m_Collider;

        BVHNode* newLeaf = new BVHNode(this);
        newLeaf->m_Volume = newVolume;
        newLeaf->m_Collider = newCollider;

        this->m_Collider = nullptr;
        this->m_Left = oldLeaf;
        this->m_Right = newLeaf;
        this->RecalculateBoundingVolume();
    }
    else 
    {
        float growthLeft = m_Left->m_Volume.GetGrowth(newVolume);
        float growthRight = m_Right->m_Volume.GetGrowth(newVolume);
        if (growthLeft < growthRight) m_Left->Insert(newCollider, newVolume);
        else m_Right->Insert(newCollider, newVolume);
        this->RecalculateBoundingVolume();
    }
}

void BVHNode::RecalculateBoundingVolume()
{
    if (IsLeaf()) return;
    m_Volume = BoundingSphere::Merge(m_Left->m_Volume, m_Right->m_Volume);
}

void BVHNode::GetPotentialContacts(std::vector<std::pair<Collider*, Collider*>>& contacts) const
{
    if (IsLeaf()) return;

    if (m_Left->IsLeaf() && m_Right->IsLeaf())
    {
        if (m_Left->m_Volume.Overlaps(m_Right->m_Volume))
        {
            contacts.emplace_back(m_Left->m_Collider,
                m_Right->m_Collider);
        }
    }
    else 
    {
        if (m_Left) m_Left->GetPotentialContacts(contacts);
        if (m_Right) m_Right->GetPotentialContacts(contacts);
    }
}
