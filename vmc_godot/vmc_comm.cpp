#include "vmc_godot/vmc_comm.h"

#include "vmc_godot/vmc_packets.h"

#include <iostream>
#include <ip/IpEndpointName.h>
#include <osc/OscOutboundPacketStream.h>
#include <cmath>


VMCComm::VMCListener::VMCListener(const std::string &addr, uint16_t port)
    : _endpoint(addr.data(), port),
      _listenSocket(this->_endpoint, dynamic_cast<osc::OscPacketListener*>(this))
{
	this->StartOSCServerThread();
}

VMCComm::VMCListener::~VMCListener()
{
	try
	{
		this->StopOSCServerThread();
		this->ProcessMapChanges();
	}
	catch(std::exception &e)
	{
		std::cerr << "OSC UDP Listener failed on shutdown: " << e.what() << std::endl;
	}
}

void VMCComm::VMCListener::StartOSCServerThread()
{
	if(this->_lockThread.try_lock())
		this->_oscServerThread = std::async(std::launch::async, &UdpListeningReceiveSocket::Run, this->_listenSocket);
}

void VMCComm::VMCListener::StopOSCServerThread()
{
	if(this->_oscServerThread.wait_for(std::chrono::microseconds(0)) == std::future_status::timeout)
	{
		// Stop socket
		this->_listenSocket.Break();

		// Wait until thread stops. Continue sending empty messages to server to prevent hangups
		UdpTransmitSocket sock(this->_endpoint);

		char buffer[1024];
		osc::OutboundPacketStream empty(buffer, 1024);
		empty << osc::BeginBundleImmediate << osc::EndBundle;

		while(this->_oscServerThread.wait_for(std::chrono::microseconds(10)) == std::future_status::timeout)
		{
			sock.SendTo(this->_endpoint, empty.Data(), empty.Size());
		}

		this->_lockThread.unlock();
	}
}

void VMCComm::VMCListener::ProcessMessage(const osc::ReceivedMessage &m, const IpEndpointName &remoteEndpoint)
{
	std::cout << "Received Msg: " << m.AddressPattern() << std::endl;

	if(this->AwaitingProcMapChanges())
		this->ProcessMapChanges();

	// TODO: Check remoteEndpoint against Whitelist

	auto procIt = this->_processors.find(m.AddressPattern());
	if(procIt != this->_processors.end())
		procIt->second->ReceiveMessage(m, remoteEndpoint);
}

void VMCComm::VMCListener::ChangeEndpoint(const std::string &ip, uint16_t port)
{
	this->StopOSCServerThread();
	this->_endpoint = IpEndpointName(ip.data(), port);
	this->_listenSocket.LocalEndpointFor(this->_endpoint);
	this->StartOSCServerThread();
}

VMCComm::VMCListener *VMCComm::EnsureActiveListener()
{
	if(!VMCComm::VMCListener::Instance())
	{
		VMCListener::Reset(DefaultAddr.data(), DefaultPort);
		VMCListener::Instance()->StartOSCServerThread();
	}

	return VMCComm::VMCListener::Instance();
}

VMCComm::VMCListener *VMCComm::ListenerInstance()
{
	assert(VMCComm::VMCListener::Instance());
	return VMCComm::VMCListener::Instance();
}

void VMCComm::DeleteListenerInstance()
{
	VMCListener::Delete();
}

void VMCComm::_register_methods()
{
	godot::register_method((char*)"_process", &VMCComm::_process);

	godot::String dat(DefaultAddr.data());
	godot::register_property("address", &VMCComm::SetAddr, &VMCComm::GetAddr, dat);
	godot::register_property("port", &VMCComm::SetPort, &VMCComm::GetPort, (uint16_t)DefaultPort);
}

void VMCComm::_init()
{
	if(auto *pList = VMCListener::Instance())
		pList->ChangeEndpoint(this->_addr, this->_port);
	else
		VMCListener::Reset(this->_addr, this->_port)->StartOSCServerThread();

	this->_time = 0.f;
	this->_sumDelta = 0.f;
	this->_rootTf.basis.set_euler({0,0,0});
	this->_rootTf.origin = {0,0,0};
	this->_blendVal = 0.f;
}

