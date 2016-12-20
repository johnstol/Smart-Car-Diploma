#pragma once
#include <stdio.h>
#include <string.h>
#include <iostream>
#include<fstream>
#include <io.h> 
#include <libssh\libssh.h>
#include "ssh_func.h"


int sonaread(ssh_session ssh_session) {
	std::string mycommand;
	int sonar;

	//bool sonr_debug = true;//comment for no debug execution
	bool sonr_debug = false;//comment for normal execution

	mycommand = "cat /home/onram/sonar.txt";	//read sonar
	sonar = shell_session(ssh_session, mycommand,true);
	if(sonr_debug == true) {
		printf("Sonar from thread: %d\n", sonar);
	}
	return sonar;
}