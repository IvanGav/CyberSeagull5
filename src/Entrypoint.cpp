#include "drillengine/DrillLib.h"
#include "drillengine/SerializeTools.h"
#include "drillengine/PNG.h"
#include "Cyber5eagull.h"

int main() {
	const U32 failToInitializeDrillLib = 2;
	U32 result = failToInitializeDrillLib;
	if (drill_lib_init()) {
		_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
		_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
		PNG::init_loader();
		result = Cyber5eagull::run_cyber5eagull();
	}
	return int(result);
}