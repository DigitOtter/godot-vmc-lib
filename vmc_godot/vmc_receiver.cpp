#include "vmc_receiver.h"

#include <osc/OscOutboundPacketStream.h>

VmcReceiver::~VmcReceiver()
{
	this->StopUDPServerThread();
}

void VmcReceiver::_register_methods()
{
	godot::register_method("_process", &VmcReceiver::_process);

	godot::register_property("address", &VmcReceiver::SetAddr, &VmcReceiver::GetAddr, godot::String(DEFAULT_ADDRESS.data()));
	godot::register_property("port", &VmcReceiver::SetPort, &VmcReceiver::GetPort, DEFAULT_PORT);

	godot::register_property("vmc_time", &VmcReceiver::_godotVmcTime, -0.f);
	godot::register_property("blend_shapes", &VmcReceiver::_godotBlendShapes, godot::Dictionary());
	godot::register_property("new_blend_shapes", &VmcReceiver::_godotNewBlendShapes, godot::Dictionary());
	godot::register_property("bone_poses", &VmcReceiver::_godotBonePoses, godot::Dictionary());
	godot::register_property("root_poses", &VmcReceiver::_godotRootPoses, godot::Dictionary());
	godot::register_property("camera_pose", &VmcReceiver::_godotCameraPose, godot::Transform());
	godot::register_property("other_data", &VmcReceiver::_godotOtherData, godot::Dictionary());

	godot::register_property("godot_time", &VmcReceiver::_godotElapsedTime, 0.f);
}

void VmcReceiver::_init()
{
	this->_godotOtherData.clear();
	this->_godotElapsedTime = 0.f;
	this->_godotVmcTime = std::numeric_limits<float>::min();
	this->_godotBlendShapes.clear();
	this->_godotNewBlendShapes.clear();
	this->_godotBonePoses.clear();
	this->_godotRootPoses.clear();
	this->_godotCameraPose = godot::Transform::IDENTITY;

	this->_address = DEFAULT_ADDRESS.data();
	this->_port = DEFAULT_PORT;
	this->ChangeEndpoint(this->_address, this->_port);
}

void VmcReceiver::_process(float delta)
{
	this->_godotElapsedTime += delta;

	if(this->_oscUpdated)
	{
		std::lock_guard lock(this->_lockOscBuffer);
		this->_oscUpdated = false;

		const auto &oscBuffer = this->_oscBuffers[!this->_udpRecBuffer];
		const auto &oscSize   = this->_oscPacketSize[!this->_udpRecBuffer];

		osc::ReceivedPacket oscPacket(oscBuffer.data(), oscSize);
		if(oscPacket.IsBundle())
			this->ProcessOSCBundle(osc::ReceivedBundle(oscPacket));
		else
			this->ProcessOSCMessage(osc::ReceivedMessage(oscPacket));
	}
}

void VmcReceiver::SetAddr(godot::String addr)
{
	const auto *pAddr = addr.ascii().get_data();
	if(this->_address != pAddr)
	{
		this->ChangeEndpoint(pAddr, this->_port);
		this->_address = pAddr;
	}
}

godot::String VmcReceiver::GetAddr()
{
	return godot::String(this->_address.data());
}

void VmcReceiver::SetPort(uint16_t port)
{
	if(port != this->_port)
	{
		this->ChangeEndpoint(this->_address, port);
		this->_port = port;
	}
}

uint16_t VmcReceiver::GetPort()
{
	return this->_port;
}

void VmcReceiver::ChangeEndpoint(const std::string &ip, uint16_t port)
{
	this->StopUDPServerThread();

	this->_udpSocket.reset(new UdpSocket());

	this->_endpoint = IpEndpointName(ip.data(), port);
	this->_udpSocket->Bind(this->_endpoint);

	this->StartUDPServerThread();
}

void VmcReceiver::UDPThread()
{
	// Create a local buffer. Copy contents to
	while(!this->_udpThreadBreak)
	{
		auto &curBuffer = this->_oscBuffers[this->_udpRecBuffer];

		const auto size = this->_udpSocket->ReceiveFrom(this->_endpoint, curBuffer.data(), curBuffer.size());
		if(size > 0)
		{
			std::lock_guard lock(this->_lockOscBuffer);

			this->_oscPacketSize[this->_udpRecBuffer] = size;

			this->_udpRecBuffer = !this->_udpRecBuffer;
			this->_oscUpdated = true;
		}
	}

	this->_udpThreadBreak = false;
}

