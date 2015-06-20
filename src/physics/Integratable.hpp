// ------------------------------------------------------------------
// physics::Integratable - stardazed
// (c) 2015 by Arthur Langereis
// Based on code by Glenn Fiedler (gafferongames.com)
// ------------------------------------------------------------------

#ifndef SD_PHYSICS_INTEGRATABLE_H
#define SD_PHYSICS_INTEGRATABLE_H

#include "system/Config.hpp"
#include "physics/PhysicsState.hpp"
#include "system/Logging.hpp"

namespace stardazed {
namespace physics {


struct Derivative {
	math::Vec3 velocity;
	math::Vec3 force;

	math::Quat spin;
	math::Vec3 torque;
};


template <typename Impl>
class Integratable {
	Derivative evaluate(const PhysicsState& initial, const Time t) {
		Derivative output;
		output.velocity = initial.velocity();
		output.spin = initial.spin();
		static_cast<Impl*>(this)->calcForces(initial, t, output.force, output.torque);
		return output;
	}

	Derivative evaluate(const PhysicsState& initial, const Time t, const Time dt, const Derivative& derivative) {
		Transform tempTransform{ initial.transform };
		PhysicsState state{ tempTransform, initial.mass(), initial.angularInertia() };
		state.transform.position += derivative.velocity * dt;
		state.momentum += derivative.force * dt;
		state.transform.rotation += derivative.spin * dt;
		state.angularMomentum += derivative.torque * dt;
		state.recalcSecondaryValues();

		Derivative output;
		output.velocity = state.velocity();
		output.spin = state.spin();
		static_cast<Impl*>(this)->calcForces(initial, t + dt, output.force, output.torque);
		return output;
	}

public:
	void integrate(PhysicsState& state, const Time t, const Time dt) {
		Derivative a = evaluate(state, t);
		Derivative b = evaluate(state, t, dt*0.5f, a);
		Derivative c = evaluate(state, t, dt*0.5f, b);
		Derivative d = evaluate(state, t, dt, c);
		
		math::Vec3 dxdt = 1.0f/6.0f * (a.velocity + 2.0f*(b.velocity + c.velocity) + d.velocity);
		math::Vec3 dpdt = 1.0f/6.0f * (a.force + 2.0f*(b.force + c.force) + d.force);
		math::Quat drdt = 1.0f/6.0f * (a.spin + 2.0f*(b.spin + c.spin) + d.spin);
		math::Vec3 dTdt = 1.0f/6.0f * (a.torque + 2.0f*(b.torque + c.torque) + d.torque);

		state.transform.position += dxdt * dt;
		state.momentum += dpdt * dt;
		state.transform.rotation += drdt * dt;
		state.angularMomentum += dTdt * dt;

		state.recalcSecondaryValues();
	}
};


} // ns physics
} // ns stardazed

#endif