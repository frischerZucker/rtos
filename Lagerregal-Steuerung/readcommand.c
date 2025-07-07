/*
 * readcommand.c
 *
 *  Created on: 13.01.2010
 *      Author: Oliver Jack
 */

#include <stdio.h>
#include <string.h>

#include "ioLib.h"

#include "headers/readcommand.h"

int input_available(){
	int bytes = 0;
	
	if (ioctl(STD_IN, FIONREAD, &bytes) == ERROR) {
		printf("ERROR: ioctl failed!\n");
		return false;
	}
	
	printf("BYTES: %d\n", bytes);
	    	
	return bytes;
}


char cmd_buffer[100];
int buffer_pos = 0;


command readcommand(char *cmd) {
	char line[100];
	command cmd_struct = { .parse_ok = false };
	printf("Bitte Kommando eingeben: ");
	fflush(stdout);

	if (fgets(line, sizeof(line), stdin) != NULL) {
		char cmd[20];
		int x, y;
		if (sscanf(line, "%s%d%d", cmd, &x, &y) == 3) {
			if (strcmp(cmd, "insert") == 0 || strcmp(cmd, "insert ") == 0 ||
				strcmp(cmd, "remove") == 0 ||  strcmp(cmd, "remove ") == 0 ||
			    strcmp(cmd, "vsetspace") == 0 || strcmp(cmd, "vsetspace ") == 0 || 
				strcmp(cmd, "clearspace") == 0 || strcmp(cmd, "clearspace") == 0 ||
				strcmp(cmd, "getspace") == 0) {
				cmd_struct.parse_ok = true;
				strcpy(cmd_struct.cmd, cmd);
				cmd_struct.par1 = x;
				cmd_struct.par2 = y;
			}
		}
	}
	return cmd_struct;
}

//command readcommand(char *cmd) {
//	int x, y;
//	command cmd_struct = {0};
//	printf("Bitte Kommando eingeben: ");
//	fflush(stdout);
//	
//	if(!input_available()) {
//		cmd_struct.parse_ok = false;
//		cmd_struct.no_input = true;
//		return cmd_struct;
//	}
//	
//	cmd_struct.no_input = false;
//	
//	if (scanf("%[^[][%d][%d]", cmd, &x, &y)) {
//		if (strcmp(cmd, "insert") == 0 || strcmp(cmd, "remove") == 0 || strcmp(
//				cmd, "vsetspace") == 0 || strcmp(cmd, "clearspace") == 0) {
//			cmd_struct.parse_ok = true;
//			strcpy(cmd_struct.cmd, cmd);
//			cmd_struct.par1 = x;
//			cmd_struct.par2 = y;
//		}
//		else {
//			cmd_struct.parse_ok = false;
//		}
//	}
//	return cmd_struct;
//}
