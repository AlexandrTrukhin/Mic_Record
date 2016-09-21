#pragma once

extern "C"
{
	#include <libavdevice/avdevice.h>
}

#include "string"
#include "vector"
#include <codecvt>
#include <iostream>

using namespace std;

// Add switch for another OS
#include "windows.h"
#include "dshow.h"
#include "conio.h"


//----------------------------------------------------------------------------------------------------

class MICROFONE_RECORDER
{
	protected:
		const char      *_file_name;
		AVOutputFormat  *_output_format;
		AVInputFormat   *_input_format;
		AVFormatContext *_input_format_context;
		AVFormatContext *_output_format_context;
		AVStream        *_input_audio_stream;
		AVStream        *_output_audio_stream;

		uint8_t         *_audio_out_buff;
		int16_t         *_sampels;
		int32_t          _audio_input_frame_size;

		std::string     _device_name;

		bool             _record;

	public:
		MICROFONE_RECORDER();
		virtual ~MICROFONE_RECORDER();

		void RECORDER_HANDLER();
		void START_RECORD(const char * _name);
		void STOP_RECORD();

	private:
		void virtual find_device();
		void genetate_input_stream();
		void generate_output_stream();
		void get_packet();
		void end_get_packet();

};

//--------------------------------------------------------------------------------------------------
/*
*	Microfobne_Record for windows. Use this if WIN
*/

class WIN_MICROFONE_RECORDER: public MICROFONE_RECORDER
{
	protected:
		std::vector<std::wstring> _device_names;

	public:
		WIN_MICROFONE_RECORDER();
		~WIN_MICROFONE_RECORDER();

	protected:
		void find_device();

	private:
		void generate_device_list();
		HRESULT enumerare_device(REFGUID category, IEnumMoniker **ppEnum);
		void display_device(IEnumMoniker *pEnum);
		void user_select_device();
};

