#ifndef VMC_GODOT_H
#define VMC_GODOT_H

#include <godot_cpp/core/class_db.hpp>

void initialize_vmc_godot(godot::ModuleInitializationLevel p_level);
void uninitialize_vmc_godot(godot::ModuleInitializationLevel p_level);

#endif
