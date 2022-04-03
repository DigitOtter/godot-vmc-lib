#ifndef VMC_PACKETS_H
#define VMC_PACKETS_H

#include <string>
#include <string_view>
#include <vector>

#ifndef VMC_VERSION
#define VMC_VERSION 20700
#endif

#if VMC_VERSION >= 20700

namespace vmc
{
    namespace marionette
	{
	    static constexpr std::string_view EXT_OK             = "/VMC/Ext/OK";
		static constexpr std::string_view EXT_T              = "/VMC/Ext/T";
		static constexpr std::string_view EXT_ROOT_POS       = "/VMC/Ext/Root/Pos";
		static constexpr std::string_view EXT_BONE_POS       = "/VMC/Ext/Bone/Pos";
		static constexpr std::string_view EXT_BLEND_VAL      = "/VMC/Ext/Blend/Val";
		static constexpr std::string_view EXT_BLEND_APPLY    = "/VMC/Ext/Blend/Apply";
		static constexpr std::string_view EXT_CAM            = "/VMC/Ext/Cam";
		static constexpr std::string_view EXT_CON            = "/VMC/Ext/Con";
		static constexpr std::string_view EXT_KEY            = "/VMC/Ext/Key";
		static constexpr std::string_view EXT_MIDI_NOTE      = "/VMC/Ext/Midi/Note";
		static constexpr std::string_view EXT_MIDI_CC_VAL    = "/VMC/Ext/Midi/CC/Val";
		static constexpr std::string_view EXT_MIDI_CC_BIT    = "/VMC/Ext/Midi/CC/Bit";
		static constexpr std::string_view EXT_HMD_POS        = "/VMC/Ext/Hmd/Pos";
		static constexpr std::string_view EXT_CON_POS        = "/VMC/Ext/Con/Pos";
		static constexpr std::string_view EXT_TRA_POS        = "/VMC/Ext/Tra/Pos";
		static constexpr std::string_view EXT_RCV            = "/VMC/Ext/Rcv";
		static constexpr std::string_view EXT_LIGHT          = "/VMC/Ext/Light";
		static constexpr std::string_view EXT_REMOTE         = "/VMC/Ext/Remote";
		static constexpr std::string_view EXT_OPT            = "/VMC/Ext/Opt";
		static constexpr std::string_view EXT_SETTING_COLOR  = "/VMC/Ext/Setting/Color";
		static constexpr std::string_view EXT_SETTING_WIN    = "/VMC/Ext/Setting/Win";
		static constexpr std::string_view EXT_CONFIG         = "/VMC/Ext/Config";
		static constexpr std::string_view EXT_THRU           = "/VMC/Thru/";

		struct ExtOK
		{
			int Loaded;
			int CalibrationState;
			int TrackingStatus;
		};

		struct ExtT
		{
			float T;
		};

		struct ExtBonePos
		{
			std::string Name;
			float LinX;
			float LinY;
			float LinZ;
			float AngX;
			float AngY;
			float AngZ;
			float AngW;
		};

		struct ExtBlendVal
		{
			std::string Name;
			float Value;
		};

		struct ExtCam
		{
			std::string Name;
			float LinX;
			float LinY;
			float LinZ;
			float AngX;
			float AngY;
			float AngZ;
			float AngW;
			float FOV;
		};

		struct ExtCon
		{
			int Active;
			std::string Name;
			int IsLeft;
			int IsTouch;
			int IsAxis;
			float AxisX;
			float AxisY;
			float AxisZ;
		};

		struct ExtKey
		{
			int Active;
			std::string Name;
			int KeyCode;
		};

		struct ExtMidiNote
		{
			int Active;
			int Channel;
			int Note;
			float Velocity;
		};

		struct ExtMidiCCBit
		{
			int Knob;
			int Active;
		};

		struct ExtHmdPosLocal
		{
			std::string Serial;
			float LinX;
			float LinY;
			float LinZ;
			float AngX;
			float AngY;
			float AngZ;
			float AngW;
		};

		struct ExtConPosLocal
		{
			std::string Serial;
			float LinX;
			float LinY;
			float LinZ;
			float AngX;
			float AngY;
			float AngZ;
			float AngW;
		};

		struct ExtTraPosLocal
		{
			std::string Serial;
			float LinX;
			float LinY;
			float LinZ;
			float AngX;
			float AngY;
			float AngZ;
			float AngW;
		};

		struct ExtRcv
		{
			int Enable;
			int Port;
			std::string IP;
		};

		struct ExtLight
		{
			std::string Name;
			float LinX;
			float LinY;
			float LinZ;
			float AngX;
			float AngY;
			float AngZ;
			float AngW;
			float ColorR;
			float ColorG;
			float ColorB;
			float ColorA;
		};

		struct ExtVRM
		{
			std::string Path;
			std::string Title;
			std::string Hash;
		};

		struct ExtRemote
		{
			std::string Service;
			std::string JSON;
		};

		struct ExtOpt
		{
			std::string Option;
		};

		struct ExtSettingColor
		{
			float ColorR;
			float ColorG;
			float ColorB;
			float ColorA;
		};

		struct ExtSettingWin
		{
			int IsTopMost;
			int IsTransparent;
			int WindowClickThrough;
			int HideBorder;
		};

		struct ExtConfig
		{
			std::string Config;
		};

		struct Through
		{
			std::string URL;
			std::vector<std::vector<unsigned char> > Params;
		};
	}
}

#endif

#endif // VMC_PACKETS_H
