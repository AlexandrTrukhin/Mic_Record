#include "Recorder.h"

int main()
{
	std::string _file_name;
	WIN_MICROFONE_RECORDER *_recoreder_ptr = new WIN_MICROFONE_RECORDER;

	printf("Define file name: ");
	getline(cin, _file_name);
	_file_name.append(".wav");

	printf("Press any key to start record\n");
	while (!_kbhit());
	_getch();
	_recoreder_ptr->START_RECORD(_file_name.c_str());


	printf("Press any key to stop record\n");
	while (!_kbhit()) _recoreder_ptr->RECORDER_HANDLER();

	_recoreder_ptr->STOP_RECORD();

	delete _recoreder_ptr;

    return 0;
}

