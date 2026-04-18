#pragma once

#include "drillengine/DrillLib.h"


namespace Bee {

enum class TaskType {
	TASK_NONE = 0,
	TASK_SHORE,
	TASK_ORE,
	TASK_FLOWER,
	TASK_BUILD,
	TASK_GENERIC,
};

enum class State {
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

}
