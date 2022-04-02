#include "vmc_godot/vmc_msg_process_queue.h"

#include "vmc_godot/vmc_comm.h"

MsgProcessQueue::MsgProcessQueue()
    : _lockProcessors(0)
{}

MsgProcessQueue::~MsgProcessQueue()
{
	try
	{	this->ProcessMapChanges();	}
	catch(std::exception &e)
	{	godot::Godot::print(godot::String("Failed to shutdown message queue: ") + e.what());	}
}

void MsgProcessQueue::RegisterMsgProcessor(const std::string &param, VMCMsgProcessor *pProc)
{
	//this->_lockProcessors.acquire();

	auto res = this->_processors.emplace(param, pProc);
	if(!res.second)
		res.first->second = pProc;

	//this->_lockProcessors.release();
}

void MsgProcessQueue::RegisterDefaultMsgProcessor(VMCMsgProcessor *pProc)
{
	this->_defaultProcessor = pProc;
}

void MsgProcessQueue::UnregisterMsgProcessor(const std::string &param)
{
	auto procIt = this->_processors.find(param);
	if(procIt != this->_processors.end())
	{
		this->_waitingProcMod = true;
		this->_procToUnregister.emplace_back(procIt);
		this->_lockProcessors.acquire();
		this->_lockProcessors.release();
	}
}

void MsgProcessQueue::UnregisterMsgProcessor(const VMCMsgProcessor *pProc)
{
	std::vector<decltype(this->_processors.begin())> toErase;

	for(auto procIt = this->_processors.begin(); procIt != this->_processors.end(); ++procIt)
	{
		if(procIt->second == pProc)
		{
			toErase.push_back(procIt);
		}
	}

	this->_waitingProcMod = true;
	this->_lockProcessors.acquire();
	for(auto &procIt : toErase)
		this->_procToUnregister.emplace_back(procIt);
	if(this->_defaultProcessor == pProc)
		this->_unregisterDefaultProc = true;
	this->_lockProcessors.release();
}

void MsgProcessQueue::ProcessMapChanges()
{
	this->_lockProcessors.release();
	this->_waitingProcMod = false;
	this->_lockProcessors.acquire();

	for(auto &procIt : this->_procToUnregister)
		this->_processors.erase(procIt);

	if(this->_unregisterDefaultProc)
	{
		this->_defaultProcessor = nullptr;
		this->_unregisterDefaultProc = true;
	}
}

VMCMsgProcessor::VMCMsgProcessor(MsgProcessQueue &registrationQueue)
    : _pRegistrationQueue(&registrationQueue)
{
	registrationQueue.RegisterDefaultMsgProcessor(this);
}

VMCMsgProcessor::~VMCMsgProcessor()
{
	try
	{
		if(this->_pRegistrationQueue)
		{
			this->_pRegistrationQueue->UnregisterMsgProcessor(this);
			this->_pRegistrationQueue = nullptr;
		}
	}
	catch(std::exception &e)
	{
		std::cerr << "OSC Msg Processor failed on shutdown: " << e.what() << std::endl;
	}
}

MsgProcessQueue *VMCMsgProcessor::RegisteredQueue() const
{
	return this->_pRegistrationQueue;
}

//VMCMsgProcessQueue::~VMCMsgProcessQueue()
//{
//	try
//	{
//		this->ProcessMapChanges();
//	}
//	catch(std::exception &e)
//	{
//		std::cerr << "OSC Msg Processor Queue failed on shutdown: " << e.what() << std::endl;
//	}
//}

//void VMCMsgProcessQueue::ReceiveMessage(const osc::ReceivedMessage &m, const IpEndpointName &)
//{
//	// Erase already processed messages
//	this->_messages.erase(this->_messages.begin(), this->_lastMsgProcessed);
//	this->_lastMsgProcessed = this->_messages.begin();

//	// Check message
//	this->_messages.push_back(m);
//}

//VMCMsgProcessQueue::VMCMsgProcessQueue()
//    : _lastMsgProcessed(this->_messages.end())
//{}
