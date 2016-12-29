#include "main_gui.h"
//#include "gamepad_function.h"
//#include "ssh_func.h"
//////////////////////////
#include <string>
#include <sstream>
#include <iomanip>
#include <Windows.h>
#include <NvGamepad\NvGamepad.h>
#include <NvGamepad\NvGamepadXInput.h>
///////////////////////////////////
using namespace System;
using namespace System::Windows::Forms;
///////////////////////////////////////////////

[STAThread]
void Main(array<String^>^ args)
{
	OutputDebugString("\nHello world \n\n");
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	guiWopencv::MyForm form;
	Application::Run(%form);


}
