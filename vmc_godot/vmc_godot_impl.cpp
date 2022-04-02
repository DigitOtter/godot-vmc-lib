#include "vmc_godot_impl.h"

#include "vmc_receiver.h"

//std::string VMCGodotImpl::VMCLibraryPath = "";

extern "C" void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options* o)
{
	godot::Godot::gdnative_init(o);

//	godot::String *pLibPath = godot::detail::get_wrapper<godot::String>((godot_object*)o->active_library_path);
//	assert(pLibPath);
//	std::string libPath = pLibPath->ascii().get_data();
//	VMCGodotImpl::VMCLibraryPath = libPath.substr(0, libPath.find_last_of('/'));
}

extern "C" void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options* o)
{
	godot::Godot::gdnative_terminate(o);
	//VMCComm::DeleteListenerInstance();
}

extern "C" void GDN_EXPORT godot_nativescript_init(void* handle)
{
	godot::Godot::nativescript_init(handle);

//	godot::register_class<VMCComm>();
//	godot::register_class<VMCAvatarListener>();
//	godot::register_class<VMCAvatarBlendShapeController>();
//	godot::register_class<VMCAvatarSkeletonController>();
//	godot::register_class<VMCAvatarRootController>();

	godot::register_class<VmcReceiver>();
}
