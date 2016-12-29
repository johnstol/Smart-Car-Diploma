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
#include "gamepad_Xinput.h"
#include "ssh_func.h"
#include "video_stream.h"
#include "Sonar.h"

std::string ip_address_stream;
ssh_session pub_ssh_session,sonar_pub_ssh_session;

bool connection_established = false, dxthread_started,xinputhread_started,DX_enable=false;
bool main_debug = true; //comment for no debug execution
//bool main_debug = false; //comment for normal execution


int rc_pub=1;




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
			InitializeComponent();	//initialize gui components
			InitializeBackgoundWorker();	//initialize background thread for live gui update (sonar value)
											//we use BackgoundWorker thread because is the right way to live update a label at the gui 

		}

	private:
		// Set up the BackgroundWorker object by 
		// attaching event handlers. 
		void InitializeBackgoundWorker()	//we need at least 2 events 
		{
			backgroundWorker1->DoWork += gcnew DoWorkEventHandler(this, &MyForm::backgroundWorker1_DoWork);		//create event dowork (buildin event) 
			backgroundWorker1->ProgressChanged += gcnew ProgressChangedEventHandler(this, &MyForm::backgroundWorker1_ProgressChanged);	//create event ProgressChanged (buildin event)
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
		}	//declare gui components 
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::TextBox^  textBox1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::Button^  DX;
	private: System::Windows::Forms::Label^  DX_label;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::Label^  SonarFL;
	private: System::Windows::Forms::Label^  SonarFL;

	private: System::ComponentModel::BackgroundWorker^  backgroundWorker1;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::Label^  label9;
	private: System::Windows::Forms::Label^  label11;
	private: System::Windows::Forms::Label^  label13;
	private: System::Windows::Forms::Label^  SonarFR;
	private: System::Windows::Forms::Label^  SonarBL;
	private: System::Windows::Forms::Label^  SonarBR;
	private: System::Windows::Forms::Label^  SonarCM;
	private: System::Windows::Forms::Label^  label6;
	private: System::Windows::Forms::Label^  label7;
	private: System::Windows::Forms::Label^  label8;
	private: System::Windows::Forms::Label^  label9;
	private: System::Windows::Forms::Label^  label10;
	private: System::Windows::Forms::Label^  label11;
	private: System::Windows::Forms::Label^  label12;
	private: System::Windows::Forms::Label^  label13;


	protected:

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;
		this->button1 = (gcnew System::Windows::Forms::Button());
		this->textBox1 = (gcnew System::Windows::Forms::TextBox());
		this->label2 = (gcnew System::Windows::Forms::Label());
		this->label3 = (gcnew System::Windows::Forms::Label());
		this->button2 = (gcnew System::Windows::Forms::Button());
		this->button3 = (gcnew System::Windows::Forms::Button());
		this->DX = (gcnew System::Windows::Forms::Button());
		this->DX_label = (gcnew System::Windows::Forms::Label());
		this->label1 = (gcnew System::Windows::Forms::Label());
		this->label4 = (gcnew System::Windows::Forms::Label());
		this->SonarFL = (gcnew System::Windows::Forms::Label());
		this->backgroundWorker1 = (gcnew System::ComponentModel::BackgroundWorker());
		this->SonarFR = (gcnew System::Windows::Forms::Label());
		this->label7 = (gcnew System::Windows::Forms::Label());
		this->SonarBL = (gcnew System::Windows::Forms::Label());
		this->label9 = (gcnew System::Windows::Forms::Label());
		this->SonarBR = (gcnew System::Windows::Forms::Label());
		this->label11 = (gcnew System::Windows::Forms::Label());
		this->SonarCM = (gcnew System::Windows::Forms::Label());
		this->label13 = (gcnew System::Windows::Forms::Label());
		this->SuspendLayout();
		// 
		// button1
		// 
		this->button1->Location = System::Drawing::Point(189, 52);
		this->button1->Name = L"button1";
		this->button1->Size = System::Drawing::Size(61, 23);
		this->button1->TabIndex = 0;
		this->button1->Text = L"Camera";
		this->button1->UseVisualStyleBackColor = true;
		this->button1->Click += gcnew System::EventHandler(this, &MyForm::button1_Click);
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
		this->label3->Location = System::Drawing::Point(12, 138);
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
		// button3
		// 
		this->button3->Location = System::Drawing::Point(189, 110);
		this->button3->Name = L"button3";
		this->button3->Size = System::Drawing::Size(61, 23);
		this->button3->TabIndex = 6;
		this->button3->Text = L"Close";
		this->button3->UseVisualStyleBackColor = true;
		this->button3->Click += gcnew System::EventHandler(this, &MyForm::button3_Click);
		// 
		// DX
		// 
		this->DX->Location = System::Drawing::Point(189, 81);
		this->DX->Name = L"DX";
		this->DX->Size = System::Drawing::Size(61, 23);
		this->DX->TabIndex = 7;
		this->DX->Text = L"DX";
		this->DX->UseVisualStyleBackColor = true;
		this->DX->Click += gcnew System::EventHandler(this, &MyForm::DX_Click);
		// 
		// DX_label
		// 
		this->DX_label->AutoSize = true;
		this->DX_label->ForeColor = System::Drawing::SystemColors::GrayText;
		this->DX_label->Location = System::Drawing::Point(105, 86);
		this->DX_label->Name = L"DX_label";
		this->DX_label->Size = System::Drawing::Size(64, 13);
		this->DX_label->TabIndex = 8;
		this->DX_label->Text = L"DX disabled";
		this->DX_label->Click += gcnew System::EventHandler(this, &MyForm::label4_Click);
		// 
		// label1
		// 
		this->label1->AutoSize = true;
		this->label1->Location = System::Drawing::Point(256, 28);
		this->label1->Name = L"label1";
		this->label1->Size = System::Drawing::Size(47, 13);
		this->label1->TabIndex = 9;
		this->label1->Text = L"SonarFL";
		// 
		// label4
		// 
		this->label4->AutoSize = true;
		this->label4->ForeColor = System::Drawing::Color::Red;
		this->label4->Location = System::Drawing::Point(105, 62);
		this->label4->Name = L"label4";
		this->label4->Size = System::Drawing::Size(78, 13);
		this->label4->TabIndex = 10;
		this->label4->Text = L"No Connection";
		// 
		// SonarFL
		// 
		this->SonarFL->AutoSize = true;
		this->SonarFL->Location = System::Drawing::Point(309, 28);
		this->SonarFL->Name = L"SonarFL";
		this->SonarFL->Size = System::Drawing::Size(27, 13);
		this->SonarFL->TabIndex = 11;
		this->SonarFL->Text = L"N/A";
		// 
		// backgroundWorker1
		// 
		this->backgroundWorker1->WorkerReportsProgress = true;
		this->backgroundWorker1->DoWork += gcnew System::ComponentModel::DoWorkEventHandler(this, &MyForm::backgroundWorker1_DoWork);
		// 
		// SonarFR
		// 
		this->SonarFR->AutoSize = true;
		this->SonarFR->Location = System::Drawing::Point(434, 28);
		this->SonarFR->Name = L"SonarFR";
		this->SonarFR->Size = System::Drawing::Size(27, 13);
		this->SonarFR->TabIndex = 13;
		this->SonarFR->Text = L"N/A";
		// 
		// label7
		// 
		this->label7->AutoSize = true;
		this->label7->Location = System::Drawing::Point(379, 28);
		this->label7->Name = L"label7";
		this->label7->Size = System::Drawing::Size(49, 13);
		this->label7->TabIndex = 12;
		this->label7->Text = L"SonarFR";
		// 
		// SonarBL
		// 
		this->SonarBL->AutoSize = true;
		this->SonarBL->Location = System::Drawing::Point(310, 120);
		this->SonarBL->Name = L"SonarBL";
		this->SonarBL->Size = System::Drawing::Size(27, 13);
		this->SonarBL->TabIndex = 15;
		this->SonarBL->Text = L"N/A";
		// 
		// label9
		// 
		this->label9->AutoSize = true;
		this->label9->Location = System::Drawing::Point(256, 120);
		this->label9->Name = L"label9";
		this->label9->Size = System::Drawing::Size(48, 13);
		this->label9->TabIndex = 14;
		this->label9->Text = L"SonarBL";
		// 
		// SonarBR
		// 
		this->SonarBR->AutoSize = true;
		this->SonarBR->Location = System::Drawing::Point(434, 120);
		this->SonarBR->Name = L"SonarBR";
		this->SonarBR->Size = System::Drawing::Size(27, 13);
		this->SonarBR->TabIndex = 17;
		this->SonarBR->Text = L"N/A";
		// 
		// label11
		// 
		this->label11->AutoSize = true;
		this->label11->Location = System::Drawing::Point(378, 120);
		this->label11->Name = L"label11";
		this->label11->Size = System::Drawing::Size(50, 13);
		this->label11->TabIndex = 16;
		this->label11->Text = L"SonarBR";
		// 
		// SonarCM
		// 
		this->SonarCM->AutoSize = true;
		this->SonarCM->Location = System::Drawing::Point(372, 81);
		this->SonarCM->Name = L"SonarCM";
		this->SonarCM->Size = System::Drawing::Size(27, 13);
		this->SonarCM->TabIndex = 19;
		this->SonarCM->Text = L"N/A";
		// 
		// label13
		// 
		this->label13->AutoSize = true;
		this->label13->Location = System::Drawing::Point(315, 81);
		this->label13->Name = L"label13";
		this->label13->Size = System::Drawing::Size(51, 13);
		this->label13->TabIndex = 18;
		this->label13->Text = L"SonarCM";
		// 
		// MyForm
		// 
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->ClientSize = System::Drawing::Size(701, 191);
		this->ControlBox = false;
		this->Controls->Add(this->SonarCM);
		this->Controls->Add(this->label13);
		this->Controls->Add(this->SonarBR);
		this->Controls->Add(this->label11);
		this->Controls->Add(this->SonarBL);
		this->Controls->Add(this->label9);
		this->Controls->Add(this->SonarFR);
		this->Controls->Add(this->label7);
		this->Controls->Add(this->SonarFL);
		this->Controls->Add(this->label4);
		this->Controls->Add(this->label1);
		this->Controls->Add(this->DX_label);
		this->Controls->Add(this->DX);
		this->Controls->Add(this->button3);
		this->Controls->Add(this->button2);
		this->Controls->Add(this->label3);
		this->Controls->Add(this->label2);
		this->Controls->Add(this->textBox1);
		this->Controls->Add(this->button1);
		this->Name = L"MyForm";
		this->Text = L"Car Control";
		this->ResumeLayout(false);
		this->PerformLayout();

		this->label6->Size = System::Drawing::Size(27, 13);
			this->label6->TabIndex = 13;
			this->label6->Text = L"N/A";
			// 
			// label7
			// 
			this->label7->AutoSize = true;
			this->label7->Location = System::Drawing::Point(379, 28);
			this->label7->Name = L"label7";
			this->label7->Size = System::Drawing::Size(49, 13);
			this->label7->TabIndex = 12;
			this->label7->Text = L"SonarFR";
			// 
			// label8
			// 
			this->label8->AutoSize = true;
			this->label8->Location = System::Drawing::Point(310, 120);
			this->label8->Name = L"label8";
			this->label8->Size = System::Drawing::Size(27, 13);
			this->label8->TabIndex = 15;
			this->label8->Text = L"N/A";
			// 
			// label9
			// 
			this->label9->AutoSize = true;
			this->label9->Location = System::Drawing::Point(256, 120);
			this->label9->Name = L"label9";
			this->label9->Size = System::Drawing::Size(48, 13);
			this->label9->TabIndex = 14;
			this->label9->Text = L"SonarBL";
			// 
			// label10
			// 
			this->label10->AutoSize = true;
			this->label10->Location = System::Drawing::Point(434, 120);
			this->label10->Name = L"label10";
			this->label10->Size = System::Drawing::Size(27, 13);
			this->label10->TabIndex = 17;
			this->label10->Text = L"N/A";
			// 
			// label11
			// 
			this->label11->AutoSize = true;
			this->label11->Location = System::Drawing::Point(378, 120);
			this->label11->Name = L"label11";
			this->label11->Size = System::Drawing::Size(50, 13);
			this->label11->TabIndex = 16;
			this->label11->Text = L"SonarBR";
			// 
			// label12
			// 
			this->label12->AutoSize = true;
			this->label12->Location = System::Drawing::Point(372, 81);
			this->label12->Name = L"label12";
			this->label12->Size = System::Drawing::Size(27, 13);
			this->label12->TabIndex = 19;
			this->label12->Text = L"N/A";
			// 
			// label13
			// 
			this->label13->AutoSize = true;
			this->label13->Location = System::Drawing::Point(315, 81);
			this->label13->Name = L"label13";
			this->label13->Size = System::Drawing::Size(51, 13);
			this->label13->TabIndex = 18;
			this->label13->Text = L"SonarCM";
			// 
			// MyForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(701, 191);
			this->ControlBox = false;
			this->Controls->Add(this->label12);
			this->Controls->Add(this->label13);
			this->Controls->Add(this->label10);
			this->Controls->Add(this->label11);
			this->Controls->Add(this->label8);
			this->Controls->Add(this->label9);
			this->Controls->Add(this->label6);
			this->Controls->Add(this->label7);
			this->Controls->Add(this->SonarFL);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->DX_label);
			this->Controls->Add(this->DX);
			this->Controls->Add(this->button3);
			this->Controls->Add(this->button2);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->textBox1);
			this->Controls->Add(this->button1);
			this->Name = L"MyForm";
			this->Text = L"Car Control";
			this->ResumeLayout(false);
			this->PerformLayout();

		}

