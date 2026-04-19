#pragma once

#include "drillengine/DrillLib.h"
#include "BeeTasks.h"
#include "TileSpace.h"

namespace Bee {

using namespace BeeTasks;
using namespace TileSpace;

static constexpr F32 DEFAULT_SPEED = 10.0F;

class Bee {
public:
	static constexpr F32 arrivalEpsilon = 0.01F;
	static constexpr F32 flightWobbleFrequency = 0.45F;
	static constexpr F32 flightWobbleAmplitude = 5.0F;

	V2F32 position{};
	V2F32 velocity{};
	V2U32 homeTile{};
	V2F32 homeOffsetWorld{ 0.5F, 0.5F };
	F32 moveSpeed = DEFAULT_SPEED;
	F32 workTimerSeconds = 0.0F;
	F32 travelTimerSeconds = 0.0F;
	F32 flightPhaseTurns = 0.0F;
	State state = State::STATE_IDLE;
	Task activeTask{};
	B32 hasTask = B32_FALSE;

	Bee() = default;
	Bee(V2U32 spawnHomeTile, F32 speed = DEFAULT_SPEED)
		: homeTile(spawnHomeTile), moveSpeed(speed) {
		position = home_world_position();
		flightPhaseTurns = initial_flight_phase(spawnHomeTile);
	}

	Bee(V2U32 spawnHomeTile, V2F32 spawnHomeOffsetWorld, F32 speed)
		: homeTile(spawnHomeTile), homeOffsetWorld(spawnHomeOffsetWorld), moveSpeed(speed) {
		position = home_world_position();
		flightPhaseTurns = initial_flight_phase(spawnHomeTile);
	}

	void set_home(V2U32 newHomeTile) {
		homeTile = newHomeTile;
	}

	void set_home_anchor(V2U32 newHomeTile, V2F32 newHomeOffsetWorld) {
		homeTile = newHomeTile;
		homeOffsetWorld = newHomeOffsetWorld;
	}

	void teleport(V2F32 newPosition) {
		position = newPosition;
		velocity = V2F32{};
	}

	void snap_to_home() {
		position = home_world_position();
		velocity = V2F32{};
	}

	B32 busy() const {
		return state != State::STATE_IDLE || hasTask;
	}

	B32 idle() const {
		return !busy();
	}

	B32 inside_hive() const {
		return state == State::STATE_IDLE && hasTask == B32_FALSE && is_at(home_world_position()) ? B32_TRUE : B32_FALSE;
	}

	F32 work_progress01() const {
		if (state != State::STATE_WORKING || !hasTask || activeTask.workDurationSeconds <= 0.0F) {
			return 0.0F;
		}
		return clamp01(workTimerSeconds / activeTask.workDurationSeconds);
	}

	void assign_task(const Task& task) {
		activeTask = task;
		hasTask = task_is_valid(task);
		workTimerSeconds = 0.0F;
		travelTimerSeconds = 0.0F;
		velocity = V2F32{};
		state = hasTask ? State::STATE_TRAVEL_TO_TARGET : State::STATE_IDLE;
	}

	void cancel_task() {
		activeTask = Task{};
		hasTask = B32_FALSE;
		workTimerSeconds = 0.0F;
		travelTimerSeconds = 0.0F;
		velocity = V2F32{};
		state = is_at(home_world_position()) ? State::STATE_IDLE : State::STATE_TRAVEL_HOME;
	}