void VMCComm::_process(float /*delta*/)
{
//	this->_sumDelta += delta;

//	if(this->_sumDelta > 0.1f)
//	{
//		this->_time += this->_sumDelta;
//		this->_sumDelta = 0.f;

//		std::cout << "Sending root data" << std::endl;

//		this->_rootTf.origin.x = this->_time * 0.1f;
//		const godot::Quat quat = this->_rootTf.basis;

//		char oscDat[2048];
//		osc::OutboundPacketStream rootPack(oscDat, sizeof(oscDat));
//		rootPack << osc::BeginBundleImmediate <<
//		                osc::BeginMessage(vmc::marionette::EXT_ROOT_POS.data()) <<
//		                    "Root" <<
//		                    this->_rootTf.origin.x <<
//		                    this->_rootTf.origin.y <<
//		                    this->_rootTf.origin.z <<
//		                    quat.x <<
//		                    quat.y <<
//		                    quat.z <<
//		                    quat.w <<
//		                osc::EndMessage <<
//		            osc::EndBundle;

//		std::cout << rootPack.Data() << std::endl;

//		UdpTransmitSocket sock(IpEndpointName(this->_addr.data(), this->_port));
//		sock.Send(rootPack.Data(), rootPack.Size());

//		std::cout << "Sending bone data" << std::endl;

//		this->_rootTf.origin.x = this->_time * 0.1f;
//		//const godot::Quat quat = this->_rootTf.basis;

//		//char oscDat[2048];
//		//osc::OutboundPacketStream rootPack(oscDat, sizeof(oscDat));
//		rootPack.Clear();
//		rootPack << osc::BeginBundleImmediate <<
//		                osc::BeginMessage(vmc::marionette::EXT_BONE_POS.data()) <<
//		                    "left_leg" <<
//		                    this->_rootTf.origin.x <<
//		                    this->_rootTf.origin.y <<
//		                    this->_rootTf.origin.z <<
//		                    quat.x <<
//		                    quat.y <<
//		                    quat.z <<
//		                    quat.w <<
//		                osc::EndMessage <<
//		            osc::EndBundle;

//		std::cout << rootPack.Data() << std::endl;

//		//UdpTransmitSocket sock(IpEndpointName(this->_addr.data(), this->_port));
//		sock.Send(rootPack.Data(), rootPack.Size());

//		std::cout << "Sending blend data" << std::endl;

//		this->_blendVal = this->_time * 0.1f;
//		this->_blendVal = std::fmod(this->_blendVal, 1.f);

//		//char oscDat[2048];
//		//osc::OutboundPacketStream rootPack(oscDat, sizeof(oscDat));
//		rootPack.Clear();
//		rootPack << osc::BeginBundleImmediate <<
//		                osc::BeginMessage(vmc::marionette::EXT_BLEND_VAL.data()) <<
//		                    "morph_1" <<
//		                    this->_blendVal <<
//		                osc::EndMessage <<
//		                osc::BeginMessage(vmc::marionette::EXT_BLEND_APPLY.data()) <<
//		                osc::EndMessage <<
//		            osc::EndBundle;

//		std::cout << rootPack.Data() << std::endl;

//		//UdpTransmitSocket sock(IpEndpointName(this->_addr.data(), this->_port));
//		sock.Send(rootPack.Data(), rootPack.Size());
//	}
}

void VMCComm::SetAddr(godot::String addr)
{
	const auto *pAddr = addr.ascii().get_data();
	if(this->_addr != pAddr)
	{
		if(auto *pList = ListenerInstance())
			pList->ChangeEndpoint(pAddr, this->_port);
	}

	this->_addr = pAddr;
}

godot::String VMCComm::GetAddr()
{
	return godot::String(this->_addr.data());
}

void VMCComm::SetPort(uint16_t port)
{
	if(port != this->_port)
	{
		if(auto *pList = ListenerInstance())
			pList->ChangeEndpoint(this->_addr, port);
	}

	this->_port = port;
}

uint16_t VMCComm::GetPort()
{
	return this->_port;
}
