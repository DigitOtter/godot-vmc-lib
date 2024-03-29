#include "vmc_receiver.h"

#include <osc/OscOutboundPacketStream.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/wrapped.hpp>
#include <godot_cpp/core/error_macros.hpp>


using namespace godot;

VmcReceiver::~VmcReceiver()
{
	this->StopUDPServerThread();
}

void VmcReceiver::_bind_methods()
{
	// address Property
	godot::ClassDB::bind_method(godot::D_METHOD("_set_addr", "addr"), &VmcReceiver::SetAddr);
	godot::ClassDB::bind_method(godot::D_METHOD("_get_addr"), &VmcReceiver::GetAddr);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::STRING, "address"), "_set_addr", "_get_addr");

	// port Property
	godot::ClassDB::bind_method(godot::D_METHOD("_set_port", "port"), &VmcReceiver::GodotSetPort);
	godot::ClassDB::bind_method(godot::D_METHOD("_get_port"), &VmcReceiver::GodotGetPort);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::INT, "port"), "_set_port", "_get_port");

	// vmc_time Property
	godot::ClassDB::bind_method(godot::D_METHOD("_set_vmc_time", "vmc_time"), &VmcReceiver::SetVmcTime);
	godot::ClassDB::bind_method(godot::D_METHOD("_get_vmc_time"), &VmcReceiver::GetVmcTime);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::FLOAT, "vmc_time"), "_set_vmc_time", "_get_vmc_time");

	// blend_shapes Property
	godot::ClassDB::bind_method(godot::D_METHOD("_set_blend_shapes", "blend_shapes"), &VmcReceiver::SetBlendShapes);
	godot::ClassDB::bind_method(godot::D_METHOD("_get_blend_shapes"), &VmcReceiver::GetBlendShapes);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::DICTIONARY, "blend_shapes"), "_set_blend_shapes", "_get_blend_shapes");

	// bone_poses Property
	godot::ClassDB::bind_method(godot::D_METHOD("_set_bone_poses", "bone_poses"), &VmcReceiver::SetBonePoses);
	godot::ClassDB::bind_method(godot::D_METHOD("_get_bone_poses"), &VmcReceiver::GetBonePoses);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::DICTIONARY, "bone_poses"), "_set_bone_poses", "_get_bone_poses");

	// root_poses Property
	godot::ClassDB::bind_method(godot::D_METHOD("_set_root_poses", "root_poses"), &VmcReceiver::SetRootPoses);
	godot::ClassDB::bind_method(godot::D_METHOD("_get_root_poses"), &VmcReceiver::GetRootPoses);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::DICTIONARY, "root_poses"), "_set_root_poses", "_get_root_poses");

	// camera_pose Property
	godot::ClassDB::bind_method(godot::D_METHOD("_set_camera_pose", "camera_pose"), &VmcReceiver::SetCameraPose);
	godot::ClassDB::bind_method(godot::D_METHOD("_get_camera_pose"), &VmcReceiver::GetCameraPose);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::TRANSFORM3D, "camera_pose"), "_set_camera_pose", "_get_camera_pose");

	// other_data Property
	godot::ClassDB::bind_method(godot::D_METHOD("_set_other_data", "other_data"), &VmcReceiver::SetOtherData);
	godot::ClassDB::bind_method(godot::D_METHOD("_get_other_data"), &VmcReceiver::GetOtherData);
	ADD_PROPERTY(godot::PropertyInfo(godot::Variant::DICTIONARY, "other_data"), "_set_other_data", "_get_other_data");
}

void VmcReceiver::_ready()
{
	this->_godotOtherData.clear();
	this->_godotVmcTime = std::numeric_limits<float>::min();
	this->_godotBlendShapes.clear();
	this->_godotNewBlendShapes.clear();
	this->_godotBonePoses.clear();
	this->_godotRootPoses.clear();
	this->_godotCameraPose = godot::Transform3D();
	this->_godotCameraPose.set_origin(godot::Vector3(0,0,0));

//	this->_address = DEFAULT_ADDRESS.data();
//	this->_port = DEFAULT_PORT;
	this->ChangeEndpoint(this->_address, this->_port);
}

void VmcReceiver::_process(double /*delta*/)
{
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
	const std::string ascii_addr = addr.ascii().get_data();
	if(this->_address != ascii_addr)
	{
		this->ChangeEndpoint(ascii_addr, this->_port);
		this->_address = ascii_addr;
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

uint16_t VmcReceiver::GetPort() const
{
	return this->_port;
}

void VmcReceiver::ChangeEndpoint(const std::string &ip, uint16_t port)
{
	this->StopUDPServerThread();

	this->_udpSocket.reset(new UdpSocket());

	this->_endpoint = IpEndpointName(ip.data(), port);

	// Skip binding if node is running in editor
	if(godot::Engine::get_singleton()->is_editor_hint())
		return;

	try
	{
		this->_udpSocket->Bind(this->_endpoint);
	}
	catch(const std::runtime_error &)
	{
		const std::string err_msg = "Failed to bind to: " + ip + ":" + std::to_string(port);
		WARN_PRINT(err_msg.data());
		return;		// Skip server start
	}

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
	// Try to process message as vmc. If not possible, add msg data to _godotOtherData
	if(!this->ProcessVMC(msg))
		this->ProcessOSCUnknown(msg);
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
		return (int32_t)data->AsInt32Unchecked();
	else if(data->IsInt64())
		return (int64_t)data->AsInt64Unchecked();
	else if(data->IsString())
		return data->AsStringUnchecked();
	else
	{
		const auto msg = std::string("Unknown osc data type '") + data->TypeTag() + "'";
		WARN_PRINT(msg.data());

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
	godot::Quaternion rot;
	rot.x = (++msgIt)->AsFloat();
	rot.y = (++msgIt)->AsFloat();
	rot.z = (++msgIt)->AsFloat();
	rot.w = (++msgIt)->AsFloat();

	this->_godotBonePoses[name] = godot::Transform3D(godot::Basis(rot), pos);
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
	godot::Quaternion rot;
	rot.x = (++msgIt)->AsFloat();
	rot.y = (++msgIt)->AsFloat();
	rot.z = (++msgIt)->AsFloat();
	rot.w = (++msgIt)->AsFloat();

	this->_godotRootPoses[name] = godot::Transform3D(godot::Basis(rot), pos);
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
	godot::Quaternion rot;
	rot.x = (++msgIt)->AsFloat();
	rot.y = (++msgIt)->AsFloat();
	rot.z = (++msgIt)->AsFloat();
	rot.w = (++msgIt)->AsFloat();

	this->_godotCameraPose.set_origin(pos);
	this->_godotCameraPose.set_basis(godot::Basis(rot));
}