void VmcReceiver::StartUDPServerThread()
{
	std::lock_guard lock(this->_lockOscBuffer);

	this->_oscUpdated = false;

	this->_udpThreadBreak = false;
	this->_udpRecBuffer = 0;
	this->_oscBuffers[0][0] = '\0';
	this->_oscBuffers[0][1] = '\0';
	this->_oscPacketSize[0] = 0;
	this->_oscPacketSize[1] = 0;

	this->_udpThread = std::async(std::launch::async, &VmcReceiver::UDPThread, this);
}

void VmcReceiver::StopUDPServerThread()
{
	if(this->_udpThread.valid() && this->_udpThread.wait_for(std::chrono::microseconds(0)) != std::future_status::ready)
	{
		this->_udpThreadBreak = true;

		// Wait until thread stops. Continue sending empty messages to server to prevent hangups
		UdpTransmitSocket sock(this->_endpoint);

		char buffer[1024];
		osc::OutboundPacketStream empty(buffer, 1024);
		empty << osc::BeginBundleImmediate << osc::EndBundle;

		while(this->_udpThread.wait_for(std::chrono::milliseconds(100)) != std::future_status::ready)
		{	sock.SendTo(this->_endpoint, empty.Data(), empty.Size());	}

		this->_udpThread.get();
	}
}

void VmcReceiver::ProcessOSCBundle(const osc::ReceivedBundle &bundle)
{
	for(auto bundleIt = bundle.ElementsBegin(); bundleIt != bundle.ElementsEnd(); ++bundleIt)
	{
		if(bundleIt->IsBundle())
			this->ProcessOSCBundle(osc::ReceivedBundle(*bundleIt));
		else if(bundleIt->IsMessage())
			this->ProcessOSCMessage(osc::ReceivedMessage(*bundleIt));
	}
}

void VmcReceiver::ProcessOSCMessage(const osc::ReceivedMessage &msg)
{
	if(!this->ProcessVMC(msg))
	{
//		const auto warnMsg = std::string("OSC Message for '") + msg.AddressPattern() + "' not processed";
//		godot::Godot::print(warnMsg.data());

		this->ProcessOSCUnknown(msg);
	}

	// TODO: Add msg to some default dict if no VMC processor was found
}

godot::Variant VmcReceiver::ConvertOSCData(const osc::ReceivedMessage::const_iterator &data)
{
	if(data->IsBool())
		return data->AsBoolUnchecked();
	else if(data->IsChar())
		return data->AsCharUnchecked();
	else if(data->IsDouble())
		return data->AsDoubleUnchecked();
	else if(data->IsFloat())
		return data->AsFloatUnchecked();
	else if(data->IsInt32())
		return data->AsInt32Unchecked();
	else if(data->IsInt64())
		return data->AsInt64Unchecked();
	else if(data->IsString())
		return data->AsStringUnchecked();
	else
	{
		const auto msg = std::string("Unknown osc data type '") + data->TypeTag() + "'";
		godot::Godot::print(msg.data());

		return godot::Variant();
	}
}

void VmcReceiver::ProcessOSCUnknown(const osc::ReceivedMessage &msg)
{
	godot::Array oscData;
	for(auto oscIt = msg.ArgumentsBegin(); oscIt != msg.ArgumentsEnd(); ++oscIt)
	{
		oscData.append(ConvertOSCData(oscIt));
	}

	this->_godotOtherData[msg.AddressPattern()] = oscData;
}

void VmcReceiver::ProcessVMCTime(const osc::ReceivedMessage &msg)
{
	assert(msg.ArgumentCount() == 1);
	this->_godotVmcTime = msg.ArgumentsBegin()->AsFloat();
}

