#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "libssh\libssh.h"
#include "opencv.hpp"
#include "video_stream.h"
#include <msclr\marshal_cppstd.h>
#include <NvGamepad\NvGamepad.h>
#include <NvGamepad\NvGamepadXInput.h>
#include <string.h>
#include "ssh_func.h"
#include <fstream>
#include <dinput.h>
bool pad_debug = true;	//comment for no debug execution
//bool pad_debug = false; //comment for normal execution

LPDIRECTINPUT8 di;
HRESULT hr;
LPDIRECTINPUTDEVICE8 joystick;
DIDEVCAPS capabilities;
DIJOYSTATE2 js;
SRWLOCK camlock,Dxlock;

double old_rx, old_ry;
bool running_xinput = true, running_Dxinput = false, blue_lights = false, white_lights = false, moving = false, steering = false, terminateme=false;
std::string mycommand;

BOOL CALLBACK enumCallback(const DIDEVICEINSTANCE* instance, VOID* context) {
	HRESULT hr;

	// Obtain an interface to the enumerated joystick.
	hr = di->CreateDevice(instance->guidInstance, &joystick, NULL);

	// If it failed, then we can't use this joystick. (Maybe the user unplugged
	// it while we were in the middle of enumerating it.)
	if (FAILED(hr)) {
		if (pad_debug == true) {
			printf("HR FAILED\n");
		}
		return DIENUM_CONTINUE;
	}

	// Stop enumeration. Note: we're just taking the first joystick we get. You
	// could store all the enumerated joysticks and let the user pick.
	return DIENUM_STOP;
}