#pragma endregion
//camera button
	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
		//check if an SSH connection is establised 
		//we want this button actually do something after a successfull SSH connection
		if (connection_established == true) {
			//check if the streamThread  already runs
			if (first_time_pushed_camera_button == true) {
				//the streamThread already exists so we need only to update the flags
				//use SRW locks to face competitive problems between the threads
				AcquireSRWLockExclusive(&camlock);
				if (stream_tread_runs == true) {
					if (main_debug == true) {
						printf("Pausing...\n");
					}
					stop_strem = true;
					stream_tread_runs = false;
				}
				else {
					if (main_debug == true) {
						printf("Resuming...\n");
					}
					stop_strem = false;
					stream_tread_runs = true;
				}
				ReleaseSRWLockExclusive(&camlock);
			}
			else {
				//convert the ip_address_stream (http:\\xxx:xxx:xxx:xxx:8081) from std string to System string
				String^ ip_address = msclr::interop::marshal_as<String ^>(ip_address_stream);
				//create an object which includes the necessary values we want to pass to streamThread
				video_stream^ stream_passer = gcnew video_stream;
				//pass the desirable value 
				stream_passer->ip_address_stream = ip_address;
				//create the streamThread which will have the values of the stream_passer and it will run the "stream" function 
				System::Threading::Thread^ streamThread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(stream_passer, &video_stream::stream));
				//we start the thread
				streamThread->Start();
				//use SRW locks for safety reasons 
				//update necessary flags
				AcquireSRWLockExclusive(&camlock);

				first_time_pushed_camera_button = true;
				stream_tread_runs = true;
				stop_strem = false;

				ReleaseSRWLockExclusive(&camlock);
			}
		}
		else {
			if (main_debug == true) {
				printf("SSH connection required\n");
			}
		}
	}

