#include "../mercury.hpp"
#include "../mercury_error.hpp"

#include <cstdlib>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
#include <pthread.h>
#endif




#ifdef _WIN32

//what the fuck is an LPVOID?
DWORD WINAPI threadfunction(LPVOID param) {
	if (!param)return 0;
	mercury_threadholder* threadvar= (mercury_threadholder*)param;

	while(mercury_stepstate(threadvar->state));

	threadvar->finished = true;
	return 0;
}
#else
void* threadfunction(void* param) {
	if (!param)return nullptr;
	mercury_threadholder* threadvar = (mercury_threadholder*)param;

	while (mercury_stepstate(threadvar->state));

	threadvar->finished = true;
	return nullptr;
}
#endif




//creates a thread, given a function and input variable. optional table to act as the enviroment. defaults current env.
void mercury_lib_thread_new(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (!args_in) {
		mercury_raise_error_nonpointer(M, M_ERROR_NOT_ENOUGH_ARGS, args_in, 1);
		return;
	}
	if (!args_out)return;


	mercury_variable table_var;
	mercury_variable func_var;

	mercury_variable* vart = nullptr;
	if (args_in > 2) {
		vart = (mercury_variable*)malloc(sizeof(mercury_variable) * args_in - 2);
		if (!vart) {
			mercury_raise_error(M, M_ERROR_ALLOCATION);
			return;
		}
	}
	

	for (mercury_int i = 2; i < args_in; i++) {
		mercury_popstack(M, vart+i - 2);
	}


	if (args_in >=2) {
		mercury_popstack(M, &table_var);
		mercury_popstack(M,&func_var);
	}
	else {
		mercury_popstack(M, &func_var);
		table_var.type = M_TYPE_NIL;
	}


	if ( func_var.type != M_TYPE_FUNCTION) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, func_var.type, M_TYPE_FUNCTION,1);
		return;
	}
	if ((table_var.type && table_var.type != M_TYPE_TABLE)) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, table_var.type, M_TYPE_TABLE, 2);
		return;
	}


	mercury_threadholder* t=(mercury_threadholder*)malloc(sizeof(mercury_threadholder));
	if (!t) {
		mercury_raise_error(M, M_ERROR_ALLOCATION);
		return;
	}

	t->finished = false;
	t->refrences = 1;
	if (table_var.type) {
		t->state = mercury_newstate();
		t->customenv = true;
		if (t->state) {
			free(t->state->enviroment);
			t->state->enviroment = (mercury_table*)table_var.data.p;
			//printf("making with custom\n");
		}
	}
	else {
		//printf("making with parent\n");
		t->state = mercury_newstate(M);
		t->customenv = false;
	}
	
	mercury_variable out;
	out.constant = false;
	out.data.i = 0;
	out.type = M_TYPE_NIL;
	
	if (t->state) {
		t->state->bytecode.numberofinstructions = ((mercury_function*)func_var.data.p)->numberofinstructions;
		t->state->bytecode.instructions = ((mercury_function*)func_var.data.p)->instructions;

		if (vart) {
			for (mercury_int i = 0; i < args_in - 2; i++) {
				mercury_pushstack(t->state, vart+i);
			}
			free(vart);
			vart = nullptr;
		}

#if defined(_WIN32) || defined(_WIN64)

		HANDLE h = CreateThread(NULL, 0, threadfunction, (LPVOID)t, 0, NULL);
		if (h) {
			t->threadobject = h;
			out.type = M_TYPE_THREAD;
			out.data.p = t;
		}
		else {
			if (table_var.type)t->state->enviroment = nullptr; //if we set a custom env, don't destroy the data.
			t->state->bytecode.instructions = nullptr; //don't mess with the host function's data
			mercury_destroystate(t->state);
			free(t);
		}

#else

		int c=pthread_create(&(t->threadobject), NULL, threadfunction, t);
		if (c) {
			out.type = M_TYPE_THREAD;
			out.data.p = t;
		}
		else {
			if (table_var.type)t->state->enviroment = nullptr; //if we set a custom env, don't destroy the data.
			t->state->bytecode.instructions = nullptr; //don't mess with the host function's data
			mercury_destroystate(t->state);
			free(t);
		}


#endif


	}

	mercury_pushstack_unrefed(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


//bool if thread is finished. simple.
void mercury_lib_thread_checkfinish(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out)return;


	mercury_variable in;
	mercury_popstack(M,&in);
	if (in.type != M_TYPE_THREAD) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, in.type, M_TYPE_THREAD, 1);
		return;
	}

	mercury_variable out;
	out.constant = false;
	out.type = M_TYPE_BOOL;
	out.data.i = ((mercury_threadholder*)in.data.p)->finished ? 1 : 0;


	mercury_pushstack(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}



