#pragma once

#include <opencv.hpp>
#include <msclr\marshal_cppstd.h>
bool stop_strem = false,first_time_pushed_camera_button = false, stream_tread_runs=false,terminate_stream_proc=false;

//bool debugstream = true;//comment for no debug execution
bool debugstream = false; //comment for normal execution

using namespace System;
using namespace System::Threading;


public ref class video_stream
{

public:
	System::String ^ ip_address_stream;

	void stream() {
		if (debugstream == true) {
			printf("Thread %s .\n", AppDomain::GetCurrentThreadId().ToString());
		}
		
		cv::VideoCapture vcap;
		cv::Mat image;

		//convert std string to System string
		msclr::interop::marshal_context context;
		std::string videoStreamAddress = context.marshal_as<std::string>(ip_address_stream);

		if (debugstream == true) {
			std::cout << "MY IP: " << videoStreamAddress << "\n";
		}
		
		//open the video stream and make sure it's opened
		if (!vcap.open(videoStreamAddress)) {
			printf("Error opening video stream or file \n");
		}
		else {
			for (;;) {
				while (stop_strem == false) {

					if (!vcap.read(image)) {	//capture image
						cv::waitKey();			//if no image captured wait
					}
					else {
						cv::imshow("Output Window", image);	//open output window and show the image
						if (cv::waitKey(1) >= 0) break;	//wait 1ms if key 1 from keyboard pressed -> break
					}
				}
				Sleep(10);
				if (terminate_stream_proc == true) break;	////terminate the tread the app is closing
			}
		}
		vcap.release();
	};

};