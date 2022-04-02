#include "vmc_godot/vmc_avatar_controller.h"

#include "vmc_godot/vmc_comm.h"
#include "vmc_godot/vmc_packets.h"

#include <Script.hpp>

void VMCAvatarListener::_register_methods()
{
	godot::register_method("_process", &VMCAvatarListener::_process);
	godot::register_method("_ready", &VMCAvatarListener::_ready);

	//godot::register_signal("new_vmc_avatar", &VMCAvatarController::_process);
}

void VMCAvatarListener::_init()
{
}

void VMCAvatarListener::_process()
{

}

void VMCAvatarListener::_ready()
{
}

VMCAvatarListener::VMCAvatarMsgQueue *VMCAvatarListener::EnsureActiveMsgStore()
{
	if(!VMCAvatarMsgQueue::Instance())
		VMCAvatarMsgQueue::Reset();

	return VMCAvatarMsgQueue::Instance();
}

VMCAvatarListener::VMCAvatarMsgQueue *VMCAvatarListener::MsgStore()
{
	return VMCAvatarListener::VMCAvatarMsgQueue::Instance();
}

inline bool CheckOSCArgType(const osc::ReceivedMessageArgumentIterator &it, const char type)
{
	return it->TypeTag() == type;
}

inline void PrintOSCArgMismatch(const char received, const char expected)
{
	godot::Godot::print(godot::String("Unexpected OSC argument type. Expected '") + expected + "', received '" + received + "'");
}

void VMCAvatarListener::VMCAvatarMsgQueue::ReceiveMessage(const osc::ReceivedMessage &m, const IpEndpointName &/*remoteEndpoint*/)
{
	if(vmc::marionette::EXT_BLEND_APPLY == m.AddressPattern())
	{
		// /Ext/Blend/Apply
		// Write new blend shaped over existing ones. Don't just move the new map over the old one,
		//     because that invalidates existing pointers
		// TODO: Improve efficiency of copy by taking advantage of sorted lists
		for(const auto &newShape : this->_newBlendShapes)
		{
			if(auto res = this->_currentBlendShapes.emplace(newShape); !res.second)
				res.first->second = newShape.second;
		}

		this->_newBlendShapes.clear();
		++this->_blendShapeChangeID;
	}
	else if(vmc::marionette::EXT_ROOT_POS == m.AddressPattern())
	{
		// /Ext/Root/Pos
		if(m.ArgumentCount() != 8)
		{
			godot::Godot::print(godot::String("Unexpected number of arguments received for Root Pos. Expected 8, got ") + std::to_string(m.ArgumentCount()).data());
			return;
		}

		std::string name;
		godot::Quat quat;
		godot::Transform transform;

		auto oscDatIt = m.ArgumentsBegin();
		name = oscDatIt->AsString();

		transform.origin.x = (++oscDatIt)->AsFloat();
		transform.origin.y = (++oscDatIt)->AsFloat();
		transform.origin.z = (++oscDatIt)->AsFloat();

		quat.x = (++oscDatIt)->AsFloat();
		quat.y = (++oscDatIt)->AsFloat();
		quat.z = (++oscDatIt)->AsFloat();
		quat.w = (++oscDatIt)->AsFloat();

		transform.basis = quat;

		if(auto res = this->_rootPos.emplace(name, transform); !res.second)
			res.first->second = std::move(transform);

		++this->_rootChangeID;
	}
	else if(vmc::marionette::EXT_BONE_POS == m.AddressPattern())
	{
		// /Ext/Bone/Pos
		if(m.ArgumentCount() != 8)
		{
			godot::Godot::print(godot::String("Unexpected number of arguments received for Root Pos. Expected 8, got ") + std::to_string(m.ArgumentCount()).data());
			return;
		}

		std::string name;
		godot::Quat quat;
		godot::Transform transform;

		auto oscDatIt = m.ArgumentsBegin();
		name = oscDatIt->AsString();

		transform.origin.x = (++oscDatIt)->AsFloat();
		transform.origin.y = (++oscDatIt)->AsFloat();
		transform.origin.z = (++oscDatIt)->AsFloat();

		quat.x = (++oscDatIt)->AsFloat();
		quat.y = (++oscDatIt)->AsFloat();
		quat.z = (++oscDatIt)->AsFloat();
		quat.w = (++oscDatIt)->AsFloat();

		transform.basis = quat;

		if(auto res = this->_bonePos.emplace(name, transform); !res.second)
			res.first->second = std::move(transform);

		++this->_boneChangeID;
	}
	else if(vmc::marionette::EXT_BLEND_VAL == m.AddressPattern())
	{
		// /Ext/Blend/Val
		if(m.ArgumentCount() != 2)
		{
			godot::Godot::print(godot::String("Unexpected number of arguments received for Root Pos. Expected 8, got ") + std::to_string(m.ArgumentCount()).data());
			return;
		}

		std::string name;
		float val;

		auto oscDatIt = m.ArgumentsBegin();
		name = oscDatIt->AsString();
		val = (++oscDatIt)->AsFloat();

		if(auto res = this->_newBlendShapes.emplace(name, val); !res.second)
			res.first->second = std::move(val);
	}
}

