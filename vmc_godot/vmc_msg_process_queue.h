#ifndef VMC_MSG_PROCESS_QUEUE_H
#define VMC_MSG_PROCESS_QUEUE_H

#include <osc/OscReceivedElements.h>
#include <osc/OscPacketListener.h>

#include <Godot.hpp>
#include <String.hpp>

#include <list>
#include <map>
#include <semaphore>

#include "vmc_godot/singleton.h"

class VMCMsgProcessor;

class MsgProcessQueue
{
	public:
		MsgProcessQueue();
		~MsgProcessQueue();

		void RegisterMsgProcessor(const std::string &param, VMCMsgProcessor *pProc);
		void RegisterDefaultMsgProcessor(VMCMsgProcessor *pProc);
		void UnregisterMsgProcessor(const std::string &param);
		void UnregisterMsgProcessor(const VMCMsgProcessor *pProc);

	private:
		std::binary_semaphore _lockProcessors;
		using addr_proc_map_t = std::map<std::string, VMCMsgProcessor*>;
		volatile bool _waitingProcMod = false;
		std::list<typename addr_proc_map_t::iterator> _procToUnregister;

	protected:
		addr_proc_map_t _processors;
		VMCMsgProcessor *_defaultProcessor = nullptr;
		bool _unregisterDefaultProc = 0;

		inline const volatile bool &AwaitingProcMapChanges() const
		{	return this->_waitingProcMod;	}
		void ProcessMapChanges();
};

class VMCMsgProcessor
{
	public:
		VMCMsgProcessor() = default;
		VMCMsgProcessor(const std::string &param, MsgProcessQueue &registrationQueue);
		VMCMsgProcessor(MsgProcessQueue &registrationQueue);
		virtual ~VMCMsgProcessor();
		virtual void ReceiveMessage(const osc::ReceivedMessage &m, const IpEndpointName &remoteEndpoint) = 0;

	protected:
		MsgProcessQueue *RegisteredQueue() const;

	private:
		MsgProcessQueue *_pRegistrationQueue = nullptr;
};

//class VMCMsgProcessQueue
//        : public VMCMsgProcessor,
//          public MsgProcessQueue
//{
//	public:
//		virtual ~VMCMsgProcessQueue() override;

//		void ReceiveMessage(const osc::ReceivedMessage &m, const IpEndpointName &remoteEndpoint) override;

//	private:
//		VMCMsgProcessQueue();

//		using osc_msg_list_t = std::list<osc::ReceivedMessage>;
//		osc_msg_list_t _messages;
//		osc_msg_list_t::iterator _lastMsgProcessed;
//};

#endif // VMC_MSG_PROCESS_QUEUE_H
