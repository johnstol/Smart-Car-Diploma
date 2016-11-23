#pragma once
#include "libssh\libssh.h"
#include <stdio.h>
#include <string>
bool ssh_debug = true;//comment for no debug execution
//bool ssh_debug=false; //comment for normal execution
//create ssh session
ssh_session create_ssh_connection(std::string ip_address) {
	ssh_session my_ssh_session;

	int rc, port = 22 ,verbosity = SSH_LOG_PROTOCOL;
	char *password;
	// Open session and set options
	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL)
		exit(-1);
	//ip option
	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, ip_address.c_str());
	//username option
	ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, "john");
	//quantity of messages that are printed option
	ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);

	// Connect to server
	rc = ssh_connect(my_ssh_session);
	//check if connection is ok
	if (rc != SSH_OK)
	{
		if (ssh_debug == true) {
			printf("Error Connecting: %s\n", ssh_get_error(my_ssh_session));
		}
		ssh_free(my_ssh_session); //free the session
		return 0;
	}
	else {
		// Authenticate ourselves
		password = "mariamatz";
		//send password 
		rc = ssh_userauth_password(my_ssh_session, NULL, password);
		//check if authentication is successfull
		if (rc != SSH_AUTH_SUCCESS)
		{
			if (ssh_debug == true) {
				printf("Error authenticating with password: %s\n", ssh_get_error(my_ssh_session));	//get error code
			}
			ssh_disconnect(my_ssh_session);	//disconnect from session
			ssh_free(my_ssh_session);	//free the session
			return 0;
		}
				
	}
	return my_ssh_session;
};

int shell_session(ssh_session session, std::string my_command, bool sonar_read) {
	ssh_channel kanali1;

	int rc2;
	char buffer[256];
	int nbytes, sonar_var;
	//create a channel for communication
	kanali1 = ssh_channel_new(session);
	//check for errors
	if (kanali1 == NULL) {
		if (ssh_debug == true) {
			printf("\n channel error!!!!\n");
		}
	}
	//Open a session channel
	rc2 = ssh_channel_open_session(kanali1);
	//check for errors
	if (rc2 != SSH_OK) {
		if (ssh_debug == true) {
			printf("\n open_session error!!!!\n");
		}
		ssh_channel_free(kanali1);
	}
	//Run a shell command without an interactive shell
	rc2 = ssh_channel_request_exec(kanali1, my_command.c_str());
	//check for errors
	if (rc2 != SSH_OK) {
		if (ssh_debug == true) {
			printf("\n channel request error!!!!\n");
		}
		ssh_channel_free(kanali1);
		ssh_channel_close(kanali1);
	}
	//	Read data from the channel
	nbytes = ssh_channel_read(kanali1, buffer, sizeof(buffer), 0);
	while (nbytes > 0)
	{	//with success output gets printed
		if (fwrite(buffer, 1, nbytes, stdout) != nbytes)	
		{	//fwrite faced some errors
			ssh_channel_close(kanali1);
			ssh_channel_free(kanali1);
			if (ssh_debug == true) {
				printf("\n fwrite error!!!!\n");
			}
		}
		nbytes = ssh_channel_read(kanali1, buffer, sizeof(buffer), 0); //re-read
	}
	if (nbytes < 0)	//channel read failed
	{
		ssh_channel_close(kanali1);
		ssh_channel_free(kanali1);
		if (ssh_debug == true) {
			printf("\n nbytes error!!!!\n");
		}
	}
	//Send an end of file on the channel.
	ssh_channel_send_eof(kanali1);
	//clode the channel
	ssh_channel_close(kanali1);
	//free the channel
	ssh_channel_free(kanali1);
	//if the function is called for sonar reading return sonar value
	if (sonar_read == true) {
		sonar_var=atoi(buffer);	//convert string to int
		if (ssh_debug == true) {
			printf("ssh call get sonar: %d\n", sonar_var);
		}
		return sonar_var;
	}
	else {
		return SSH_OK;
	}
}

