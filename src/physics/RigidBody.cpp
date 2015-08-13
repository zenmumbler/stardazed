// ------------------------------------------------------------------
// physics::RigidBody.cpp - stardazed
// (c) 2015 by Arthur Langereis
// ------------------------------------------------------------------

#include "physics/RigidBody.hpp"

namespace stardazed {
namespace physics {


RigidBodyManager::RigidBodyManager(memory::Allocator& allocator, scene::TransformComponent& transform)
: transformMgr_(transform)
, instanceData_{ allocator, 1024 }
, entityMap_{ allocator, 1024 }
{}


auto RigidBodyManager::create(scene::Entity entity, float mass, float angularInertia) -> Instance {
	instanceData_.extend();
	uint index = instanceData_.count() - 1;

	*(basePtr<InstField::Mass>() + index) = mass;
	*(basePtr<InstField::AngularInertia>() + index) = angularInertia;

	Instance h { index };
	entityMap_.insert(entity, h);
	return h;
}


void RigidBodyManager::addForce(Instance h, const math::Vec3& force) {
	auto fp = instancePtr<InstField::CurrentForce>(h);
	*fp += force;
}


void RigidBodyManager::addTorque(Instance h, const math::Vec3& torque) {
	auto tp = instancePtr<InstField::CurrentTorque>(h);
	*tp += torque;
}


void RigidBodyManager::recalcSecondaries(Instance h) {
	velocity(h) = momentum(h) * inverseMass(h);
	angularVelocity(h) = angularMomentum(h) * inverseAngInertia(h);
	
	auto transInst = linkedTransform(h);
	auto rotation = transformMgr_.rotation(transInst);
	transformMgr_.setRotation(transInst, math::normalize(rotation));
	spin(h) = 0.5f * math::Quat{angularVelocity(h), 0} * rotation;
}


void RigidBodyManager::integrateAll(Time dt) {
	auto transformBase = basePtr<InstField::Transform>();
	auto invMassBase = basePtr<InstField::InverseMass>();
	auto velocityBase = basePtr<InstField::Velocity>();

/*
	last_acceleration = acceleration
	position += velocity * time_step + ( 0.5 * last_acceleration * time_step^2 )
	new_acceleration = force / mass
	avg_acceleration = ( last_acceleration + new_acceleration ) / 2
	velocity += avg_acceleration * time_step
*/
//	auto curAccel =
}


/*
	
	
	void PhysicsState::copyPrimaryAndSecondaryValuesFrom(const PhysicsState& other) {
		transform = other.transform;
		momentum = other.momentum;
		angularMomentum = other.angularMomentum;
		
		velocity_ = other.velocity_;
		angularVelocity_ = other.angularVelocity_;
		spin_ = other.spin_;
	}


	RigidBody::RigidBody(Transform& linkedTransform, float mass, float angularInertia)
	: previousTransform_(linkedTransform)
	, previous_{ previousTransform_, mass, angularInertia }
	, current_ { linkedTransform, mass, angularInertia }
	{}


	void RigidBody::update(Time t, Time dt) {
		previous_.copyPrimaryAndSecondaryValuesFrom(current_);
		integrate(current_, t, dt);
		userForce = {0, 0, 0};
		userTorque = {0, 0, 0};
	}


	void RigidBody::calcForces(const PhysicsState& state, const Time globalTime, math::Vec3& outForce, math::Vec3& outTorque) const {
		outForce  = userForce - state.momentum * .5;
		outTorque = userTorque - state.angularMomentum * .5;
	}
*/


} // ns physics
} // ns stardazed
