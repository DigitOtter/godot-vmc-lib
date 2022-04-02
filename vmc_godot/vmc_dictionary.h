#ifndef VMC_DICTIONARY_H
#define VMC_DICTIONARY_H

#include <Godot.hpp>
#include <Resource.hpp>
#include <Dictionary.hpp>

#include "singleton.h"
#include "vmc_msg_process_queue.h"

class VmcDictionary
        : public godot::Resource
{
		class VmcDefaultMsgQueue
		        : public VMCMsgProcessor,
		          public Singleton<VmcDefaultMsgQueue>
		{
			public:


			private:
				void ReceiveMessage(const osc::ReceivedMessage &m, const IpEndpointName &remoteEndpoint) override;

				std::mutex _msgLock;
				osc::ReceivedMessage _lastMessage = osc::ReceivedMessage();

				friend Singleton;
		};

	public: GODOT_CLASS(VmcDictionary, godot::Resource);
	public:
		VmcDictionary() = default;

		static void _register_methods();
		void _init();
		void _process();
		void _ready();

	private:
		godot::Dictionary _vmcDict;

		VmcDefaultMsgQueue *EnsureActiveQueue();
		VmcDefaultMsgQueue *ActiveQueue();
};

#endif //VMC_DICTIONARY_H
