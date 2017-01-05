#pragma once
#include <stdio.h>
#include <string.h>
#include <iostream>
#include<fstream>
#include <io.h> 
#include <libssh\libssh.h>
#include "ssh_func.h"


std::vector<int> sonaread(ssh_session ssh_session) {
	std::string mycommand, sonars_read;
	int i;
	std::vector<int> sonar(5);
	std::string delimeter = "-";
	size_t pos = 0;
	//bool sonr_debug = true;//comment for no debug execution
	bool sonr_debug = false;//comment for normal execution

	mycommand = "cat /home/onram/sonar.txt";	//read sonar
	sonars_read = sonar_shell_session(ssh_session, mycommand);
	
	for (i = 0; i<5; i++) {	// split buffer string in 5 pieces (one for every sonar) using "-" delimeter , convert them to int and pass them to an int array 
		sonar[i] = atoi(sonars_read.substr(0, sonars_read.find(delimeter)).c_str());
		sonars_read.erase(0, sonars_read.find(delimeter) + 1);
	}

	if (sonr_debug == true) {
		printf("Printing vector!\n");
		for (auto i = sonar.begin(); i != sonar.end(); ++i) {
			std::cout << *i << "\n";
		}
	}

	return sonar;
}