//connection status label
	private: System::Void label1_Click(System::Object^  sender, System::EventArgs^  e) {
	}

//DX label
	private: System::Void label4_Click(System::Object^  sender, System::EventArgs^  e) {
	}
//connect button
	private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
		//get the IP user has given
		String ^ ip_address2 = textBox1->Text;
		ssh_session my_ssh_session,sonar_ssh_session;
		std::string  httpadder = "http://", streamport = ":8080", ip_address;
		System::Threading::Thread^ padthread;
		//check if connection already exists
		if(connection_established==false){
			//convert the taken ip from system string to std string
			ip_address = msclr::interop::marshal_as<std::string>(ip_address2);
			//check if the given IP is actualy IP 
			if (check_input(ip_address) == false) {
				label3->Text = "This is not a valid IP ->" +ip_address2;
				label3->ForeColor = System::Drawing::Color::Red;
			}
			else {
				//create ssh session for gamepad
				my_ssh_session = create_ssh_connection(ip_address);
				//create ssh session for sonar read
				sonar_ssh_session = create_ssh_connection(ip_address);
				//convert ip from xxx.xxx.xxx.xxx to http://xxx.xxx.xxx.xxx:8081
				ip_address.insert(0, httpadder);
				ip_address.insert(ip_address.size(), streamport);

				ip_address_stream = ip_address;
				
				//convert std string to system string
				ip_address2 = msclr::interop::marshal_as<String ^>(ip_address_stream);
				//check if connection is established
				if (my_ssh_session == 0) {	//connection is NOT established
					if (main_debug == true) {
						printf("Cannot connect to server\n");
					}
				}
				else {
					connection_established = true;
					if (main_debug == true) {
						printf("Connection established\n");
					}
					//check second session for errors
					if (sonar_ssh_session != 0) {
						if (main_debug == true) {
							printf("Second Connection established\n");
						}
					}
					//print the gotten IP to the label3				
					label3->Text = ip_address2;
					label3->ForeColor = System::Drawing::Color::Green;

					//update gui
					label4->Text = "Connected!";
					label4->ForeColor = System::Drawing::Color::Green;
					//start backgroundWorker1 for sonar read
					backgroundWorker1->RunWorkerAsync();
					//create a passer object for padthread
					GamepadRead^ passer = gcnew GamepadRead;
					//fill the passer with necessary data 
					passer->pub_ssh_session = my_ssh_session;
					passer->ip_address_stream = ip_address2;
					//pass sessions to public variables
					sonar_pub_ssh_session = sonar_ssh_session;
					pub_ssh_session = my_ssh_session;

					//check type of input
					if (DX_enable == false) {	//Xinput selected
						//create a padthread with passer's data calling XinputRead function
						padthread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(passer, &GamepadRead::XinputRead));
						//set flags
						xinputhread_started = true;
						running_xinput = true;
						running_Dxinput = false;
					}
					else {//DirectInput selected
						//create a padthread with passer's data calling DxinputRead function
						padthread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(passer, &GamepadRead::DxinputRead));
						//set flags
						dxthread_started = true;
						running_xinput = false;
						running_Dxinput = true;
					}
					//start padthread
					padthread->Start();
				}
			}
		}
		else {
			if (main_debug == true) {
				printf("Connection already established!\n");
			}
			//print the gotten IP to the label3				
			label3->Text = ip_address2;
			label3->ForeColor = System::Drawing::Color::Red;
		}
		
}

