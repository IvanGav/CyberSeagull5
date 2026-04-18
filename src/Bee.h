#pragma once

#include "drillengine/DrillLib.h"
#include "BeeTasks.h"
#include "TileSpace.h"


namespace Bee {

static constexpr F32 DEFAULT_SPEED = 10.0F;

class Bee {
public:
	static constexpr F32 arrivalEpsilon = 0.01F;

	V2F32 position{};
	V2F32 velocity{};
	V2U32 homeTile{};
	F32 moveSpeed = DEFAULT_SPEED;
	F32 workTimerSeconds = 0.0F;
	State state = State::STATE_IDLE;
	Task activeTask{};
	B32 hasTask = B32_FALSE;

	Bee() = default;
	Bee(V2U32 spawnHomeTile, F32 speed = DEFAULT_SPEED)
		: homeTile(spawnHomeTile), moveSpeed(speed) {
		position = home_world_position();
	}

	void set_home(V2U32 newHomeTile) {
		homeTile = newHomeTile;
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

	void assign_task(const Task& task) {
		activeTask = task;
		hasTask = task.type != TaskType::TASK_NONE;
		workTimerSeconds = 0.0F;
		velocity = V2F32{};
		state = hasTask ? State::STATE_TRAVEL_TO_TARGET : State::STATE_IDLE;
	}

	void cancel_task() {
		activeTask = Task{};
		hasTask = B32_FALSE;
		workTimerSeconds = 0.0F;
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
			move_toward(targetPosition, dt);
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
			move_toward(homePosition, dt);
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
		return World::tile_space().tile_to_world_center(homeTile);
	}

	V2F32 task_world_position(const Task& task) const {
		return World::tile_space().tile_to_world_center(task.targetTile);
	}

private:
	B32 is_at(V2F32 target) const {
		return distance_sq(position, target) <= arrivalEpsilon * arrivalEpsilon;
	}

	void move_toward(V2F32 target, F32 dt) {
		V2F32 delta = target - position;
		F32 distSq = length_sq(delta);
		if (distSq <= arrivalEpsilon * arrivalEpsilon) {
			position = target;
			velocity = V2F32{};
			return;
		}

		F32 maxStep = moveSpeed * dt;
		if (distSq <= maxStep * maxStep) {
			velocity = dt > 0.0F ? delta / dt : V2F32{};
			position = target;
			return;
		}

		V2F32 direction = delta / sqrtf32(distSq);
		velocity = direction * moveSpeed;
		position += velocity * dt;
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
