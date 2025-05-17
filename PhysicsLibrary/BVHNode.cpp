#include "pch.h"
#include "BVHNode.h"

BVHNode::BVHNode(BVHNode* parent)
    : parent(parent), left(nullptr), right(nullptr), collider(nullptr) {
}

BVHNode::~BVHNode() {
    delete left;
    delete right;
}

bool BVHNode::isLeaf() const {
    return (collider != nullptr);
}

void BVHNode::insert(Collider* newCollider, const BoundingSphere& newVolume) {
    if (isLeaf()) {
        BVHNode* oldLeaf = new BVHNode(this);
        oldLeaf->volume = this->volume;
        oldLeaf->collider = this->collider;

        BVHNode* newLeaf = new BVHNode(this);
        newLeaf->volume = newVolume;
        newLeaf->collider = newCollider;

        this->collider = nullptr;
        this->left = oldLeaf;
        this->right = newLeaf;
        this->recalculateBoundingVolume();
    }
    else {
        float growthLeft = left->volume.getGrowth(newVolume);
        float growthRight = right->volume.getGrowth(newVolume);
        if (growthLeft < growthRight) left->insert(newCollider, newVolume);
        else right->insert(newCollider, newVolume);
        this->recalculateBoundingVolume();
    }
}

void BVHNode::recalculateBoundingVolume() {
    if (isLeaf()) return;
    volume = BoundingSphere::merge(left->volume, right->volume);
}

void BVHNode::getPotentialContacts(std::vector<std::pair<Collider*, Collider*>>& contacts) {
    if (isLeaf()) return;

    if (left->isLeaf() && right->isLeaf()) {
        if (left->volume.overlaps(right->volume)) {
            contacts.emplace_back(left->collider, right->collider);
        }
    }
    else {
        if (left) left->getPotentialContacts(contacts);
        if (right) right->getPotentialContacts(contacts);
    }
}