	UpdateResult update(F32 dtSeconds) {
		UpdateResult result{};
		F32 dt = max(dtSeconds, 0.0F);

		switch (state) {
		case State::STATE_IDLE: {
			velocity = V2F32{};
		} break;

		case State::STATE_TRAVEL_TO_TARGET: {
			if (!hasTask) {
				state = State::STATE_TRAVEL_HOME;
				break;
			}

			V2F32 targetPosition = task_world_position(activeTask);
			move_toward_flying(targetPosition, dt);
			if (is_at(targetPosition)) {
				position = targetPosition;
				velocity = V2F32{};
				state = State::STATE_WORKING;
				workTimerSeconds = 0.0F;
				result.reachedTarget = B32_TRUE;
				result.startedWorking = B32_TRUE;

				if (activeTask.workDurationSeconds <= 0.0F) {
					finish_work_cycle(&result);
				}
			}
		} break;

		case State::STATE_WORKING: {
			if (!hasTask) {
				state = State::STATE_TRAVEL_HOME;
				velocity = V2F32{};
				break;
			}

			velocity = V2F32{};
			workTimerSeconds += dt;
			if (workTimerSeconds >= activeTask.workDurationSeconds) {
				finish_work_cycle(&result);
			}
		} break;

		case State::STATE_TRAVEL_HOME: {
			V2F32 homePosition = home_world_position();
			move_toward_flying(homePosition, dt);
			if (is_at(homePosition)) {
				position = homePosition;
				velocity = V2F32{};
				result.reachedHome = B32_TRUE;

				if (hasTask && activeTask.persistent) {
					state = State::STATE_TRAVEL_TO_TARGET;
				}
				else {
					state = State::STATE_IDLE;
					activeTask = Task{};
					hasTask = B32_FALSE;
					result.taskFinished = B32_TRUE;
				}
			}
		} break;
		}

		return result;
	}

	V2F32 home_world_position() const {
		return TileSpace::tile_to_world(homeTile) + homeOffsetWorld;
	}

	V2F32 task_world_position(const Task& task) const {
		return TileSpace::tile_to_world_center(task.targetTile);
	}

private:
	static F32 initial_flight_phase(V2U32 tile) {
		U32 hash = tile.x * 747796405u + tile.y * 2891336453u + 277803737u;
		hash ^= hash >> 16;
		hash *= 2246822519u;
		hash ^= hash >> 13;
		return F32(hash & 1023u) * (1.0F / 1024.0F);
	}

	B32 is_at(V2F32 target) const {
		return distance_sq(position, target) <= arrivalEpsilon * arrivalEpsilon;
	}

	void move_toward_flying(V2F32 target, F32 dt) {
		V2F32 delta = target - position;
		F32 distSq = length_sq(delta);
		if (distSq <= arrivalEpsilon * arrivalEpsilon) {
			position = target;
			velocity = V2F32{};
			return;
		}

		F32 dist = sqrtf32(distSq);
		V2F32 forward = delta / dist;
		V2F32 side = get_orthogonal(forward);
		travelTimerSeconds += dt;

		F32 wobbleTurns = travelTimerSeconds * flightWobbleFrequency + flightPhaseTurns;
		F32 wobble = sinf32(wobbleTurns) + 0.35F * sinf32(wobbleTurns * 1.89F + 0.17F);
		wobble *= 0.74F;

		F32 wobbleScale = min(flightWobbleAmplitude, dist * 0.85F);
		wobbleScale *= 0.35F + 0.65F * clamp01(dist / 1.5F);

		V2F32 desiredPosition = target + side * (wobble * wobbleScale);
		V2F32 desiredDelta = desiredPosition - position;
		F32 desiredDistSq = length_sq(desiredDelta);
		if (desiredDistSq <= arrivalEpsilon * arrivalEpsilon) {
			position = target;
			velocity = V2F32{};
			return;
		}

		F32 maxStep = moveSpeed * dt;
		if (desiredDistSq <= maxStep * maxStep) {
			velocity = dt > 0.0F ? desiredDelta / dt : V2F32{};
			position = desiredPosition;
		}
		else {
			V2F32 desiredDirection = desiredDelta / sqrtf32(desiredDistSq);
			velocity = desiredDirection * moveSpeed;
			position += velocity * dt;
		}

		if (distance_sq(position, target) <= arrivalEpsilon * arrivalEpsilon) {
			position = target;
			velocity = V2F32{};
		}
	}

	void finish_work_cycle(UpdateResult* result) {
		result->finishedWork = B32_TRUE;
		workTimerSeconds = 0.0F;

		if (activeTask.returnHomeAfterWork) {
			state = State::STATE_TRAVEL_HOME;
		}
		else if (activeTask.persistent) {
			state = State::STATE_WORKING;
		}
		else {
			state = State::STATE_IDLE;
			activeTask = Task{};
			hasTask = B32_FALSE;
			result->taskFinished = B32_TRUE;
		}
	}
};

}
