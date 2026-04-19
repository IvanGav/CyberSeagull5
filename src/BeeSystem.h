#pragma once

#include "Bee.h"

namespace BeeSystem {

struct HomeAnchor {
	V2U32 tile{};
	V2F32 offsetWorld{ 0.5F, 0.5F };
};

using SelectHomeAnchorFn = HomeAnchor (*)(V2U32 targetTile, void* userData);

enum class EventType : U8 {
	EVENT_NONE = 0,
	EVENT_TASK_ASSIGNED,
	EVENT_TASK_UNASSIGNED,
	EVENT_WORK_CYCLE_FINISHED,
	EVENT_BEE_REACHED_HOME,
	EVENT_TASK_FINISHED,
};

struct Event {
	EventType type = EventType::EVENT_NONE;
	U32 beeIndex = 0;
	BeeTasks::Task task{};
};

struct QueuedTask {
	BeeTasks::Task task{};
	I32 assignedBee = -1;
	B32 selected = B32_FALSE;
};

class Colony {
public:
	MemoryArena* allocator = &globalArena;
	ArenaArrayList<Bee::Bee> bees{};
	ArenaArrayList<QueuedTask> queuedTasks{};
	ArenaArrayList<Event> events{};
	HomeAnchor defaultHome{};
	F32 beeSpeed = Bee::DEFAULT_SPEED;
	SelectHomeAnchorFn selectHomeAnchorFn = nullptr;
	void* selectHomeAnchorUserData = nullptr;

	void init(U32 beeCount, V2U32 hiveTile, F32 beeMoveSpeed = Bee::DEFAULT_SPEED, MemoryArena* arena = &globalArena) {
		allocator = arena ? arena : &globalArena;
		bees.allocator = allocator;
		queuedTasks.allocator = allocator;
		events.allocator = allocator;
		bees.clear();
		queuedTasks.clear();
		events.clear();
		defaultHome.tile = hiveTile;
		defaultHome.offsetWorld = V2F32{ 0.5F, 0.5F };
		beeSpeed = beeMoveSpeed;
		for (U32 i = 0; i < beeCount; i++) {
			bees.push_back(Bee::Bee{ hiveTile, defaultHome.offsetWorld, beeSpeed });
			Bee::Bee& bee = bees.back();
			F32 phaseOffset = F32((i * 173u) & 1023u) * (1.0F / 1024.0F);
			bee.flightPhaseTurns += phaseOffset;
			bee.chaosSeed = Bee::hash01((i + 1u) * 0x9E3779B9u ^ hiveTile.x * 0x85EBCA6Bu ^ hiveTile.y * 0xC2B2AE35u);
			bee.speedJitterSeed = Bee::hash01((i + 1u) * 0x27D4EB2Fu ^ hiveTile.x * 0x165667B1u ^ hiveTile.y * 0xD3A2646Cu);
		}
	}

	void set_default_home(V2U32 hiveTile, V2F32 homeOffsetWorld = V2F32{ 0.5F, 0.5F }) {
		defaultHome.tile = hiveTile;
		defaultHome.offsetWorld = homeOffsetWorld;
	}

	void set_home_selector(SelectHomeAnchorFn fn, void* userData = nullptr) {
		selectHomeAnchorFn = fn;
		selectHomeAnchorUserData = userData;
	}

	U32 total_bee_count() const {
		return bees.size;
	}

	U32 busy_bee_count() const {
		U32 result = 0;
		for (U32 i = 0; i < bees.size; i++) {
			result += bees[i].busy() ? 1u : 0u;
		}
		return result;
	}

	U32 idle_bee_count() const {
		return total_bee_count() - busy_bee_count();
	}

	U32 count_bees_inside_home_tile(V2U32 homeTile) const {
		U32 result = 0;
		for (U32 i = 0; i < bees.size; i++) {
			result += bees[i].inside_hive_tile(homeTile) ? 1u : 0u;
		}
		return result;
	}

	void add_bee() {
		bees.push_back(Bee::Bee{ defaultHome.tile, defaultHome.offsetWorld, beeSpeed });
			Bee::Bee& bee = bees.back();
		U32 i = bees.size - 1;
		bee.flightPhaseTurns += F32((i * 173u) & 1023u) * (1.0F / 1024.0F);
		bee.chaosSeed = Bee::hash01((i + 1u) * 0x9E3779B9u ^ defaultHome.tile.x * 0x85EBCA6Bu ^ defaultHome.tile.y * 0xC2B2AE35u);
		bee.speedJitterSeed = Bee::hash01((i + 1u) * 0x27D4EB2Fu ^ defaultHome.tile.x * 0x165667B1u ^ defaultHome.tile.y * 0xD3A2646Cu);
		assign_waiting_tasks();
	}

	void add_bee(HomeAnchor home) {
		bees.push_back(Bee::Bee{ home.tile, home.offsetWorld, beeSpeed });
			Bee::Bee& bee = bees.back();
		U32 i = bees.size - 1;
		bee.flightPhaseTurns += F32((i * 173u) & 1023u) * (1.0F / 1024.0F);
		bee.chaosSeed = Bee::hash01((i + 1u) * 0x9E3779B9u ^ home.tile.x * 0x85EBCA6Bu ^ home.tile.y * 0xC2B2AE35u);
		bee.speedJitterSeed = Bee::hash01((i + 1u) * 0x27D4EB2Fu ^ home.tile.x * 0x165667B1u ^ home.tile.y * 0xD3A2646Cu);
		assign_waiting_tasks();
	}

