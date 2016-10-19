#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "libssh\libssh.h"
#include <iostream>
#include "opencv.hpp"
#include <msclr\marshal_cppstd.h>
#include <Windows.h>
#include <NvGamepad\NvGamepad.h>
#include <NvGamepad\NvGamepadXInput.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <io.h> 
#include <conio.h>



std::string ip_address_stream;
ssh_session pub_ssh_session;


int shell_session(ssh_session session, std::string my_command) {
	ssh_channel kanali1;

	int rc2;
	char buffer[256];
	int nbytes;

	kanali1 = ssh_channel_new(session);
	if (kanali1 == NULL) {
		printf("\n kanali1 error!!!!\n");
		return SSH_ERROR;
	}
	rc2 = ssh_channel_open_session(kanali1);
	//rc2 = ssh_channel_request_pty(kanali1);
	if (rc2 != SSH_OK) {
		printf("\n open_session error!!!!\n");
	//	ssh_channel_free(kanali1);
		return rc2;
	}

	
	
	rc2 = ssh_channel_request_exec(kanali1, my_command.c_str());
	if (rc2 != SSH_OK) {
		printf("\n channel request error!!!!\n");
	//	ssh_channel_free(kanali1);
	//	ssh_channel_close(kanali1);
		return rc2;
	}

	nbytes = ssh_channel_read(kanali1, buffer, sizeof(buffer), 0);
	while (nbytes > 0)
	{
		if (fwrite(buffer, 1, nbytes, stdout) != nbytes)
		{
	//		ssh_channel_close(kanali1);
	//		ssh_channel_free(kanali1);
			printf("\n fwrite error!!!!\n");
			return SSH_ERROR;
		}
		nbytes = ssh_channel_read(kanali1, buffer, sizeof(buffer), 0);
	}
	if (nbytes < 0)
	{
	//	ssh_channel_close(kanali1);
	//	ssh_channel_free(kanali1);
		printf("\n nbytes error!!!!\n");
		return SSH_ERROR;
	}

	ssh_channel_send_eof(kanali1);
	ssh_channel_close(kanali1);
	ssh_channel_free(kanali1);

	return SSH_OK;
}



int rc_pub=1;

