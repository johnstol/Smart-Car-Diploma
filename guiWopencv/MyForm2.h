#pragma once
#include <stdio.h>
#include <iostream>
#include "opencv.hpp"
#include <msclr\marshal_cppstd.h>
#include <Windows.h>
#include <NvGamepad\NvGamepad.h>
#include <NvGamepad\NvGamepadXInput.h>
#include <string>
#include <sstream>
#include <iomanip>


namespace guiWopencv {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace std;
	/// <summary>
	/// Summary for MyForm
	/// </summary>
	public ref class MyForm2 : public System::Windows::Forms::Form
	{
	public:
		MyForm2(void)
		{
			OutputDebugString("\nHelloFrom form 2 \n\n");
			katialo();
		}



#pragma kati
		void katialo(void) {
			int miametavliti;
			char msgbuf[5];


			static NvGamepad* sGamepad = NULL;
			NvGamepad::State state;
			static uint32_t sChangedMask = 0;


			bool running = true;

			NvGamepadXInput* gamepadXInput = new NvGamepadXInput;
			sGamepad = gamepadXInput;



			while (running) {
				MSG msg;

				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {	// a message is available -after peek, remove message from queue
					if (msg.message == WM_QUIT) {
						running = false;
						break;
					}

					TranslateMessage(&msg); //virtual-key messages into character messages
					DispatchMessage(&msg); //message to a window procedure

				}
				else {											// no messages are available
					sChangedMask |= gamepadXInput->pollGamepads(); //update the state of all gamepads

					if (sChangedMask) {

						///////////////////////////////////////////////////
						//sStatusString = ProcessGamepad(sGamepad);

						sGamepad->getState(0, state);
						sprintf(msgbuf, "%d\n", state.mButtons);
						OutputDebugString(msgbuf);
						if (state.mButtons == 32768) {
							OutputDebugString("pathses to Y\n");
						}
						//////////////////////////////////////////////////
						// Log the latest values
						//OutputDebugString(state.mButtons);	//Sends a string to the debugger for display

						// Force a paint to the window, so we print the latest values
						//RedrawWindow(hwnd, NULL, 0, RDW_INVALIDATE);

						sChangedMask = 0;
					}
				}
			}

			// Exit program
			//exit(EXIT_SUCCESS);

			/////////////////////////////////////////////////////////////////
		}
	};

}