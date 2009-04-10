/*
 * (C) Copyright 2001-2009 Diomidis Spinellis.  All rights reserved.
 *
 * You may only use this code if you agree to the terms of the CScout
 * Source Code License agreement (see License.txt).
 * If you do not agree to the terms, do not use the code.
 *
 * IFstream wrapper for providing a fast tellg()
 * The current (gcc 3.4.2) implementation calls pubseek_off,
 * and then seekoff, which clears the input buffer.
 * Thus, every tellg amounts to an lseek and a read system call.
 *
 * This file is included by fchar.h; do not include it directly.
 * If a C++ library optimizes the getg() method to avoid the
 * overhead the buffer flush and the lseek, remove the class
 * and add the following:
 * typedef ifstream fifstream;
 *
 * After applying this change, the runtime changes from:
 * real    0m7.782s
 * user    0m5.992s
 * sys     0m1.711s
 * to:
 * real    0m4.524s
 * user    0m4.409s
 * sys     0m0.041s
 *
 * $Id: fifstream.h,v 1.5 2009/04/10 08:19:57 dds Exp $
 */

#ifndef FIFSTREAM_
#define FIFSTREAM_

#include <fstream>

using namespace std;

#include "error.h"

class fifstream {
private:
	bool dirty;		// True if we can't use mypos
	ifstream i;		// The underlying ifstream
	long mypos;		// Cached position
public:
	fifstream() {}
	fifstream(const char *s, ios_base::openmode mode = ios_base::out) : i(s, mode) {
		// If the file is not binary, this optimization will not work
		csassert(mode & ios::binary);
		dirty = true;
	}

	// fifstream supports a subset of the ifstream methods
	bool is_open() { return i.is_open(); }
	void close() {
		i.close();
		dirty = true;
	}
	void clear(ifstream::iostate state = ifstream::goodbit) {
		i.clear(state);
		dirty = true;
	}
	void open(const char *s, ios_base::openmode mode = ios_base::out) {
		// If the file is not binary, this optimization will not work
		csassert(mode & ios::binary);
		i.open(s, mode);
		dirty = true;
	}
	bool fail() const {
		return i.fail();
	}
	bool eof() const {
		return i.eof();
	}
	ifstream::pos_type tellg() {
		if (dirty) {
			dirty = false;
			mypos = (unsigned long)i.tellg();
		}
		return (ifstream::pos_type)mypos;
	}
	ifstream::int_type get() {
		ifstream::int_type r = i.get();
		if (r == EOF)
			dirty = true;
		else
			mypos++;
		return r;
	}
	fifstream &putback(char c) {
		i.putback(c);
		mypos--;
		return *this;
	}
	fifstream &seekg(ifstream::pos_type pos) {
		i.seekg(pos);
		dirty = true;
		return *this;
	}
};

#endif /* FIFSTREAM_ */