	I32 find_task_by_tile(V2U32 tile) const {
		for (U32 i = 0; i < queuedTasks.size; i++) {
			const QueuedTask& queued = queuedTasks[i];
			if (!queued.selected) {
				continue;
			}
			if (queued.task.targetTile.x == tile.x && queued.task.targetTile.y == tile.y) {
				return I32(i);
			}
		}
		return -1;
	}

	B32 is_tile_selected(V2U32 tile) const {
		return find_task_by_tile(tile) >= 0 ? B32_TRUE : B32_FALSE;
	}

	I32 queue_task(const BeeTasks::Task& task) {
		if (!BeeTasks::task_is_valid(task)) {
			return -1;
		}

		I32 existingIndex = find_task_by_tile(task.targetTile);
		if (existingIndex >= 0) {
			queuedTasks[existingIndex].task = task;
			return existingIndex;
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

		QueuedTask removed = queuedTasks[taskIndex];
		if (removed.assignedBee >= 0 && U32(removed.assignedBee) < bees.size) {
			bees[removed.assignedBee].cancel_task();
			push_event(EventType::EVENT_TASK_UNASSIGNED, U32(removed.assignedBee), removed.task);
		}
		queuedTasks.remove_ordered(taskIndex);
	}

	void unqueue_task_for_tile(V2U32 tile) {
		I32 taskIndex = find_task_by_tile(tile);
		if (taskIndex >= 0) {
			unqueue_task(U32(taskIndex));
		}
	}

	void clear_tasks() {
		for (U32 i = 0; i < bees.size; i++) {
			bees[i].cancel_task();
		}
		queuedTasks.clear();
	}

	void update(F32 dtSeconds) {
		events.clear();
		for (U32 beeIndex = 0; beeIndex < bees.size; beeIndex++) {
			I32 taskIndex = find_task_assigned_to_bee(beeIndex);
			BeeTasks::Task eventTask = taskIndex >= 0 ? queuedTasks[taskIndex].task : BeeTasks::Task{};

			BeeTasks::UpdateResult result = bees[beeIndex].update(dtSeconds);

			if (result.finishedWork && taskIndex >= 0) {
				push_event(EventType::EVENT_WORK_CYCLE_FINISHED, beeIndex, eventTask);
			}
			if (result.reachedHome) {
				push_event(EventType::EVENT_BEE_REACHED_HOME, beeIndex, eventTask);
			}
			if (result.taskFinished && taskIndex >= 0) {
				BeeTasks::Task finishedTask = queuedTasks[taskIndex].task;
				release_finished_task(beeIndex);
				push_event(EventType::EVENT_TASK_FINISHED, beeIndex, finishedTask);
			}
		}
		assign_waiting_tasks();
	}

private:
	void push_event(EventType type, U32 beeIndex, const BeeTasks::Task& task) {
		Event& event = events.push_back_zeroed();
		event.type = type;
		event.beeIndex = beeIndex;
		event.task = task;
	}

	HomeAnchor home_for_task(V2U32 taskTile) const {
		if (selectHomeAnchorFn) {
			return selectHomeAnchorFn(taskTile, selectHomeAnchorUserData);
		}
		return defaultHome;
	}

	I32 find_task_assigned_to_bee(U32 beeIndex) const {
		for (U32 taskIndex = 0; taskIndex < queuedTasks.size; taskIndex++) {
			if (queuedTasks[taskIndex].assignedBee == I32(beeIndex)) {
				return I32(taskIndex);
			}
		}
		return -1;
	}

	I32 find_best_idle_bee(V2U32 taskTile) const {
		I32 bestBeeIndex = -1;
		F32 bestDistSq = F32_INF;
		V2F32 target = TileSpace::tile_to_world_center(taskTile);
		for (U32 i = 0; i < bees.size; i++) {
			if (!bees[i].idle()) {
				continue;
			}
			F32 distSq = distance_sq(bees[i].position, target);
			if (bestBeeIndex < 0 || distSq < bestDistSq) {
				bestBeeIndex = I32(i);
				bestDistSq = distSq;
			}
		}
		return bestBeeIndex;
	}

	void assign_waiting_tasks() {
		for (U32 taskIndex = 0; taskIndex < queuedTasks.size; taskIndex++) {
			QueuedTask& queued = queuedTasks[taskIndex];
			if (!queued.selected || queued.assignedBee >= 0) {
				continue;
			}

			I32 beeIndex = find_best_idle_bee(queued.task.targetTile);
			if (beeIndex < 0) {
				return;
			}

			HomeAnchor home = home_for_task(queued.task.targetTile);
			bees[beeIndex].set_home_anchor(home.tile, home.offsetWorld);
			bees[beeIndex].snap_to_home();
			bees[beeIndex].assign_task(queued.task);
			queued.assignedBee = beeIndex;
			push_event(EventType::EVENT_TASK_ASSIGNED, U32(beeIndex), queued.task);
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
