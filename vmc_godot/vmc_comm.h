#ifndef VMC_COMM_H
#define VMC_COMM_H

#include <Godot.hpp>
#include <Resource.hpp>

#include <ip/UdpSocket.h>
#include <osc/OscPacketListener.h>

#include <future>
#include <list>
#include <map>
#include <semaphore>
#include <vector>

#include "vmc_godot/vmc_msg_process_queue.h"


class VMCComm
        : public godot::Resource
{
		class VMCListener
		        : private osc::OscPacketListener,
		          public MsgProcessQueue,
		          public Singleton<VMCListener>
		{
			public:
				~VMCListener() override;

				void StartOSCServerThread();
				void StopOSCServerThread();

				void ProcessMessage(const osc::ReceivedMessage &m, const IpEndpointName &remoteEndpoint) override;

				void ChangeEndpoint(const std::string &ip, uint16_t port);

			private:
				VMCListener(const std::string &ip, uint16_t port);

				IpEndpointName _endpoint;
				UdpListeningReceiveSocket _listenSocket;

				std::mutex _lockThread;
				std::future<void> _oscServerThread;

				friend Singleton<VMCListener>;
		};

		static constexpr std::string_view DefaultAddr = "127.0.0.1";
		static constexpr uint16_t DefaultPort = 11753;

	public:
		GODOT_CLASS(VMCComm, godot::Resource)

	public:
		static VMCListener *EnsureActiveListener();
		static VMCListener *ListenerInstance();
		static void DeleteListenerInstance();

		static void _register_methods();
		void _init();
		void _process(float delta);

	private:
		void SetAddr(godot::String addr);
		godot::String GetAddr();

		void SetPort(uint16_t port);
		uint16_t GetPort();

		std::string _addr = "127.0.0.1";
		uint16_t _port = 11573;

		float _time = 0.f;
		float _sumDelta = 0.f;
		godot::Transform _rootTf;
		float _blendVal = 0.f;
};

namespace godot
{
    using VMCComm = ::VMCComm;
}

#endif // VMC_COMM_H
