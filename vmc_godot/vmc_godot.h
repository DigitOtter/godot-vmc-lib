#ifndef VMC_GODOT_H
#define VMC_GODOT_H

#include <Godot.hpp>
#include <MeshInstance.hpp>

class VMCGodot
        : public godot::MeshInstance
{
	public:
		GODOT_CLASS(VMCGodot, godot::MeshInstance);

	private:
		void test();
};

#endif