//close button
private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) {
	//set flags in way to terminate all threads and then close the app
	stop_strem = true;
	terminate_stream_proc = true;
	running_xinput = false;
	running_Dxinput = false;
	Application::Exit();
}

//DX button
private: System::Void DX_Click(System::Object^  sender, System::EventArgs^  e) {
	System::Threading::Thread^ padthread;
	GamepadRead^ passer = gcnew GamepadRead;
	//check if a SSH connection is established
	if (connection_established == true) {
		//connection is established
		//passer gets the ssh_session for padthread
		passer->pub_ssh_session = pub_ssh_session;
		//enable Direct Input
		if (DX_enable == false) {
			DX_enable = true;
			//update gui
			DX_label->Text = "DX enabled";
			DX_label->ForeColor = System::Drawing::Color::Green;
			//check if padthread with direct input has created before
			if (dxthread_started == false) {
				//padthread with direct input has NOT created before so we create one now
				padthread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(passer, &GamepadRead::DxinputRead));
				//update flags using SRW locks
				AcquireSRWLockExclusive(&Dxlock);
				//Xinput padthread runs now on an idle endless loop
				running_xinput = false;
				running_Dxinput = true;
				//start the padthread with direct input
				padthread->Start();
				dxthread_started = true; 
				ReleaseSRWLockExclusive(&Dxlock);
			}
			else {
				//padthread with direct input has created before so we update flags using SRW locks
				AcquireSRWLockExclusive(&Dxlock);
				//Xinput padthread runs now on an idle endless loop
				running_xinput = false;
				//direct input padthread continue its work
				running_Dxinput = true;
				ReleaseSRWLockExclusive(&Dxlock);
			}
		}
		//enable Xinput
		else {
			DX_enable = false;
			//update gui
			DX_label->Text = "DX disabled";
			DX_label->ForeColor = System::Drawing::Color::Gray;
			//check if padthread with Xinput has created before
			if (xinputhread_started == false) {
				//padthread with Xinput has NOT created before so we create one now
				padthread = gcnew System::Threading::Thread(gcnew System::Threading::ThreadStart(passer, &GamepadRead::XinputRead));
				//update flags using SRW locks
				AcquireSRWLockExclusive(&Dxlock);
				running_xinput = true;
				//direct input padthread runs now on an idle endless loop
				running_Dxinput = false;
				//start the padthread with Xinput
				padthread->Start();
				xinputhread_started = true;
				ReleaseSRWLockExclusive(&Dxlock);
			}
			else {
				//padthread with Xinput has created before so we update flags using SRW locks
				AcquireSRWLockExclusive(&Dxlock);
				//direct input padthread runs now on an idle endless loop
				running_Dxinput = false;
				//Xinput padthread continue its work
				running_xinput = true;
				ReleaseSRWLockExclusive(&Dxlock);
			}

		}
	}
	//connection is NOT established just change the flags
	else {
		if (DX_enable == false) {
			DX_enable = true;
			//update gui
			DX_label->Text = "DX enabled";
			DX_label->ForeColor = System::Drawing::Color::Green;
			
		}
		else {
			DX_enable = false;
			//update gui
			DX_label->Text = "DX disabled";
			DX_label->ForeColor = System::Drawing::Color::Gray;
		}
	}
}

private: System::Void backgroundWorker1_DoWork(System::Object^  sender, System::ComponentModel::DoWorkEventArgs^  e) {
	int i, fnl_sonar[5];
	std::vector<int> sonar;
	for (;;) {
		//get sonar value
		sonar = sonaread(sonar_pub_ssh_session);
		printf("Printing vector FROM main!\n");
		for (auto i = sonar.begin(); i != sonar.end(); ++i) {
			std::cout << *i << "\n";
			fnl_sonar[i] = *i;
			printf("Sonar[%d] = %d \n", i, fnl_sonar[i]);
		}
		Sleep(1000);
		//send report with the value
		//backgroundWorker1->ReportProgress(sonar);	
	}
}

 private: System::Void backgroundWorker1_ProgressChanged(System::Object^ sender, System::ComponentModel::ProgressChangedEventArgs^ e)
		 {
			 label5->Text = e->ProgressPercentage.ToString();	//update the label5
		 }
};




}