bool VMCAvatarListener::VMCAvatarMsgQueue::RootPosAvailable(const std::string &name) const
{	return this->_rootPos.find(name) != this->_rootPos.end();	}

void VMCAvatarListener::VMCAvatarMsgQueue::SetRootPos(const std::string &name, godot::Transform transform)
{
	if(auto res = this->_rootPos.emplace(name, transform); !res.second)
		res.first->second = std::move(transform);
}

const godot::Transform *VMCAvatarListener::VMCAvatarMsgQueue::GetRootPos(const std::string &name) const
{
	if(auto it = this->_rootPos.find(name); it != this->_rootPos.end())
		return &it->second;

	return nullptr;
}


bool VMCAvatarListener::VMCAvatarMsgQueue::BonePosAvailable(const std::string &name) const
{	return this->_bonePos.find(name) != this->_bonePos.end();	}

void VMCAvatarListener::VMCAvatarMsgQueue::SetBonePos(const std::string &name, godot::Transform transform)
{
	if(auto res = this->_bonePos.emplace(name, transform); !res.second)
		res.first->second = std::move(transform);
}

const godot::Transform *VMCAvatarListener::VMCAvatarMsgQueue::GetBonePos(const std::string &name) const
{
	if(auto it = this->_bonePos.find(name); it != this->_bonePos.end())
		return &it->second;

	return nullptr;
}


bool VMCAvatarListener::VMCAvatarMsgQueue::BlendShapeAvailable(const std::string &name) const
{	return this->_currentBlendShapes.find(name) != this->_currentBlendShapes.end();	}

void VMCAvatarListener::VMCAvatarMsgQueue::SetBlendShape(const std::string &name, float value)
{
	if(auto res = this->_currentBlendShapes.emplace(name, value); !res.second)
		res.first->second = std::move(value);
}

const float *VMCAvatarListener::VMCAvatarMsgQueue::GetBlendShape(const std::string &name)
{
	if(auto it = this->_currentBlendShapes.find(name); it != this->_currentBlendShapes.end())
		return &it->second;

	return nullptr;
}


VMCAvatarListener::VMCAvatarMsgQueue::VMCAvatarMsgQueue()
        : VMCMsgProcessor(vmc::marionette::EXT_ROOT_POS.data(), *VMCComm::EnsureActiveListener())
{
	this->RegisteredQueue()->RegisterMsgProcessor(vmc::marionette::EXT_BLEND_VAL.data(), this);
	this->RegisteredQueue()->RegisterMsgProcessor(vmc::marionette::EXT_BLEND_APPLY.data(), this);
	this->RegisteredQueue()->RegisterMsgProcessor(vmc::marionette::EXT_BONE_POS.data(), this);
}

void VMCAvatarController::_register_methods()
{

}

void VMCAvatarController::_init()
{}

void VMCAvatarBlendShapeController::_register_methods()
{
	//godot::register_method("_ready", &VMCAvatarBlendShapeController::_ready);
	godot::register_method("_process", &VMCAvatarBlendShapeController::_process);
}

