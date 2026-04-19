#pragma once

#include "Bee.h"


namespace BeeSystem {

using namespace Bee;

struct QueuedTask {
	Task task{};
	I32 assignedBee = -1;
	B32 selected = B32_FALSE;
};

class Colony {
public:
	MemoryArena* allocator = &globalArena;
	ArenaArrayList<Bee> bees{};
	ArenaArrayList<QueuedTask> queuedTasks{};
	V2U32 defaultHiveTile{};

	void init(U32 beeCount, V2U32 hiveTile, F32 beeSpeed = DEFAULT_SPEED, MemoryArena* arena = &globalArena) {
		allocator = arena ? arena : &globalArena;
		bees.allocator = allocator;
		queuedTasks.allocator = allocator;
		bees.clear();
		queuedTasks.clear();
		defaultHiveTile = hiveTile;
		for (U32 i = 0; i < beeCount; i++) {
			bees.push_back(Bee{ hiveTile, beeSpeed });
		}
	}

	U32 total_bee_count() const {
		return bees.size;
	}

	U32 busy_bee_count() const {
		U32 result = 0;
		for (U32 i = 0; i < bees.size; i++) {
			result += bees.data[i].busy() ? 1u : 0u;
		}
		return result;
	}

	U32 idle_bee_count() const {
		return total_bee_count() - busy_bee_count();
	}

	void add_bee(V2U32 hiveTile) {
		bees.push_back(Bee{ hiveTile, DEFAULT_SPEED });
		assign_waiting_tasks();
	}

	I32 queue_task(const Task& task) {
		if (!task_is_valid(task)) {
			return -1;
		}
		QueuedTask& queued = queuedTasks.push_back_zeroed();
		queued.task = task;
		queued.assignedBee = -1;
		queued.selected = B32_TRUE;
		assign_waiting_tasks();
		return I32(queuedTasks.size - 1);
	}

	void unqueue_task(U32 taskIndex) {
		if (taskIndex >= queuedTasks.size) {
			return;
		}

		QueuedTask& queued = queuedTasks[taskIndex];
		if (queued.assignedBee >= 0 && U32(queued.assignedBee) < bees.size) {
			bees[queued.assignedBee].cancel_task();
		}
		queuedTasks.remove_ordered(taskIndex);
	}

	void update(F32 dtSeconds) {
		for (U32 beeIndex = 0; beeIndex < bees.size; beeIndex++) {
			UpdateResult result = bees[beeIndex].update(dtSeconds);
			if (result.taskFinished) {
				release_finished_task(beeIndex);
			}
		}
		assign_waiting_tasks();
	}

private:
	I32 find_idle_bee() {
		for (U32 i = 0; i < bees.size; i++) {
			if (bees[i].idle()) {
				return I32(i);
			}
		}
		return -1;
	}

	void assign_waiting_tasks() {
		for (U32 taskIndex = 0; taskIndex < queuedTasks.size; taskIndex++) {
			QueuedTask& queued = queuedTasks[taskIndex];
			if (!queued.selected || queued.assignedBee >= 0) {
				continue;
			}

			I32 beeIndex = find_idle_bee();
			if (beeIndex < 0) {
				return;
			}

			queued.assignedBee = beeIndex;
			bees[beeIndex].assign_task(queued.task);
		}
	}

	void release_finished_task(U32 beeIndex) {
		for (U32 taskIndex = 0; taskIndex < queuedTasks.size; taskIndex++) {
			QueuedTask& queued = queuedTasks[taskIndex];
			if (queued.assignedBee != I32(beeIndex)) {
				continue;
			}

			queued.assignedBee = -1;
			if (!queued.task.persistent) {
				queuedTasks.remove_ordered(taskIndex);
			}
			return;
		}
	}
};

}
