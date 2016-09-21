#include "Recorder.h"

MICROFONE_RECORDER::MICROFONE_RECORDER()
{
	av_register_all();
	avdevice_register_all();

	_record = false;
	//av_log_set_level(AV_LOG_DEBUG);
}

MICROFONE_RECORDER::~MICROFONE_RECORDER()
{
}


void MICROFONE_RECORDER::genetate_input_stream()
{
	int result = NULL;

	// Find all audio device and chose one
	find_device();

	_input_format_context = avformat_alloc_context();

	// Open device with given name
	if (avformat_open_input(&_input_format_context, _device_name.c_str(), _input_format, NULL) < NULL) 
	{
		printf("Could not open input file");
		exit(EXIT_FAILURE);
	}
	
	if ((result = avformat_find_stream_info(_input_format_context, NULL) < NULL))
	{
		printf("Failed to get input stream info\n");
		exit(EXIT_FAILURE);
	}

	_input_audio_stream = _input_format_context->streams[NULL];

	av_dump_format(_input_format_context, NULL, _device_name.c_str(), NULL);
}

void MICROFONE_RECORDER::generate_output_stream()
{
	int _return;

	avformat_alloc_output_context2(&_output_format_context, NULL, NULL, _file_name);
	if (!_output_format_context)
	{
		printf("Could not create output context \n");
		exit(EXIT_FAILURE);
	}
	_output_format = _output_format_context->oformat;

	_output_audio_stream = avformat_new_stream(_output_format_context, _input_audio_stream->codec->codec);
	if (!_output_audio_stream)
	{
		printf("Failed allocatinf output stream\n");
		exit(EXIT_FAILURE);
	}

	// Copy context form input to output
	_return = avcodec_copy_context(_output_audio_stream->codec, _input_audio_stream->codec);
	if (_return < 0)
	{
		printf("Failed to copy context input to output\n");
		exit(EXIT_FAILURE);
	}

	//From FFmpeg Example (muxing)
	_output_audio_stream->codec->codec_tag = 0;
	if (_output_format_context->oformat->flags & AVFMT_GLOBALHEADER)
		_output_audio_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

	if (!(_output_format->flags & AVFMT_NOFILE))
	{
		_return = avio_open(&_output_format_context->pb, _file_name, AVIO_FLAG_WRITE);
		if (_return < 0)
		{
			printf("Fail to open file\n");
			exit(EXIT_FAILURE);
		}
	}

	// Write header to file
	_return = avformat_write_header(_output_format_context, NULL);
	if (_return < NULL)
	{
		printf("Error while open file");
		exit(EXIT_FAILURE);
	}

	av_dump_format(_output_format_context, 0, _file_name, 1);
}

void MICROFONE_RECORDER::get_packet()
{
	AVPacket _packet;
	int32_t  _return;

	_return = av_read_frame(_input_format_context, &_packet);

	if(_return >= 0)
	{
		av_write_frame(_output_format_context, &_packet);
		av_free_packet(&_packet);
	}
}

void MICROFONE_RECORDER::RECORDER_HANDLER()
{
	if(_record) get_packet();
}

void MICROFONE_RECORDER::START_RECORD(const char * _name)
{
	_file_name = _name;

	genetate_input_stream();
	generate_output_stream();

	_record = true;

	get_packet();
}

void MICROFONE_RECORDER::STOP_RECORD()
{
	_record = false;
	get_packet();
	end_get_packet();
}

void MICROFONE_RECORDER::end_get_packet()
{
	// Write ending information to file
	av_write_trailer(_output_format_context);

	avformat_close_input(&_input_format_context);
	if (_output_format_context && !(_output_format_context->flags & AVFMT_NOFILE)) avio_close(_output_format_context->pb);

	avformat_free_context(_output_format_context);
	avformat_free_context(_input_format_context);
}

void MICROFONE_RECORDER::find_device()
{
}

//------------------------------------------------------------------------------------------------

WIN_MICROFONE_RECORDER::WIN_MICROFONE_RECORDER()
{
	_device_names.reserve(5);
}

WIN_MICROFONE_RECORDER::~WIN_MICROFONE_RECORDER()
{
}

void WIN_MICROFONE_RECORDER::find_device()
{
	
	SetConsoleOutputCP(65001); // Need for cyrilic console text
	generate_device_list();

	user_select_device();

	_input_format = av_find_input_format("dshow");
}

void WIN_MICROFONE_RECORDER::generate_device_list()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (SUCCEEDED(hr))
	{
		IEnumMoniker *pEnum;

		hr = enumerare_device(CLSID_AudioInputDeviceCategory, &pEnum);
		if (SUCCEEDED(hr))
		{
			display_device(pEnum);
			pEnum->Release();
		}
		CoUninitialize();
	}
}

HRESULT WIN_MICROFONE_RECORDER::enumerare_device(REFGUID category, IEnumMoniker **ppEnum)
{
	// Create the System Device Enumerator.
	ICreateDevEnum *pDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the category.
		hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
		}
		pDevEnum->Release();
	}
	return hr;

}

void WIN_MICROFONE_RECORDER::display_device(IEnumMoniker *pEnum)
{
	IMoniker *pMoniker = NULL;

	while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
	{
		IPropertyBag *pPropBag;
		HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
		if (FAILED(hr))
		{
			pMoniker->Release();
			continue;
		}

		VARIANT var;
		VariantInit(&var);

		// Get friendly name.
		hr = pPropBag->Read(L"FriendlyName", &var, 0);

		if (SUCCEEDED(hr))
		{
			printf("%S\n", var.bstrVal);
			std::wstring _tmp_str(var.bstrVal, SysStringLen(var.bstrVal));
			_device_names.push_back(_tmp_str);

			VariantClear(&var);
		}


		pPropBag->Release();
		pMoniker->Release();
	}
}


void WIN_MICROFONE_RECORDER::user_select_device()
{
	size_t _device_cnt   = _device_names.size();
	uint8_t _record_from = NULL;
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::string _tmp_disp_str;

 	if (_device_cnt > 1 && _device_cnt < 10)
	{
		printf("Select device to Record:\n");
		for (uint8_t _ind = NULL; _ind < _device_cnt; _ind++)
		{
			_tmp_disp_str = converter.to_bytes(_device_names[_ind]);
			printf("%i: %s \n", _ind, _tmp_disp_str.c_str());
		}

		char _key_code = NULL;

		do
		{
			printf("\nPrint 'N' to EXIT\n");
			printf("Print num of selected device: ");
			std::cin >> _key_code;
		} while (_key_code < 48 || _key_code > (_device_cnt + 47) && _key_code != 'N');

		if(_key_code == 'N') exit(EXIT_SUCCESS);
		
		_record_from = _key_code - 47;
	}

	_tmp_disp_str = converter.to_bytes(_device_names[_record_from]);
	printf("Record from device: %s \n", _tmp_disp_str.c_str());

	_device_name = "audio=";
	_device_name.append(converter.to_bytes(_device_names[_record_from]));
}