public ref class GamepadThread
{
public:

	

	static void GamepadRead(void) {
		std::string mycommand;
		char msgbuf[15];
		static NvGamepad* sGamepad = NULL;
		NvGamepad::State state;
		static uint32_t sChangedMask = 0;
		bool running = true;
		NvGamepadXInput* gamepadXInput = new NvGamepadXInput;
		sGamepad = gamepadXInput;
		double old_rx, old_ry;
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

					sGamepad->getState(0, state);
					//sprintf_s(msgbuf, "%d\n", state.mButtons);
					//OutputDebugString(msgbuf);

					//----------------Checking Buttons and Triggers---------------------//
					
					//Ry stick triggered
					if (state.mThumbRY < -0.1 || state.mThumbRY > 0.1) {

						old_ry = state.mThumbRY;
						do {

							if (state.mThumbRY<0) {
								mycommand = "echo cu > /home/johnny/myone.txt";
								shell_session(pub_ssh_session, mycommand);
							}
							else{
								mycommand = "echo cd > /home/johnny/myone.txt";
								shell_session(pub_ssh_session, mycommand);
							}
							sGamepad->getState(0, state);
						} while (old_ry == state.mThumbRY);
					}

					//Rx stick triggered
					if (state.mThumbRX < -0.1 || state.mThumbRX > 0.1) {

						old_rx = state.mThumbRX;
						do {
							if (state.mThumbRX<0) {
								mycommand = "echo cl > /home/johnny/myone.txt";
								shell_session(pub_ssh_session, mycommand);
							}
							
							else{
								mycommand = "echo cr > /home/johnny/myone.txt";
								shell_session(pub_ssh_session, mycommand);
							}
							sGamepad->getState(0, state);
						}while (old_rx == state.mThumbRX);

					}

					//Y button is pressed
					if (state.mButtons == 32768) {
						OutputDebugString("pathses to Y\n");
						cv::VideoCapture vcap;
						cv::Mat image;
						const std::string videoStreamAddress = ip_address_stream;
						//open the video stream and make sure it's opened
						if (!vcap.open(videoStreamAddress)) {
							//label1->Text = "Error opening video stream or file";
						}
						else {
							for (;;) {
								if (!vcap.read(image)) {
									//label1->Text = "No frame";
									cv::waitKey();
								}
								cv::imshow("Output Window", image);
								if (cv::waitKey(1) >= 0) break;

								sGamepad->getState(0, state);

								//----------------Checking Buttons---------------------//

								//Y button is pressed
								if (state.mButtons == 32768) {
									break;
								}					
								
								//Ry stick triggered
								if (state.mThumbRY < -0.1 || state.mThumbRY > 0.1) {

									old_ry = state.mThumbRY;
									do {

										if (state.mThumbRY<0) {
											mycommand = "echo cu > /home/johnny/myone.txt";
											shell_session(pub_ssh_session, mycommand);
										}
										else {
											mycommand = "echo cd > /home/johnny/myone.txt";
											shell_session(pub_ssh_session, mycommand);
										}
										sGamepad->getState(0, state);
									} while (old_ry == state.mThumbRY);
								}

								//Rx stick triggered
								if (state.mThumbRX < -0.1 || state.mThumbRX > 0.1) {

									old_rx = state.mThumbRX;
									do {
										if (state.mThumbRX<0) {
											mycommand = "echo cl > /home/johnny/myone.txt";
											shell_session(pub_ssh_session, mycommand);
										}

										else {
											mycommand = "echo cr > /home/johnny/myone.txt";
											shell_session(pub_ssh_session, mycommand);
										}
										sGamepad->getState(0, state);
									} while (old_rx == state.mThumbRX);

								}


								//Rs button is pressed
								if (state.mButtons == 128) {
									mycommand = "echo cc > /home/johnny/myone.txt";
									shell_session(pub_ssh_session, mycommand);
								}

								//B button is pressed
								if (state.mButtons == 8192) {  //B button is pressed
									OutputDebugString("\n B button pressed\n");
								}

								//A button is pressed
								if (state.mButtons == 4096) {  //A button is pressed
									OutputDebugString("\n A button pressed\n");
								}

								//X button is pressed
								if (state.mButtons == 16384) {  //X button is pressed
									OutputDebugString("\n X button pressed\n");
								}

								//----------------Checking Buttons ENDS---------------------//




							}
						}
					}
					
					//Rs button is pressed
					if (state.mButtons == 128) {
						mycommand = "echo cc > /home/johnny/myone.txt";
						shell_session(pub_ssh_session, mycommand);
					}
					
					//B button is pressed
					if (state.mButtons == 8192) {  //B button is pressed
						OutputDebugString("\n B button pressed\n");
					}
					
					//A button is pressed
					if (state.mButtons == 4096) {  //A button is pressed
						OutputDebugString("\n A button pressed\n");
					}
					
					//X button is pressed
					if (state.mButtons == 16384) {  //X button is pressed
						OutputDebugString("\n X button pressed\n");
					}
				
					//----------------Checking Buttons and Triggers ENDS---------------------//
					
					//mycommand = "tail -n 1  /home/johnny/presonar.txt";
					//shell_session(pub_ssh_session, mycommand);
					sChangedMask = 0;
				}
			}
		}
	};


	static void sonaread(void) {
		std::string mycommand;
		std::string com_ret;
		int sonar;
		for (;;) {
			if (rc_pub != 1) {
				mycommand = "tail -n 1  /home/johnny/presonar.txt";
				com_ret =shell_session(pub_ssh_session, mycommand);
				sonar = atoi(com_ret.c_str());
				printf("Sonar: %d\n", sonar);
				
			}
			//printf("hello\n");
			Sleep(2000);
		}
	}


};


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
	public ref class MyForm : public System::Windows::Forms::Form
	{
	public:
		
		MyForm(void)
		{
			InitializeComponent();
			//System::Threading::Thread padthread;
		    System::Threading::Thread^ padthread = gcnew System::Threading::Thread ( gcnew System::Threading::ThreadStart( &GamepadThread::GamepadRead));
			System::Threading::Thread^ sothread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(&GamepadThread::sonaread));
			padthread->Start();
			sothread->Start();
			//padthread->Abort();
			
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MyForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::TextBox^  textBox1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Button^  button2;

	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->SuspendLayout();
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(189, 52);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(58, 23);
			this->button1->TabIndex = 0;
			this->button1->Text = L"Camera";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(26, 91);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(0, 13);
			this->label1->TabIndex = 1;
			this->label1->Click += gcnew System::EventHandler(this, &MyForm::label1_Click);
			// 
			// textBox1
			// 
			this->textBox1->Location = System::Drawing::Point(52, 23);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(131, 20);
			this->textBox1->TabIndex = 2;
			this->textBox1->Text = L"192.168.1.5";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(26, 28);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(20, 13);
			this->label2->TabIndex = 3;
			this->label2->Text = L"IP:";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(26, 114);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(54, 13);
			this->label3->TabIndex = 4;
			this->label3->Text = L"No IP yet.";
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point(189, 23);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(61, 23);
			this->button2->TabIndex = 5;
			this->button2->Text = L"Connect";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &MyForm::button2_Click);
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(294, 136);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->button1);
			this->Name = L"MyForm";
			this->Text = L"MyForm";
			this->ResumeLayout(false);
			this->PerformLayout();

		}