void VMCAvatarBlendShapeController::_init()
{
	auto *const pMsgs = VMCAvatarListener::EnsureActiveMsgStore();
	this->_blendShapeChangeID = pMsgs->BlendShapeChangeID();

	auto props = this->get_property_list();
	for(int i=0; i<props.size(); ++i)
	{
		godot::Dictionary prop = props[i];
		godot::String prop_name = prop["name"];
		if(prop_name.begins_with_char_array(BlendShapePrefix.data()))
		{
			const std::string bshape_name = prop_name.substr(BlendShapePrefix.length(), prop_name.length()-BlendShapePrefix.length()).ascii().get_data();
			const float *const pVal = pMsgs->GetBlendShape(bshape_name);

			this->_shapeVals.emplace(prop_name, BlendShapeData({bshape_name, pVal}));
		}
	}
}

//void VMCAvatarBlendShapeController::_ready()
//{}

void VMCAvatarBlendShapeController::_process(float)
{
	if(this->_blendShapeChangeID != VMCAvatarListener::MsgStore()->BlendShapeChangeID())
	{
		auto *const pMsgs = VMCAvatarListener::MsgStore();

		this->_blendShapeChangeID = VMCAvatarListener::MsgStore()->BlendShapeChangeID();

		for(auto &shape : this->_shapeVals)
		{
			if(shape.second.Val == nullptr)
			{
				shape.second.Val = pMsgs->GetBlendShape(shape.second.Name);
				if(!shape.second.Val)
					continue;
			}

			this->set(shape.first, *shape.second.Val);
		}
	}
}

void VMCAvatarSkeletonController::_register_methods()
{
	//godot::register_method("_ready", &VMCAvatarSkeletonController::_ready);
	godot::register_method("_process", &VMCAvatarSkeletonController::_process);
}

void VMCAvatarSkeletonController::_init()
{
	auto *const pMsgs = VMCAvatarListener::EnsureActiveMsgStore();
	this->_boneChangeID = pMsgs->BoneChangeID();

	const auto numBones = this->get_bone_count();
	for(int64_t i=0; i < numBones; ++i)
	{
		godot::String name = this->get_bone_name(i);
		const std::string name_str = name.ascii().get_data();
		const godot::Transform *const pPose = pMsgs->GetBonePos(name_str);

		this->_bonePoses.emplace(i, BonePoseData({name_str, pPose}));
	}
}

//void VMCAvatarSkeletonController::_ready()
//{}

void VMCAvatarSkeletonController::_process(float)
{
	if(this->_boneChangeID != VMCAvatarListener::MsgStore()->BoneChangeID())
	{
		auto *const pMsgs = VMCAvatarListener::MsgStore();

		this->_boneChangeID = VMCAvatarListener::MsgStore()->BoneChangeID();

		for(auto &bone : this->_bonePoses)
		{
			if(bone.second.Pose == nullptr)
			{
				bone.second.Pose = pMsgs->GetBonePos(bone.second.Name);
				if(!bone.second.Pose)
					continue;
			}

			this->set_bone_pose(bone.first, *bone.second.Pose);
		}
	}
}

void VMCAvatarRootController::_register_methods()
{
	//godot::register_method("_ready", &VMCAvatarRootController::_ready);
	godot::register_method("_process", &VMCAvatarRootController::_process);
}

void VMCAvatarRootController::_init()
{
	auto *const pMsgs = VMCAvatarListener::EnsureActiveMsgStore();
	this->_rootChangeID = pMsgs->RootChangeID();

	godot::String transform = Transform.data();
	std::string name = this->get_name().ascii().get_data();

	this->_rootPose = pMsgs->GetBonePos(name);
}

//void VMCAvatarRootController::_ready()
//{}

void VMCAvatarRootController::_process(float)
{
	auto *const pMsgs = VMCAvatarListener::MsgStore();

	if(this->_rootChangeID != pMsgs->RootChangeID())
	{
		this->_rootChangeID = pMsgs->RootChangeID();

		if(this->_rootPose == nullptr)
		{
			std::string name = this->get_name().ascii().get_data();
			this->_rootPose = pMsgs->GetRootPos(name);
			if(!this->_rootPose)
				return;
		}

		this->set_global_transform(*this->_rootPose);
	}
}
