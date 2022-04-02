#ifndef VMC_RECEIVER_H
#define VMC_RECEIVER_H

#include <Godot.hpp>
#include <Dictionary.hpp>
#include <Resource.hpp>
#include <future>
#include <ip/UdpSocket.h>
#include <functional>
#include <osc/OscReceivedElements.h>

#include "singleton.h"
#include "vmc_packets.h"


class VmcReceiver
        : public godot::Resource
{
		static constexpr std::string_view DEFAULT_ADDRESS = "localhost";
		static constexpr uint16_t DEFAULT_PORT = 39539;

		static constexpr uint32_t OSC_BUFFER_SIZE = 8192;

	public: GODOT_CLASS(VmcReceiver, godot::Resource)
	public:
		VmcReceiver() = default;
		~VmcReceiver();

		static void _register_methods();
		void _init();
		void _process(float delta);

	private:
		float			  _godotElapsedTime;
		float             _godotVmcTime;
		godot::Dictionary _godotBlendShapes;
		godot::Dictionary _godotBonePoses;
		godot::Dictionary _godotRootPoses;
		godot::Transform  _godotCameraPose;
		godot::Dictionary _godotOtherData;

		// New Blend shapes. Apply after receiving BLEND_SHAPES_APPLY
		godot::Dictionary _godotNewBlendShapes;

		std::string _address;
		uint16_t    _port;
		IpEndpointName _endpoint;

		std::mutex  _lockOscBuffer;
		bool		_oscUpdated = false;
		bool		_udpRecBuffer = 0;
		using osc_buffer_t = std::array<char, OSC_BUFFER_SIZE>;
		std::array<osc_buffer_t, 2> _oscBuffers;
		std::array<std::size_t, 2>  _oscPacketSize;

		std::unique_ptr<UdpSocket> _udpSocket;
		std::future<void> _udpThread;
		volatile bool _udpThreadBreak = false;

		void SetAddr(godot::String addr);
		godot::String GetAddr();

		void SetPort(uint16_t port);
		uint16_t GetPort();

		void ChangeEndpoint(const std::string &ip, uint16_t port);
		void UDPThread();
		void StartUDPServerThread();
		void StopUDPServerThread();

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