void VmcReceiver::ProcessVMCBlendVal(const osc::ReceivedMessage &msg)
{
	assert(msg.ArgumentCount() == 2);

	auto msgIt = msg.ArgumentsBegin();
	godot::String name = msgIt->AsString();

	this->_godotNewBlendShapes[name] = (++msgIt)->AsFloat();
}

void VmcReceiver::ProcessVMCApplyBlendShapes(const osc::ReceivedMessage &msg)
{
	assert(msg.ArgumentCount() == 0);

	// I didn't find any duplicate function for the dict, so copy all values manually. This is fine as long as the blendshapes are floats
	this->_godotBlendShapes.clear();
	const auto keys = this->_godotNewBlendShapes.keys();
	const auto vals = this->_godotNewBlendShapes.values();

	for(int i=0; i<this->_godotNewBlendShapes.size(); ++i)
	{
		this->_godotBlendShapes[keys[i]] = vals[i];
	}

	this->_godotNewBlendShapes.clear();
}

void VmcReceiver::ProcessVMCBonePos(const osc::ReceivedMessage &msg)
{
	assert(msg.ArgumentCount() == 8);

	auto msgIt = msg.ArgumentsBegin();

	const godot::String name = msgIt->AsString();
	//const godot::Vector3 pos((++msgIt)->AsFloat(), (++msgIt)->AsFloat(), (++msgIt)->AsFloat());
	godot::Vector3 pos;
	pos.x = (++msgIt)->AsFloat();
	pos.y = (++msgIt)->AsFloat();
	pos.z = (++msgIt)->AsFloat();

	//const godot::Quat rot((++msgIt)->AsFloat(), (++msgIt)->AsFloat(), (++msgIt)->AsFloat(), (++msgIt)->AsFloat());
	godot::Quat rot;
	rot.x = (++msgIt)->AsFloat();
	rot.y = (++msgIt)->AsFloat();
	rot.z = (++msgIt)->AsFloat();
	rot.w = (++msgIt)->AsFloat();

	this->_godotBonePoses[name] = godot::Transform(godot::Basis(rot), pos);
}

void VmcReceiver::ProcessVMCRootPose(const osc::ReceivedMessage &msg)
{
	assert(msg.ArgumentCount() >= 8);

	auto msgIt = msg.ArgumentsBegin();

	const godot::String name = msgIt->AsString();
	//const godot::Vector3 pos((++msgIt)->AsFloat(), (++msgIt)->AsFloat(), (++msgIt)->AsFloat());
	godot::Vector3 pos;
	pos.x = (++msgIt)->AsFloat();
	pos.y = (++msgIt)->AsFloat();
	pos.z = (++msgIt)->AsFloat();

	//const godot::Quat rot((++msgIt)->AsFloat(), (++msgIt)->AsFloat(), (++msgIt)->AsFloat(), (++msgIt)->AsFloat());
	godot::Quat rot;
	rot.x = (++msgIt)->AsFloat();
	rot.y = (++msgIt)->AsFloat();
	rot.z = (++msgIt)->AsFloat();
	rot.w = (++msgIt)->AsFloat();

	this->_godotRootPoses[name] = godot::Transform(godot::Basis(rot), pos);
}

void VmcReceiver::ProcessVMCCameraPose(const osc::ReceivedMessage &msg)
{
	assert(msg.ArgumentCount() >= 7);

	auto msgIt = msg.ArgumentsBegin();

	//const godot::Vector3 pos((++msgIt)->AsFloat(), (++msgIt)->AsFloat(), (++msgIt)->AsFloat());
	godot::Vector3 pos;
	pos.x = (++msgIt)->AsFloat();
	pos.y = (++msgIt)->AsFloat();
	pos.z = (++msgIt)->AsFloat();

	//const godot::Quat rot((++msgIt)->AsFloat(), (++msgIt)->AsFloat(), (++msgIt)->AsFloat(), (++msgIt)->AsFloat());
	godot::Quat rot;
	rot.x = (++msgIt)->AsFloat();
	rot.y = (++msgIt)->AsFloat();
	rot.z = (++msgIt)->AsFloat();
	rot.w = (++msgIt)->AsFloat();

	this->_godotCameraPose.set_origin(pos);
	this->_godotCameraPose.set_basis(godot::Basis(rot));
}
