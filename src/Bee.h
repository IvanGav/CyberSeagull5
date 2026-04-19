#pragma once

#include "drillengine/DrillLib.h"
#include "BeeTasks.h"
#include "TileSpace.h"

namespace Bee {

using namespace BeeTasks;
using namespace TileSpace;

static constexpr F32 DEFAULT_SPEED = 10.0F;
using PathFindFn = B32 (*)(V2U32 startTile, V2U32 goalTile, V2U32* pathTilesOut, U32* pathCountOut, U32 maxPathTiles, void* userData);
using PositionBlockedFn = B32 (*)(V2F32 position, V2U32 startTile, V2U32 goalTile, void* userData);

PathFindFn gPathFindFn = nullptr;
void* gPathFindUserData = nullptr;
PositionBlockedFn gPositionBlockedFn = nullptr;
void* gPositionBlockedUserData = nullptr;

FINLINE void set_path_finder(PathFindFn fn, void* userData = nullptr) {
    gPathFindFn = fn;
    gPathFindUserData = userData;
}

FINLINE void set_position_collider(PositionBlockedFn fn, void* userData = nullptr) {
    gPositionBlockedFn = fn;
    gPositionBlockedUserData = userData;
}

FINLINE V2F32 normalize_v2_safe(V2F32 v, F32 epsilon = 0.0001F) {
    F32 lenSq = length_sq(v);
    if (lenSq <= epsilon * epsilon) {
        return V2F32{};
    }
    return v / sqrtf32(lenSq);
}

FINLINE F32 hash01(U32 value) {
    U32 h = hash32(value);
    return F32(h & 0xFFFFu) * (1.0F / 65535.0F);
}

class Bee {
public:
    static constexpr F32 arrivalEpsilon = 0.01F;
    static constexpr F32 flightWobbleFrequency = 0.24F;
    static constexpr F32 flightWobbleAmplitude = 0.10F;
    static constexpr F32 flightSteerResponse = 7.0F;
    static constexpr F32 flightCruiseSpeedJitter = 0.08F;
    static constexpr F32 flightForwardNoiseAmplitude = 0.035F;
    static constexpr F32 flightWaypointLookAhead = 0.55F;
    static constexpr U32 MAX_PATH_TILES = 1024u;

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

    U32 carriedItem = 0;
    U32 carriedCount = 0;

    V2U32 pathTiles[MAX_PATH_TILES]{};
    U32 pathTileCount = 0;
    U32 pathTileIndex = 0;
    V2U32 pathGoalTile{};
    B32 pathGoalValid = B32_FALSE;

    F32 chaosSeed = 0.0F;
    F32 speedJitterSeed = 0.0F;

    Bee() = default;
    Bee(V2U32 spawnHomeTile, F32 speed = DEFAULT_SPEED)
        : homeTile(spawnHomeTile), moveSpeed(speed) {
        position = home_world_position();
        flightPhaseTurns = initial_flight_phase(spawnHomeTile);
        chaosSeed = hash01(spawnHomeTile.x * 0x9E3779B9u ^ spawnHomeTile.y * 0x85EBCA6Bu ^ 0x1234ABCDu);
        speedJitterSeed = hash01(spawnHomeTile.x * 0xC2B2AE35u ^ spawnHomeTile.y * 0x27D4EB2Fu ^ 0xB5297A4Du);
    }

    Bee(V2U32 spawnHomeTile, V2F32 spawnHomeOffsetWorld, F32 speed)
        : homeTile(spawnHomeTile), homeOffsetWorld(spawnHomeOffsetWorld), moveSpeed(speed) {
        position = home_world_position();
        flightPhaseTurns = initial_flight_phase(spawnHomeTile);
        chaosSeed = hash01(spawnHomeTile.x * 0x9E3779B9u ^ spawnHomeTile.y * 0x85EBCA6Bu ^ 0x1234ABCDu);
        speedJitterSeed = hash01(spawnHomeTile.x * 0xC2B2AE35u ^ spawnHomeTile.y * 0x27D4EB2Fu ^ 0xB5297A4Du);
    }

