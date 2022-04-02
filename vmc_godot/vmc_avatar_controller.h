#ifndef VMC_AVATAR_CONTROLLER_H
#define VMC_AVATAR_CONTROLLER_H

#include <Godot.hpp>
#include <MeshInstance.hpp>
#include <Node.hpp>
#include <Skeleton.hpp>
#include <Spatial.hpp>

#include "vmc_godot/singleton.h"
#include "vmc_godot/vmc_comm.h"

namespace vmc_avatar
{
    struct ExtBonePos
	{
	};
}

class VMCAvatarListener
        : public godot::Node
{
	private:
		class VMCAvatarMsgQueue
		        : public VMCMsgProcessor,
		          public Singleton<VMCAvatarMsgQueue>
		{
			public:
				~VMCAvatarMsgQueue() override = default;

				void ReceiveMessage(const osc::ReceivedMessage &m, const IpEndpointName &remoteEndpoint) override;

				bool RootPosAvailable(const std::string &name) const;
				void SetRootPos(const std::string &name, godot::Transform transform);
				const godot::Transform *GetRootPos(const std::string &name) const;

				bool BonePosAvailable(const std::string &name) const;
				void SetBonePos(const std::string &name, godot::Transform transform);
				const godot::Transform *GetBonePos(const std::string &name) const;

				bool BlendShapeAvailable(const std::string &name) const;
				void SetBlendShape(const std::string &name, float value);
				const float *GetBlendShape(const std::string &name);

				inline const unsigned int &RootChangeID() const
				{	return this->_rootChangeID;	}
				inline const unsigned int &BoneChangeID() const
				{	return this->_boneChangeID;	}
				inline const unsigned int &BlendShapeChangeID() const
				{	return this->_blendShapeChangeID;	}

			private:
				VMCAvatarMsgQueue();

				std::map<std::string, godot::Transform> _rootPos;
				std::map<std::string, godot::Transform> _bonePos;
				std::map<std::string, float> _currentBlendShapes;
				std::map<std::string, float> _newBlendShapes;

				unsigned int _rootChangeID = 0;
				unsigned int _boneChangeID = 0;
				unsigned int _blendShapeChangeID = 0;

				friend Singleton;
		};

	public:	GODOT_CLASS(VMCAvatarListener, godot::Node);

	public:
		static void _register_methods();
		void _init();
		void _process();
		void _ready();

		static VMCAvatarMsgQueue *EnsureActiveMsgStore();
		static VMCAvatarMsgQueue *MsgStore();

	private:
};

class VMCAvatarController
        : public godot::Resource
{
	public: GODOT_CLASS(VMCAvatarController, godot::Resource);
	public:
		static void _register_methods();
		void _init();

	private:
		std::vector<std::string> _ocmBlendShapeNames;
		std::map<int64_t, std::string> _ocmBoneNames;
};

class VMCAvatarBlendShapeController
        : public godot::MeshInstance
{
		static constexpr std::string_view BlendShapePrefix = "blend_shapes/";

		struct BlendShapeData
		{
			std::string Name;
			const float *Val = nullptr;
		};

	public: GODOT_CLASS(VMCAvatarBlendShapeController, godot::MeshInstance);
	public:
		static void _register_methods();
		void _init();
		//void _ready();
		void _process(float delta);

	private:
		unsigned int _blendShapeChangeID = 0;
		std::map<godot::String, BlendShapeData> _shapeVals;
};

class VMCAvatarSkeletonController
        : public godot::Skeleton
{
		struct BonePoseData
		{
			std::string Name;
			const godot::Transform *Pose = nullptr;
		};

	public: GODOT_CLASS(VMCAvatarSkeletonController, godot::Skeleton);
	public:
		static void _register_methods();
		void _init();
		//void _ready();
		void _process(float delta);

	private:
		unsigned int _boneChangeID = 0;
		std::map<int64_t, BonePoseData> _bonePoses;
};

class VMCAvatarRootController
        : public godot::Spatial
{
		static constexpr std::string_view Transform = "transform";

	public: GODOT_CLASS(VMCAvatarRootController, godot::Spatial);
	public:
		static void _register_methods();
		void _init();
		//void _ready();
		void _process(float delta);

	private:
		unsigned int _rootChangeID = 0;
		const godot::Transform *_rootPose;
};



#endif // VMC_AVATAR_CONTROLLER_H
