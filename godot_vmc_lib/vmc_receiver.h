#ifndef VMC_RECEIVER_H
#define VMC_RECEIVER_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <future>
#include <ip/UdpSocket.h>
#include <functional>
#include <osc/OscReceivedElements.h>

#include "vmc_packets.h"


class VmcReceiver
        : public godot::Node
{
		static constexpr std::string_view DEFAULT_ADDRESS = "localhost";
		static constexpr uint16_t DEFAULT_PORT = 39539;

		static constexpr uint32_t OSC_BUFFER_SIZE = 8192;

	GDCLASS(VmcReceiver, godot::Node);

	public:
		VmcReceiver() = default;
		~VmcReceiver();

		static void _bind_methods();
		void _init();
		void _process(double delta) override;

	private:
		float              _godotVmcTime;
		godot::Dictionary  _godotBlendShapes;
		godot::Dictionary  _godotBonePoses;
		godot::Dictionary  _godotRootPoses;
		godot::Transform3D _godotCameraPose;
		godot::Dictionary  _godotOtherData;

		// New Blend shapes. Apply after receiving BLEND_SHAPES_APPLY
		godot::Dictionary _godotNewBlendShapes;

		std::string    _address = DEFAULT_ADDRESS.data();
		uint16_t       _port = DEFAULT_PORT;
		IpEndpointName _endpoint;

		std::mutex  _lockOscBuffer;
		bool		_oscUpdated = false;

		// Use 2 alternating osc buffers for receive. Prevents packet skip
		bool		_udpRecBuffer = 0;
		using osc_buffer_t = std::array<char, OSC_BUFFER_SIZE>;
		std::array<osc_buffer_t, 2> _oscBuffers;
		std::array<std::size_t, 2>  _oscPacketSize;

		std::unique_ptr<UdpSocket> _udpSocket;
		std::future<void> _udpThread;
		volatile bool _udpThreadBreak = false;

		// Godot Interface for _vmc_time
		void  SetVmcTime(float vmc_time)	{	this->_godotVmcTime = vmc_time;	};
		float GetVmcTime() const			{	return this->_godotVmcTime;	};

		// Godot Interface for _blend_shapes
		void  SetBlendShapes(godot::Dictionary blend_shapes)	{	this->_godotBlendShapes = blend_shapes;	};
		godot::Dictionary GetBlendShapes() const				{	return this->_godotBlendShapes;	};

		// Godot Interface for _bone_poses
		void  SetBonePoses(godot::Dictionary bone_poses)	{	this->_godotBonePoses = bone_poses;	};
		godot::Dictionary GetBonePoses() const				{	return this->_godotBonePoses;	};

		// Godot Interface for _root_poses
		void  SetRootPoses(godot::Dictionary root_poses)	{	this->_godotRootPoses = root_poses;	};
		godot::Dictionary GetRootPoses() const				{	return this->_godotRootPoses;	};

		// Godot Interface for _camera_pose
		void  SetCameraPose(godot::Transform3D camera_pose)	{	this->_godotCameraPose = camera_pose;	};
		godot::Transform3D GetCameraPose() const			{	return this->_godotCameraPose;	};

		// Godot Interface for _other_data
		void  SetOtherData(godot::Dictionary other_data)	{	this->_godotOtherData = other_data;	};
		godot::Dictionary GetOtherData() const				{	return this->_godotOtherData;	};

		// Godot Interface for _other_data
		void GodotSetPort(int port)	{	this->_port = port;	};
		int GodotGetPort() const	{	return this->_port;	};


		// Godot Interface for _address
		void SetAddr(godot::String addr);
		godot::String GetAddr();

		// Godot Interface for _port
		void SetPort(uint16_t port);
		uint16_t GetPort();

		// OSC Network Receiver
		void ChangeEndpoint(const std::string &ip, uint16_t port);
		void UDPThread();
		void StartUDPServerThread();
		void StopUDPServerThread();

		// Process received osc messages
		void ProcessOSCBundle(const osc::ReceivedBundle &bundle);
		void ProcessOSCMessage(const osc::ReceivedMessage &msg);

		static godot::Variant ConvertOSCData(const osc::ReceivedMessage::const_iterator &data);
		void ProcessOSCUnknown(const osc::ReceivedMessage &msg);

		void ProcessVMCTime(const osc::ReceivedMessage &msg);
		void ProcessVMCBlendVal(const osc::ReceivedMessage &msg);
		void ProcessVMCApplyBlendShapes(const osc::ReceivedMessage &msg);
		void ProcessVMCBonePos(const osc::ReceivedMessage &msg);
		void ProcessVMCRootPose(const osc::ReceivedMessage &msg);
		void ProcessVMCCameraPose(const osc::ReceivedMessage &msg);

		using process_vmc_fcn_t = void(VmcReceiver::*)(const osc::ReceivedMessage&);
		using process_vmc_fcn_srray_t = std::pair<std::string_view, process_vmc_fcn_t>;
		static constexpr process_vmc_fcn_srray_t VMC_PACKET_FCNS[] =
		{
		    {vmc::marionette::EXT_T,           &VmcReceiver::ProcessVMCTime},
		    {vmc::marionette::EXT_BLEND_VAL,   &VmcReceiver::ProcessVMCBlendVal},
		    {vmc::marionette::EXT_BLEND_APPLY, &VmcReceiver::ProcessVMCApplyBlendShapes},
		    {vmc::marionette::EXT_BONE_POS,    &VmcReceiver::ProcessVMCBonePos},
		    {vmc::marionette::EXT_ROOT_POS,    &VmcReceiver::ProcessVMCRootPose},
		    {vmc::marionette::EXT_CAM,	       &VmcReceiver::ProcessVMCCameraPose},
		};

		inline bool ProcessVMC(const osc::ReceivedMessage &msg)
		{
			constexpr auto FCNS = sizeof(VMC_PACKET_FCNS)/sizeof(process_vmc_fcn_srray_t);
			for(unsigned long i = 0; i < FCNS; ++i)
			{
				if(std::strcmp(VMC_PACKET_FCNS[i].first.data(), msg.AddressPattern()) == 0)
				{
					std::invoke(VMC_PACKET_FCNS[i].second, this, msg);
					return true;
				}
			}

			return false;
		}
};

#endif //VMC_RECEIVER_H
