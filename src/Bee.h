#pragma once

#include "drillengine/DrillLib.h"
#include <math.h>

namespace Bee {

	enum class TaskType {
		None,
		Gather,
		Mine,
		Pollinate,
		Build,
		MoveOnly,
	};

	enum class State {
		Idle,
		FlyingToTask,
		Working,
		ReturningHome,
	};

	struct Task {
		TaskType type = TaskType::None;
		F32 targetX = 0.0F;
		F32 targetY = 0.0F;
		F32 workDurationSeconds = 0.0F;

		// Persistent tasks keep the bee assigned until cancelled.
		// This is for uncompleteable tasks like miningg	
		bool persistent = false;

		// If true, the bee returns to its hive after each work cycle.
		// If false, the bee stays on site and then starts the next cycle.
		// Just here for like conveyor mining(false if conveyor mining).
		bool returnHomeAfterWork = true;
	};

	struct UpdateResult {
		bool reachedTask = false;
		bool finishedWorkCycle = false;
		bool deliveredItem = false;
		bool taskFinished = false;
	};

	class Bee {
	public:
		F32 x = 0.0F;
		F32 y = 0.0F;
		F32 homeX = 0.0F;
		F32 homeY = 0.0F;
		F32 speed = 120.0F;

		State state = State::Idle;
		Task task{};
		F32 workTimerSeconds = 0.0F;
		bool carryingItem = false;

		Bee() = default;
		Bee(F32 spawnX, F32 spawnY, F32 moveSpeed = 120.0F) {
			x = spawnX;
			y = spawnY;
			homeX = spawnX;
			homeY = spawnY;
			speed = moveSpeed;
		}

		bool is_busy() const {
			return state != State::Idle;
		}

		void set_home(F32 newHomeX, F32 newHomeY) {
			homeX = newHomeX;
			homeY = newHomeY;
			if (state == State::Idle) {
				x = homeX;
				y = homeY;
			}
		}

		bool assign_task(const Task& newTask) {
			if (is_busy() || newTask.type == TaskType::None) {
				return false;
			}
			task = newTask;
			workTimerSeconds = 0.0F;
			carryingItem = false;
			state = State::FlyingToTask;
			return true;
		}

		void cancel_task() {
			if (state == State::Idle) {
				return;
			}
			workTimerSeconds = 0.0F;
			carryingItem = false;
			state = State::ReturningHome;
			task.persistent = false;
		}

		UpdateResult update(F32 dt) {
			UpdateResult result{};
			if (dt <= 0.0F) {
				return result;
			}

			switch (state) {
			case State::Idle: {
			} break;

			case State::FlyingToTask: {
				if (move_towards(x, y, task.targetX, task.targetY, speed * dt)) {
					state = State::Working;
					workTimerSeconds = 0.0F;
					result.reachedTask = true;
				}
			} break;

			case State::Working: {
				workTimerSeconds += dt;
				if (workTimerSeconds >= task.workDurationSeconds) {
					workTimerSeconds = 0.0F;
					result.finishedWorkCycle = true;

					if (task.returnHomeAfterWork) {
						carryingItem = true;
						state = State::ReturningHome;
					}
					else if (!task.persistent) {
						finish_task();
						result.taskFinished = true;
					}
				}
			} break;

			case State::ReturningHome: {
				if (move_towards(x, y, homeX, homeY, speed * dt)) {
					if (carryingItem) {
						carryingItem = false;
						result.deliveredItem = true;
					}

					if (task.persistent) {
						state = State::FlyingToTask;
					}
					else {
						finish_task();
						result.taskFinished = true;
					}
				}
			} break;
			}

			return result;
		}

	private:
		static bool move_towards(F32& fromX, F32& fromY, F32 toX, F32 toY, F32 maxDistance) {
			F32 dx = toX - fromX;
			F32 dy = toY - fromY;
			F32 distanceSq = dx * dx + dy * dy;
			if (distanceSq <= 0.000001F) {
				fromX = toX;
				fromY = toY;
				return true;
			}

			F32 distance = sqrtf(distanceSq);
			if (distance <= maxDistance) {
				fromX = toX;
				fromY = toY;
				return true;
			}

			F32 scale = maxDistance / distance;
			fromX += dx * scale;
			fromY += dy * scale;
			return false;
		}

		void finish_task() {
			task = {};
			state = State::Idle;
			workTimerSeconds = 0.0F;
			carryingItem = false;
		}
	};

}