#pragma endregion
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
		
		cv::VideoCapture vcap;
		cv::Mat image;

		const std::string videoStreamAddress = ip_address_stream;
		//open the video stream and make sure it's opened
		if (!vcap.open(videoStreamAddress)) {
			label1->Text = "Error opening video stream or file";
		}
		else {
			for (;;) {
				if (!vcap.read(image)) {
					//label1->Text = "No frame";
					cv::waitKey();
				}
				cv::imshow("Output Window", image);

				if (cv::waitKey(1) >= 0) break;
			}
		}
	}
	private: System::Void label1_Click(System::Object^  sender, System::EventArgs^  e) {
	}
protected: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
	String ^ ip_address2 = textBox1->Text;
	std::string  httpadder = "http://";
	std::string  streamport = ":8081";
	std::string ip_address = msclr::interop::marshal_as<std::string>(ip_address2);
	ip_address_stream = ip_address;
	ip_address_stream.insert(0, httpadder);
	ip_address_stream.insert(ip_address_stream.size(), streamport);
	label3->Text = ip_address2;
	printf("\nStream IP: %s\n\n", ip_address_stream.c_str());
	
	ssh_session my_ssh_session;
	int rc;
	int port = 22;
	int verbosity = SSH_LOG_PROTOCOL;
	char *password;
	// Open session and set options
	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL)
		exit(-1);

	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, ip_address.c_str());
	ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "john");
	//ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "johnny");
	ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

	// Connect to server
	rc = ssh_connect(my_ssh_session);
	
	if (rc != SSH_OK)
	{
		fprintf(stderr, "Error Connecting: %s\n",
			ssh_get_error(my_ssh_session));
		ssh_free(my_ssh_session);
		//exit(-1);
	}
	else {
		// Authenticate ourselves
		password = "mariamatz";
		//password = "772985";
		rc = ssh_userauth_password(my_ssh_session, NULL, password);
		rc_pub = rc;
		if (rc != SSH_AUTH_SUCCESS)
		{
			fprintf(stderr, "Error authenticating with password: %s\n",
				ssh_get_error(my_ssh_session));
			ssh_disconnect(my_ssh_session);
			ssh_free(my_ssh_session);
			exit(-1);
		}

		
		std::string mycommand;

		pub_ssh_session = my_ssh_session;

	}
}
};
}