public ref class GamepadRead
{
//private:
	
public:
	System::String ^ ip_address_stream;

	ssh_session pub_ssh_session;
	


	void XinputRead() {
		//char msgbuf[15];
		static NvGamepad* sGamepad = NULL;
		NvGamepad::State state;
		static uint32_t sChangedMask = 0;


		NvGamepadXInput* gamepadXInput = new NvGamepadXInput;
		sGamepad = gamepadXInput;
		for (;;) {
			while (running_xinput == true) {
				//MSG msg;
				Sleep(100);	//slowdown gamepad read for multiple same-button calls fix
				//update gamepad stare
				sGamepad->getState(0, state);
				//----------------Checking Buttons and Triggers---------------------//
				
				//Ry stick triggered
				//move camera up/down
				if (state.mThumbRY < -0.1 || state.mThumbRY > 0.1) {
					old_ry = state.mThumbRY;
					do {
						if (state.mThumbRY < 0) { //move camera up
							mycommand = "echo cu > /home/johnny/myone.txt";
							shell_session(pub_ssh_session, mycommand,false);
						}
						else {	//move camera down
							mycommand = "echo cd > /home/johnny/myone.txt";
							shell_session(pub_ssh_session, mycommand,false);
						}
						sGamepad->getState(0, state);	//update gamepad state
					} while (old_ry == state.mThumbRY);	//do the same action until user decides to stop (speed up joystick read)
				}

				//Rx stick triggered
				//move camera right/left
				if (state.mThumbRX < -0.1 || state.mThumbRX > 0.1) {
					old_rx = state.mThumbRX;
					do {
						if (state.mThumbRX < 0) {	//move camera left
							mycommand = "echo cl > /home/johnny/myone.txt";
							shell_session(pub_ssh_session, mycommand,false);
						}

						else {	//move camera right
							mycommand = "echo cr > /home/johnny/myone.txt";
							shell_session(pub_ssh_session, mycommand,false);
						}
						sGamepad->getState(0, state);	//update gamepad state
					} while (old_rx == state.mThumbRX);	//do the same action until user decides to stop (speed up joystick read)

				}

				//Rs button is pressed
				//reset camera position
				if (state.mButtons == 128) {
					mycommand = "echo cc > /home/johnny/myone.txt";
					shell_session(pub_ssh_session, mycommand,false);
				}

				//Right/Left trigger used
				//move forward (mf) right triger used
				if (state.mRightTrigger != 0) {
					mycommand = "echo mf > /home/johnny/mytwo.txt";
					shell_session(pub_ssh_session, mycommand,false);
					if (moving == false)
						moving = true;
				}
				//move backwards (mb) left triger used
				else if (state.mLeftTrigger != 0) {
					mycommand = "echo mb > /home/johnny/mytwo.txt";
					shell_session(pub_ssh_session, mycommand,false);
					if (moving == false)
						moving = true;
				}
				//move stop (ms) Neither left triger nor right triger are pushed and the car is moving -> need to stop
				else if (moving == true) {
					moving = false;
					mycommand = "echo ms > /home/johnny/mytwo.txt";
					shell_session(pub_ssh_session, mycommand,false);
				}

				//Lx stick triggered
				//move right (mr)
				if (state.mThumbLX > 0.3) {
					mycommand = "echo mr > /home/johnny/mytwo.txt";
					shell_session(pub_ssh_session, mycommand,false);
					if (steering == false)
						steering = true;
				}
				//move left (ml)
				else if (state.mThumbLX < -0.3) {
					mycommand = "echo ml > /home/johnny/mytwo.txt";
					shell_session(pub_ssh_session, mycommand,false);
					if (steering == false)
						steering = true;
				}
				//steering stop (ss) stick is not triggered and the car is steering -> need to stop
				else if (steering == true) {
					steering = false;
					mycommand = "echo ss > /home/johnny/mytwo.txt";
					shell_session(pub_ssh_session, mycommand,false);
				}



				//Y button is pressed
				//enable/disable streaming
				if (state.mButtons == 32768) {
					//check if streamThread has created
					if (first_time_pushed_camera_button == true) {
						//streamThread already exists
						//update flags for pause/resume with SRW locks usage
						AcquireSRWLockExclusive(&camlock);
						if (stream_tread_runs == true) {
							if (pad_debug == true) {
								printf("Pausing from Thread %s...\n", System::AppDomain::GetCurrentThreadId().ToString());
							}
							stop_strem = true;
							stream_tread_runs = false;
						}
						else {
							if (pad_debug == true) {
								printf("Resuming from Thread %s...\n", System::AppDomain::GetCurrentThreadId().ToString());
							}
							stop_strem = false;
							stream_tread_runs = true;
						}
						ReleaseSRWLockExclusive(&camlock);
					}
					else {
						//streamThread has not created yet
						//we will create one here
						//create a passer
						video_stream^ stream_passer = gcnew video_stream;
						//passer gets streaming IP
						stream_passer->ip_address_stream = ip_address_stream;
						//create streamThread
						System::Threading::Thread^ streamThread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(stream_passer, &video_stream::stream));
						
						//update flags
						AcquireSRWLockExclusive(&camlock);

						first_time_pushed_camera_button = true;
						stream_tread_runs = true;
						stop_strem = false;

						ReleaseSRWLockExclusive(&camlock);

						//run streamThread
						streamThread->Start();
					}
				}

				//B button is pressed
				if (state.mButtons == 8192) {  //B button is pressed
					if (pad_debug == true) {
						printf("\nB button pressed \n");
					}
					if (blue_lights == false) {			//  on/off blue lights
						blue_lights = true;
						mycommand = "echo an > /home/johnny/myone.txt";	//blue lights on
						shell_session(pub_ssh_session, mycommand,false);
					}
					else {
						blue_lights = false;
						mycommand = "echo bf > /home/johnny/myone.txt";	//blue lights off
						shell_session(pub_ssh_session, mycommand,false);
					}
				}

				//A button is pressed
				if (state.mButtons == 4096) {  //A button is pressed
											   //OutputDebugString("\n A button pressed\n");
					if (white_lights == false) {			//  on/off white lights
						white_lights = true;
						mycommand = "echo wn > /home/johnny/myone.txt";	//white lights on
						shell_session(pub_ssh_session, mycommand,false);
					}
					else {
						white_lights = false;
						mycommand = "echo wf > /home/johnny/myone.txt";	//white lights on
						shell_session(pub_ssh_session, mycommand,false);
					}
				}

				//X button is pressed --maybe in future usage
				if (state.mButtons == 16384) {  //X button is pressed
					if (pad_debug == true) {
						printf("Xinput X pressed!\n");
					}
			//		mycommand = "mpg123 /home/john/Music/car-sound/Car_Horn_Honk1.mp3";
			//		shell_session(pub_ssh_session, mycommand,false);
				}
				
				//----------------Checking Buttons and Triggers ENDS---------------------//
			}
			//get to idle loop maybe dxinput thread is running
			Sleep(1000);
		
			if (terminateme == true) {	//terminate the tread the app is closing
				break;
			}
		}
	};


	void DxinputRead() {
		if (pad_debug == true) {
			printf("DX input started\n");
		}
		// Create a DirectInput device
		if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&di, NULL))) {
			if (pad_debug == true) {
				printf("\nFailed direct\n");
			}
		}

		if (FAILED(hr = di->EnumDevices(DI8DEVCLASS_GAMECTRL, enumCallback, NULL, DIEDFL_ATTACHEDONLY))) {
			if (pad_debug == true) {
				printf("\nFailed Enum \n");
			}
		}
		// Make sure we got a joystick
		if (joystick == NULL) {
			if (pad_debug == true) {
				printf("Joystick not found.\n");
			}
		}
		else {
			// Set the data format to "simple joystick" - a predefined data format 
			//
			// A data format specifies which controls on a device we are interested in,
			// and how they should be reported. This tells DInput that we will be
			// passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
			if (FAILED(hr = joystick->SetDataFormat(&c_dfDIJoystick2))) {
				if (pad_debug == true) {
					printf("Failed SetData \n");
				}
			}
			else {
				if (pad_debug == true) {
					printf("Set Data ok!\n");
				}
			}

			// Set the cooperative level to let DInput know how this device should
			// interact with the system and with other DInput applications.
			if (FAILED(hr = joystick->SetCooperativeLevel(NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND))) {
				if (pad_debug == true) {
					printf("Failed SetCooperative Level \n");
				}
			}

			// Determine how many axis the joystick has (so we don't error out setting
			// properties for unavailable axis)
			capabilities.dwSize = sizeof(DIDEVCAPS);
			if (FAILED(hr = joystick->GetCapabilities(&capabilities))) {
				if (pad_debug == true) {
					printf("Failed Get Capabilities \n");
				}
			}
		}

		for (;;) {
			while (running_Dxinput == true && joystick != NULL) {
				hr = joystick->Poll();

				if (FAILED(hr)) {
					// DInput is telling us that the input stream has been
					// interrupted. We aren't tracking any state between polls, so
					// we don't have any special reset that needs to be done. We
					// just re-acquire and try again.
					hr = joystick->Acquire();
					while (hr == DIERR_INPUTLOST) {
						hr = joystick->Acquire();
					}

					// If we encounter a fatal error, return failure.
					if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED)) {
						if (pad_debug == true) {
							printf("Failed encounter \n");
						}
						if (hr == DIERR_INVALIDPARAM) {
							if (pad_debug == true) {
								printf("invalid param\n");
							}
						}
						if (hr == DIERR_NOTINITIALIZED) {
							if (pad_debug == true) {
								printf("Not initialized\n");
							}
						}
					}

					// If another application has control of this device, return successfully.
					// We'll just have to wait our turn to use the joystick.
					if (hr == DIERR_OTHERAPPHASPRIO && pad_debug == true) {
						printf("Failed other \n");
					}
				}

				// Get the input's device state

				if (FAILED(hr = joystick->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {
					if (pad_debug == true) {
						printf("Failed  should have been acquired \n"); // The device should have been acquired during the Poll()
					}
				}

				//------------------------------------CHECK Direct INPUT---------------------------------
				joystick->GetDeviceState(sizeof(DIJOYSTATE2), &js);
				//check gamepad layout
				/*
				std::cout << "input is: " << js.rgbButtons[0] << "------" << js.rgbButtons[1] << "------" << js.rgbButtons[2] << "------" << js.rgbButtons[3] << "------" << js.rgbButtons[4] << "------" << js.rgbButtons[5] << "------" << js.rgbButtons[6] << "------" << js.rgbButtons[7] << "------" << js.rgbButtons[8] << "------" << js.rgbButtons[9] << "------" << js.rgbButtons[10] << "------" << js.rgbButtons[11] << "------" << "\n";
				std::cout << "input lARx: " << js.lARx << "\t";
				std::cout << "input lARy: " << js.lARy << "\t";
				std::cout << "input lARz: " << js.lARz << "\t";
				std::cout << "input lAX: " << js.lAX << "\t";
				std::cout << "input lAY: " << js.lAY << "\n";
				std::cout << "input lAZ: " << js.lAZ << "\t";
				std::cout << "input lFRx: " << js.lFRx << "\t";
				std::cout << "input lFRy: " << js.lFRy << "\t";
				std::cout << "input lFRz: " << js.lFRz << "\t";
				std::cout << "input lFX: " << js.lFX << "\n";
				std::cout << "input lFY: " << js.lFY << "\t";
				std::cout << "input lFZ: " << js.lFZ << "\t";
				std::cout << "input lRx: " << js.lRx << "\t";
				std::cout << "input lRy: " << js.lRy << "\t";
				std::cout << "input lRz: " << js.lRz << "\n";
				std::cout << "input lVRx: " << js.lVRx << "\t";
				std::cout << "input lVRy: " << js.lVRy << "\t";
				std::cout << "input lVRz: " << js.lVRz << "\t";
				std::cout << "input lVX: " << js.lVX << "\t";
				std::cout << "input lVY: " << js.lVY << "\n";
				std::cout << "input lVZ: " << js.lVZ << "\t";
				std::cout << "input lX: " << js.lX << "\t";
				std::cout << "input lY: " << js.lY << "\t";
				std::cout << "input lZ: " << js.lZ << "\n";
				std::cout << "-----------------------------------------------------\n";
				Sleep(2000);
				*/
				
								//Ry stick triggered
								//move camera up/down
								//if (js.lZ < 24511 || js.lZ > 46511) {	//uncoment for arch_lab gamepad
								if (js.lRz < 24511 || js.lRz > 46511) {
									old_ry = js.lRz;
									do {
										if (js.lRz < 32511) {	//move camera up
											mycommand = "echo cu > /home/johnny/myone.txt";
											shell_session(pub_ssh_session, mycommand,false);
										}
										else {	//move camera down
											mycommand = "echo cd > /home/johnny/myone.txt";
											shell_session(pub_ssh_session, mycommand,false);
										}
										joystick->GetDeviceState(sizeof(DIJOYSTATE2), &js);	//update gamepad state
									} while (old_ry == js.lRz);	//do the same action until user decides to stop (speed up joystick read)
								}

								//Rx stick triggered
								//move camera right/left
								//if (js.lRz < 24511 || js.lRz > 46511) { //uncoment for arch_lab gamepad
								if (js.lZ < 24511 || js.lZ > 46511) {
									old_rx = js.lZ;
									do {
										if (js.lZ < 32511) {	//move camera left
											mycommand = "echo cl > /home/johnny/myone.txt";
											shell_session(pub_ssh_session, mycommand,false);
										}

										else {	//move camera right
											mycommand = "echo cr > /home/johnny/myone.txt";
											shell_session(pub_ssh_session, mycommand,false);
										}
										joystick->GetDeviceState(sizeof(DIJOYSTATE2), &js);	//update gamepad state
									} while (old_rx == js.lZ);	//do the same action until user decides to stop (speed up joystick read)

								}

								//Rs button is pressed
								//reset camera position
								if (js.rgbButtons[11] != NULL) {	//no change required for arch_lab's gamepad support
									mycommand = "echo cc > /home/johnny/myone.txt";
									shell_session(pub_ssh_session, mycommand,false);
								}

								//Right/Left trigger used
								//move forward (mf) right triger used
								if (js.rgbButtons[7] != NULL) { //no change required for arch_lab's gamepad support
									mycommand = "echo mf > /home/johnny/mytwo.txt";
									shell_session(pub_ssh_session, mycommand,false);
									if (moving == false)
										moving = true;
								}
								//move backwards (mb) left triger used
								//else if (js.rgbButtons[5] != NULL) { //uncoment for arch_lab gamepad
								else if (js.rgbButtons[6] != NULL) {
									mycommand = "echo mb > /home/johnny/mytwo.txt";
									shell_session(pub_ssh_session, mycommand,false);
									if (moving == false)
										moving = true;
								}
								//move stop (ms)Neither left triger nor right triger are pushed and the car is moving -> need to stop
								else if (moving == true) {
									moving = false;
									mycommand = "echo ms > /home/johnny/mytwo.txt";
									shell_session(pub_ssh_session, mycommand,false);
								}

								//Lx stick triggered
								//move right (mr)
								if (js.lX > 42511) {	//no change required for arch_lab's gamepad support
									mycommand = "echo mr > /home/johnny/mytwo.txt";
									shell_session(pub_ssh_session, mycommand,false);
									if (steering == false)
										steering = true;
								}
								//move left (ml)
								else if (js.lX < 22511) {	//no change required for arch_lab's gamepad support
									mycommand = "echo ml > /home/johnny/mytwo.txt";
									shell_session(pub_ssh_session, mycommand,false);
									if (steering == false)
										steering = true;
								}
								//steering stop (ss) stick is not triggered and the car is steering -> need to stop
								else if (steering == true) {
									steering = false;
									mycommand = "echo ss > /home/johnny/mytwo.txt";
									shell_session(pub_ssh_session, mycommand,false);
								}

								//Y button is pressed
								//enable/disable streaming
								//if (js.rgbButtons[1] != NULL) {	//uncoment for arch_lab gamepad
								if (js.rgbButtons[3] != NULL) {
									//check if streamThread has created
									if (first_time_pushed_camera_button == true) {
										//streamThread already exists
										//update flags for pause/resume with SRW locks usage
										AcquireSRWLockExclusive(&camlock);
										if (stream_tread_runs == true) {
											if (pad_debug == true) {
												printf("Pausing from DX Thread %s...\n", System::AppDomain::GetCurrentThreadId().ToString());
											}
											stop_strem = true;
											stream_tread_runs = false;
										}
										else {
											if (pad_debug == true) {
												printf("Resuming from DX Thread %s...\n", System::AppDomain::GetCurrentThreadId().ToString());
											}
											stop_strem = false;
											stream_tread_runs = true;
										}
										ReleaseSRWLockExclusive(&camlock);
									}
									else {
										//streamThread has not created yet
										//we will create one here
										//create a passer
										video_stream^ stream_passer = gcnew video_stream;
										//passer gets streaming IP
										stream_passer->ip_address_stream = ip_address_stream;
										//create streamThread
										System::Threading::Thread^ streamThread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(stream_passer, &video_stream::stream));
										
										//update flags
										AcquireSRWLockExclusive(&camlock);

										first_time_pushed_camera_button = true;
										stream_tread_runs = true;
										stop_strem = false;

										ReleaseSRWLockExclusive(&camlock);
										//run streamThread
										streamThread->Start();
									}

								}

								//B button is pressed
								//if (js.rgbButtons[3] != NULL) {  //uncoment for arch_lab gamepad
								if (js.rgbButtons[2] != NULL) {
									if (blue_lights == false) {			//  on/off blue lights
										blue_lights = true;
										mycommand = "echo an > /home/johnny/myone.txt";	//blue lights on
										shell_session(pub_ssh_session, mycommand,false);
									}
									else {
										blue_lights = false;
										mycommand = "echo bf > /home/johnny/myone.txt";	//blue lights off
										shell_session(pub_ssh_session, mycommand,false);
									}
									if (pad_debug == true) {
										printf("DX B button pressed\n");
									}
								}

								//A button is pressed
								//if (js.rgbButtons[2] != NULL) {  //uncoment for arch_lab gamepad
								if (js.rgbButtons[1] != NULL) {
									if (white_lights == false) {			//  on/off white lights
										white_lights = true;
										mycommand = "echo wn > /home/johnny/myone.txt";	//white lights on
										shell_session(pub_ssh_session, mycommand,false);
									}
									else {
										white_lights = false;
										mycommand = "echo wf > /home/johnny/myone.txt";	//white lights off
										shell_session(pub_ssh_session, mycommand,false);
									}
									if (pad_debug == true) {
										printf("A button pressed \n");
									}
								}

								//X button is pressed --maybe in future usage
								if (js.rgbButtons[0] != NULL) {  //no change required for arch_lab's gamepad support
									//mycommand = "mpg123 /home/johnny/car-sound/Car_Horn_Honk1.mp3";
									//shell_session(pub_ssh_session, mycommand,false);
									if (pad_debug == true) {
										printf("DX x pressed! \n");
									}
								}

								//------------------------------------END Direct INPUT CHECK----------------------------
								Sleep(100);
							
						
			}
			//get to idle loop maybe Xiput thread is running
			Sleep(1000);
			if (terminateme == true) {	//terminate the tread the app is closing
				break;
			}
		}
	}

};