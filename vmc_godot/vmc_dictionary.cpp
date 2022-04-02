#include "vmc_dictionary.h"

#include "vmc_comm.h"

VmcDictionary::VmcDefaultMsgQueue::VmcDefaultMsgQueue()
    : VMCMsgProcessor(*VMCComm::EnsureActiveListener()),
{}

void VmcDictionary::VmcDefaultMsgQueue::ReceiveMessage(const osc::ReceivedMessage &m, const IpEndpointName &remoteEndpoint)
{

}



void VmcDictionary::_register_methods()
{
	godot::register_property("values", &VmcDictionary::_vmcDict, godot::Dictionary());
}

VmcDictionary::VmcDefaultMsgQueue *VmcDictionary::EnsureActiveQueue()
{
	if(!VmcDefaultMsgQueue::Instance())
		return VmcDefaultMsgQueue::Reset();
	return VmcDefaultMsgQueue::Instance();
}

VmcDictionary::VmcDefaultMsgQueue *VmcDictionary::ActiveQueue()
{
	return VmcDefaultMsgQueue::Instance();
}