    void set_home(V2U32 newHomeTile) {
        homeTile = newHomeTile;
        invalidate_navigation_path();
    }

    void set_home_anchor(V2U32 newHomeTile, V2F32 newHomeOffsetWorld) {
        homeTile = newHomeTile;
        homeOffsetWorld = newHomeOffsetWorld;
        invalidate_navigation_path();
    }

    void teleport(V2F32 newPosition) {
        position = newPosition;
        velocity = V2F32{};
        invalidate_navigation_path();
    }

    void snap_to_home() {
        position = home_world_position();
        velocity = V2F32{};
        invalidate_navigation_path();
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

    B32 home_is(V2U32 tile) const {
        return homeTile.x == tile.x && homeTile.y == tile.y ? B32_TRUE : B32_FALSE;
    }

    B32 inside_hive_tile(V2U32 tile) const {
        return inside_hive() && home_is(tile) ? B32_TRUE : B32_FALSE;
    }

    B32 carrying() const {
        return carriedCount != 0 ? B32_TRUE : B32_FALSE;
    }

    void set_cargo(U32 item, U32 count = 1) {
        carriedItem = item;
        carriedCount = count;
    }

    void clear_cargo() {
        carriedItem = 0;
        carriedCount = 0;
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
        invalidate_navigation_path();
        state = hasTask ? State::STATE_TRAVEL_TO_TARGET : State::STATE_IDLE;
    }

    void cancel_task() {
        activeTask = Task{};
        hasTask = B32_FALSE;
        workTimerSeconds = 0.0F;
        travelTimerSeconds = 0.0F;
        velocity = V2F32{};
        invalidate_navigation_path();
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
                invalidate_navigation_path();
                break;
            }

            V2F32 targetPosition = task_world_position(activeTask);
            if (move_toward_navigation_goal(activeTask.targetTile, targetPosition, dt)) {
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
                invalidate_navigation_path();
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
            V2U32 homeGoalTile = TileSpace::world_to_tile(homePosition);
            if (move_toward_navigation_goal(homeGoalTile, homePosition, dt)) {
                position = homePosition;
                velocity = V2F32{};
                result.reachedHome = B32_TRUE;
                invalidate_navigation_path();

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

    void invalidate_navigation_path() {
        pathTileCount = 0;
        pathTileIndex = 0;
        pathGoalTile = V2U32{};
        pathGoalValid = B32_FALSE;
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

    V2U32 navigation_start_tile() const {
        return TileSpace::world_to_tile(position);
    }

    B32 ensure_navigation_path(V2U32 goalTile) {
        V2U32 startTile = navigation_start_tile();
        if (startTile.x == goalTile.x && startTile.y == goalTile.y) {
            pathTiles[0] = startTile;
            pathTileCount = 1;
            pathTileIndex = 0;
            pathGoalTile = goalTile;
            pathGoalValid = B32_TRUE;
            return B32_TRUE;
        }

        B32 pathMatchesGoal = pathGoalValid && pathGoalTile.x == goalTile.x && pathGoalTile.y == goalTile.y;
        B32 pathMatchesStart = pathTileCount != 0 && pathTileIndex < pathTileCount && pathTiles[pathTileIndex].x == startTile.x && pathTiles[pathTileIndex].y == startTile.y;
        if (pathMatchesGoal && pathMatchesStart) {
            return B32_TRUE;
        }

        invalidate_navigation_path();
        if (!gPathFindFn) {
            return B32_FALSE;
        }

        U32 newPathCount = 0;
        if (!gPathFindFn(startTile, goalTile, pathTiles, &newPathCount, MAX_PATH_TILES, gPathFindUserData) || newPathCount == 0) {
            return B32_FALSE;
        }

        pathTileCount = newPathCount;
        pathTileIndex = 0;
        pathGoalTile = goalTile;
        pathGoalValid = B32_TRUE;
        return B32_TRUE;
    }

    void advance_navigation_progress(V2F32 exactGoalPosition) {
        while (pathTileCount > 1 && pathTileIndex + 1 < pathTileCount) {
            U32 nextIndex = pathTileIndex + 1;
            B32 lastWaypoint = nextIndex + 1 >= pathTileCount ? B32_TRUE : B32_FALSE;
            V2F32 waypoint = lastWaypoint ? exactGoalPosition : TileSpace::tile_to_world_center(pathTiles[nextIndex]);
            if (distance_sq(position, waypoint) <= 0.08F * 0.08F) {
                pathTileIndex = nextIndex;
                continue;
            }

            V2F32 segmentStart = pathTileIndex == 0 ? position : TileSpace::tile_to_world_center(pathTiles[pathTileIndex]);
            V2F32 segment = waypoint - segmentStart;
            F32 segmentLenSq = length_sq(segment);
            if (segmentLenSq > 0.0001F) {
                F32 projection = dot(position - segmentStart, segment) / segmentLenSq;
                if (projection >= 0.98F) {
                    pathTileIndex = nextIndex;
                    continue;
                }
            }
            break;
        }
    }

    V2U32 collision_start_tile() const {
        switch (state) {
        case State::STATE_TRAVEL_TO_TARGET:
            return homeTile;
        case State::STATE_TRAVEL_HOME:
            return hasTask ? activeTask.targetTile : homeTile;
        default:
            return TileSpace::world_to_tile(position);
        }
    }

    B32 position_blocked(V2F32 candidate, V2U32 goalTile) const {
        if (!gPositionBlockedFn) {
            return B32_FALSE;
        }
        return gPositionBlockedFn(candidate, collision_start_tile(), goalTile, gPositionBlockedUserData);
    }

    B32 move_toward_navigation_goal(V2U32 goalTile, V2F32 exactGoalPosition, F32 dt) {
        if (is_at(exactGoalPosition)) {
            position = exactGoalPosition;
            velocity = V2F32{};
            return B32_TRUE;
        }

        if (!ensure_navigation_path(goalTile)) {
            velocity = V2F32{};
            return B32_FALSE;
        }

        advance_navigation_progress(exactGoalPosition);

        V2F32 waypoint = exactGoalPosition;
        B32 lastWaypoint = B32_TRUE;
        if (pathTileCount > 1 && pathTileIndex + 1 < pathTileCount) {
            U32 nextIndex = pathTileIndex + 1;
            lastWaypoint = nextIndex + 1 >= pathTileCount ? B32_TRUE : B32_FALSE;
            waypoint = lastWaypoint ? exactGoalPosition : TileSpace::tile_to_world_center(pathTiles[nextIndex]);
        }

        move_toward_flying(waypoint, dt, lastWaypoint);
        advance_navigation_progress(exactGoalPosition);

        if (distance_sq(position, exactGoalPosition) <= arrivalEpsilon * arrivalEpsilon) {
            position = exactGoalPosition;
            velocity = V2F32{};
            return B32_TRUE;
        }
        return B32_FALSE;
    }

    void move_toward_flying(V2F32 target, F32 dt, B32 slowForArrival) {
        V2F32 delta = target - position;
        F32 distSq = length_sq(delta);
        if (distSq <= arrivalEpsilon * arrivalEpsilon) {
            position = target;
            velocity = V2F32{};
            return;
        }

        if (dt <= 0.0F) {
            velocity = V2F32{};
            return;
        }

        F32 dist = sqrtf32(distSq);
        V2F32 forward = delta / dist;
        V2F32 side = get_orthogonal(forward);
        travelTimerSeconds += dt;

        F32 chaosTurns = travelTimerSeconds + chaosSeed;
        F32 wobbleTurns = chaosTurns * flightWobbleFrequency + flightPhaseTurns;
        F32 lateralNoise = 0.72F * sinf32(wobbleTurns + chaosSeed * 0.71F)
            + 0.28F * sinf32(wobbleTurns * 1.61F + speedJitterSeed * 1.37F);
        F32 speedNoise = 0.70F * sinf32(chaosTurns * 0.51F + speedJitterSeed * 3.11F)
            + 0.30F * sinf32(chaosTurns * 0.93F + chaosSeed * 1.29F);

        F32 wobbleScale = min(0.05F, dist * 0.18F);
        wobbleScale *= 0.15F + 0.85F * clamp01(dist / 1.2F);

        V2F32 desiredDirection = normalize_v2_safe(forward + side * (lateralNoise * wobbleScale));
        F32 speedMultiplier = 1.0F + speedNoise * flightCruiseSpeedJitter;
        if (slowForArrival && dist < 0.60F) {
            speedMultiplier *= 0.70F + 0.30F * clamp01(dist / 0.60F);
        }
        V2F32 desiredVelocity = desiredDirection * (moveSpeed * speedMultiplier);

        F32 response = clamp01(dt * flightSteerResponse);
        velocity += (desiredVelocity - velocity) * response;

        F32 speedSq = length_sq(velocity);
        F32 maxSpeed = moveSpeed * (1.0F + flightCruiseSpeedJitter);
        if (speedSq > maxSpeed * maxSpeed) {
            velocity = normalize_v2_safe(velocity) * maxSpeed;
        }

        V2F32 step = velocity * dt;
        V2F32 remaining = target - position;
        if (dot(step, remaining) > length_sq(remaining)) {
            step = remaining;
        }

        V2U32 collisionGoalTile = pathGoalValid ? pathGoalTile : (hasTask ? activeTask.targetTile : homeTile);
        V2F32 candidate = position + step;
        if (position_blocked(candidate, collisionGoalTile)) {
            F32 stepLength = length(step);
            if (stepLength > 0.0F) {
                const F32 fallbackFactors[] = { 1.0F, 0.65F, 0.4F, 0.2F };
                V2F32 escapeDirs[5]{
                    forward,
                    normalize_v2_safe(forward + side * 0.85F),
                    normalize_v2_safe(forward - side * 0.85F),
                    normalize_v2_safe(side),
                    normalize_v2_safe(-side),
                };
                for (U32 dirIndex = 0; dirIndex < ARRAY_COUNT(escapeDirs); dirIndex++) {
                    if (length_sq(escapeDirs[dirIndex]) <= 0.000001F) {
                        continue;
                    }
                    for (U32 i = 0; i < ARRAY_COUNT(fallbackFactors); i++) {
                        V2F32 fallbackStep = escapeDirs[dirIndex] * (stepLength * fallbackFactors[i]);
                        if (dot(fallbackStep, remaining) > length_sq(remaining)) {
                            fallbackStep = remaining;
                        }
                        V2F32 fallbackCandidate = position + fallbackStep;
                        if (!position_blocked(fallbackCandidate, collisionGoalTile)) {
                            position = fallbackCandidate;
                            velocity = escapeDirs[dirIndex] * (moveSpeed * 0.45F);
                            return;
                        }
                    }
                }
            }

            V2U32 currentTile = TileSpace::world_to_tile(position);
            V2F32 currentTileCenter = TileSpace::tile_to_world_center(currentTile);
            V2F32 toTileCenter = currentTileCenter - position;
            F32 toTileCenterLenSq = length_sq(toTileCenter);
            if (toTileCenterLenSq > 0.0001F) {
                V2F32 recenterStep = normalize_v2_safe(toTileCenter) * min(sqrtf32(toTileCenterLenSq), moveSpeed * dt * 0.6F);
                V2F32 recenterCandidate = position + recenterStep;
                if (!position_blocked(recenterCandidate, collisionGoalTile)) {
                    position = recenterCandidate;
                    velocity = normalize_v2_safe(recenterStep) * (moveSpeed * 0.25F);
                    invalidate_navigation_path();
                    return;
                }
            }

            velocity = V2F32{};
            invalidate_navigation_path();
            return;
        }

        position = candidate;
        if (distance_sq(position, target) <= arrivalEpsilon * arrivalEpsilon) {
            position = target;
            velocity *= 0.55F;
        }
    }

    void finish_work_cycle(UpdateResult* result) {
        result->finishedWork = B32_TRUE;
        workTimerSeconds = 0.0F;
        invalidate_navigation_path();

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
