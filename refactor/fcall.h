/* 
 * (C) Copyright 2003 Diomidis Spinellis.
 *
 * Function call graph information
 *
 * $Id: fcall.h,v 1.1 2003/11/17 14:35:46 dds Exp $
 */

#ifndef FCALL_
#define FCALL_

/*
 * Function call information is always associated with Id objects
 * It is thus guaranteed to match the symbol table lookup operations
 */

// C function calling information
class FCall {
private:
	static FCall *current_fun;	// Function currently being parsed
	static set <FCall *> all;	// Set of all functions

	Token declaration;		// Function's first declaration 
					// (could also be reference if implicitly declared)
	Token definition;		// Function's definition
	Type type;			// Function's type
	set <FCall *> call;		// Functions this function calls
	set <FCall *> caller;		// Functions that call this function
	void add_call(FCall *f) { call.insert(f); }
	void add_caller(FCall *f) { caller.insert(f); }
public:
	// Set the funciton currently being parsed
	static void set_current_fun(const Type &t);
	static void set_current_fun(const Id *id);
	// The current function makes a call to f
	static void register_call(FCall *f);

	FCall(const Token& tok, Type typ);
};

#endif // FCALL_
