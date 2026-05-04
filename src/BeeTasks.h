#pragma once

#include "drillengine/DrillLib.h"


namespace BeeTasks {

enum class TaskType : U8 {
	TASK_NONE = 0,
	TASK_SHORE,
	TASK_ORE,
	TASK_FLOWER,
	TASK_BUILD,
	TASK_GENERIC,
};

enum class State : U8 {
	STATE_IDLE = 0,
	STATE_TRAVEL_TO_TARGET,
	STATE_WORKING,
	STATE_TRAVEL_HOME,
};

struct Task {
	TaskType type = TaskType::TASK_NONE;
	V2U32 targetTile{};
	F32 workDurationSeconds = 0.0F;
	B32 persistent = B32_FALSE;
	B32 returnHomeAfterWork = B32_TRUE;
};

struct UpdateResult {
	B32 reachedTarget = B32_FALSE;
	B32 startedWorking = B32_FALSE;
	B32 finishedWork = B32_FALSE;
	B32 reachedHome = B32_FALSE;
	B32 taskFinished = B32_FALSE;
};

B32 task_is_valid(const Task& task) {
	return task.type != TaskType::TASK_NONE;
}

Task make_shore_task(V2U32 targetTile, F32 workDurationSeconds) {
	Task task{};
	task.type = TaskType::TASK_SHORE;
	task.targetTile = targetTile;
	task.workDurationSeconds = workDurationSeconds;
	task.persistent = B32_TRUE;
	return task;
}

Task make_ore_task(V2U32 targetTile, F32 workDurationSeconds, B32 returnHomeAfterWork = B32_TRUE) {
	Task task{};
	task.type = TaskType::TASK_ORE;
	task.targetTile = targetTile;
	task.workDurationSeconds = workDurationSeconds;
	task.persistent = B32_TRUE;
	task.returnHomeAfterWork = returnHomeAfterWork;
	return task;
}

Task make_flower_task(V2U32 targetTile, F32 workDurationSeconds, B32 returnHomeAfterWork = B32_TRUE) {
	Task task{};
	task.type = TaskType::TASK_FLOWER;
	task.targetTile = targetTile;
	task.workDurationSeconds = workDurationSeconds;
	task.persistent = B32_TRUE;
	task.returnHomeAfterWork = returnHomeAfterWork;
	return task;
}

Task make_build_task(V2U32 targetTile, F32 workDurationSeconds) {
	Task task{};
	task.type = TaskType::TASK_BUILD;
	task.targetTile = targetTile;
	task.workDurationSeconds = workDurationSeconds;
	return task;
}

Task make_generic_task(V2U32 targetTile, F32 workDurationSeconds, B32 persistent = B32_FALSE, B32 returnHomeAfterWork = B32_TRUE) {
	Task task{};
	task.type = TaskType::TASK_GENERIC;
	task.targetTile = targetTile;
	task.workDurationSeconds = workDurationSeconds;
	task.persistent = persistent;
	task.returnHomeAfterWork = returnHomeAfterWork;
	return task;
}

}
