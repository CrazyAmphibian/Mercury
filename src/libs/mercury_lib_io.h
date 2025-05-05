#pragma once
/*
 io library.
 contains functions for file reading or modification
*/
#include "../mercury.h"

#define MERCURY_LIB_IO



void mercury_lib_io_open(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_read(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_close(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_write(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_getfiles(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_getdirs(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_lines(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_post(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_prompt(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_remove(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_removedir(mercury_state* M, mercury_int args_in, mercury_int args_out);
void mercury_lib_io_createdir(mercury_state* M, mercury_int args_in, mercury_int args_out);