//gets a value
void mercury_lib_thread_getvalue(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out)return;

	mercury_variable in;
	mercury_popstack(M, &in);
	if (in.type != M_TYPE_THREAD) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, in.type, M_TYPE_THREAD, 1);
		return;
	}

	mercury_threadholder* t=(mercury_threadholder*)in.data.p;
	if (!t->finished) {
#if defined(_WIN32) || defined(_WIN64)
		WaitForSingleObject(t->threadobject, INFINITE);
		CloseHandle(t->threadobject);
		t->threadobject = NULL;
#else
		pthread_join(t->threadobject, NULL);
		t->threadobject = NULL;
#endif
	}
	mercury_variable out;
	mercury_pullstack(t->state, &out); //take the bottom of stack. it's the proper order with returns.
	mercury_pushstack(M, &out);


	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}


//dire circumstances. use with caution
void mercury_lib_thread_abort(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;

	mercury_variable in;
	mercury_popstack(M,&in);
	if (in.type != M_TYPE_THREAD) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, in.type, M_TYPE_THREAD, 1);
		return;
	}

	mercury_threadholder* t = (mercury_threadholder*)in.data.p;
	if (!t->finished) {
#if defined(_WIN32) || defined(_WIN64)
		TerminateThread(t->threadobject, 0);
		CloseHandle(t->threadobject);
		t->threadobject = NULL;
#else
		pthread_cancel(t->threadobject);
		pthread_join(t->threadobject, NULL);
		t->threadobject = NULL;
#endif
	}
	t->finished = true;


	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 0);
}



//returns number of values. 0 if not finished.
void mercury_lib_thread_getnumvalues(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out)return;

	mercury_variable in;
	mercury_popstack(M,&in);
	if (in.type != M_TYPE_THREAD) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, in.type, M_TYPE_THREAD, 1);
		return;
	}

	mercury_threadholder* t = (mercury_threadholder*)in.data.p;

	mercury_variable out;
	out.constant = false;
	out.type = M_TYPE_INT;
	out.data.i = 0;

	if (t->finished) {
		out.data.i = t->state->sizeofstack;
	}
	

	mercury_pushstack(M, &out);


	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}



//waits for a closure
void mercury_lib_thread_waitfor(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;

	mercury_variable in;
	mercury_popstack(M,&in);
	if (in.type != M_TYPE_THREAD) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, in.type, M_TYPE_THREAD, 1);
		return;
	}

	mercury_threadholder* t = (mercury_threadholder*)in.data.p;
	if (!t->finished) {
#if defined(_WIN32) || defined(_WIN64)
		WaitForSingleObject(t->threadobject, INFINITE);
		CloseHandle(t->threadobject);
		t->threadobject = NULL;
#else
		pthread_join(t->threadobject, NULL);
		t->threadobject = NULL;
#endif
	}

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 0);
}


//inverse of is finished.
void mercury_lib_thread_checkrunning(mercury_state* const M_CPP_restrict M, const mercury_int args_in, const mercury_int args_out) {
	if (MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_INPUT_ARGS(M, args_in, 1))return;
	if (!args_out)return;

	mercury_variable in;
	mercury_popstack(M,&in);
	if (in.type != M_TYPE_THREAD) {
		mercury_raise_error_nonpointer(M, M_ERROR_WRONG_TYPE, in.type, M_TYPE_THREAD, 1);
		return;
	}

	mercury_variable out;
	out.constant=false;
	out.type = M_TYPE_BOOL;
	out.data.i = ((mercury_threadholder*)in.data.p)->finished ? 0 : 1;


	mercury_pushstack(M, &out);

	MERCURY_CFUNCTION_ENSURE_CORRECT_NUMBER_OUTPUT_ARGS(M, args_out, 1);
}




