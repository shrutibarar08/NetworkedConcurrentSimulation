#include "pch.h"
#include "BVHNode.h"

BVHNode::BVHNode(BVHNode* parent, const BoundingSphere& volume, Collider* collider)
	: parent(parent), volume(volume), collider(collider) {
	children[0] = nullptr;
	children[1] = nullptr;
}

bool BVHNode::isLeaf() const
{
	return collider != nullptr;
}

void BVHNode::insert(Collider* newCollider, const BoundingSphere& newVolume)
{
    if (isLeaf()) {
        children[0] = new BVHNode(this, volume, collider);
        children[1] = new BVHNode(this, newVolume, newCollider);
        collider = nullptr;
        volume = BoundingSphere::merge(children[0]->volume, children[1]->volume);
    }
    else {
        // Insert into child that needs least volume increase
        float growth0 = BoundingSphere::merge(children[0]->volume, newVolume).getSize() - children[0]->volume.getSize();
        float growth1 = BoundingSphere::merge(children[1]->volume, newVolume).getSize() - children[1]->volume.getSize();

        if (growth0 < growth1) {
            children[0]->insert(newCollider, newVolume);
        }
        else {
            children[1]->insert(newCollider, newVolume);
        }

        volume = BoundingSphere::merge(children[0]->volume, children[1]->volume);
    }
}

const BoundingSphere& BVHNode::getVolume() const
{
	return volume;
}

Collider* BVHNode::getCollider() const
{
	return collider;
}

BVHNode* BVHNode::getChild(int index) const
{
	//assert(index == 0 || index == 1);
	return children[index];
}
