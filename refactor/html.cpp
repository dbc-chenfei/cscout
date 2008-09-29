/*
 * (C) Copyright 2008 Diomidis Spinellis.
 *
 * HTML utility functions
 *
 * $Id: html.cpp,v 1.3 2008/09/29 09:17:36 dds Exp $
 */

#include <map>
#include <string>
#include <deque>
#include <vector>
#include <stack>
#include <iterator>
#include <iostream>
#include <fstream>
#include <list>
#include <set>
#include <utility>
#include <functional>
#include <algorithm>		// set_difference
#include <cctype>
#include <sstream>		// ostringstream
#include <cstdio>		// perror, rename
#include <cstdlib>		// atoi
#include <cerrno>		// errno

#include "swill.h"
#include "getopt.h"

#include "cpp.h"
#include "debug.h"
#include "error.h"
#include "ytab.h"
#include "attr.h"
#include "metrics.h"
#include "fileid.h"
#include "tokid.h"
#include "token.h"
#include "ptoken.h"
#include "fchar.h"
#include "pltoken.h"
#include "macro.h"
#include "pdtoken.h"
#include "eclass.h"
#include "ctoken.h"
#include "type.h"
#include "stab.h"
#include "license.h"
#include "fdep.h"
#include "version.h"
#include "call.h"
#include "html.h"
#include "fileutils.h"
#include "option.h"

/*
 * Return as a C string the HTML equivalent of character c
 * Handles tab-stop expansion provided all output is processed through this
 * function
 */
const char *
html(char c)
{
	static char str[2];
	static int column = 0;
	static vector<string> spaces(0);

	switch (c) {
	case '&': column++; return "&amp;";
	case '<': column++; return "&lt;";
	case '>': column++; return "&gt;";
	case '"': column++; return "&quot;";
	case ' ': column++; return "&nbsp;";
	case '\t':
		if ((int)(spaces.size()) != Option::tab_width->get()) {
			spaces.resize(Option::tab_width->get());
			for (int i = 0; i < Option::tab_width->get(); i++) {
				string t;
				for (int j = 0; j < Option::tab_width->get() - i; j++)
					t += "&nbsp;";
				spaces[i] = t;
			}
		}
		return spaces[column % Option::tab_width->get()].c_str();
	case '\n':
		column = 0;
		return "<br>\n";
	default:
		column++;
		str[0] = c;
		return str;
	}
}

// HTML-encode the given string
string
html(const string &s)
{
	string r;

	for (string::const_iterator i = s.begin(); i != s.end(); i++)
		r += html(*i);
	return r;
}

// Output s as HTML in of
void
html_string(FILE *of, string s)
{
	for (string::const_iterator i = s.begin(); i != s.end(); i++)
		fputs(html(*i), of);
}


// Create a new HTML file with a given filename and title
// The heading, if not given, will be the same as the title
void
html_head(FILE *of, const string fname, const string title, const char *heading)
{
	swill_title(title.c_str());
	if (DP())
		cerr << "Write to " << fname << endl;
	fprintf(of,
		"<!doctype html public \"-//IETF//DTD HTML//EN\">\n"
		"<html>\n"
		"<head>\n"
		"<meta name=\"GENERATOR\" content=\"CScout %s - %s\">\n",
		Version::get_revision().c_str(),
		Version::get_date().c_str());
	fputs(
		"<meta http-equiv=\"Content-Style-Type\" content=\"text/css\">"
		"<style type=\"text/css\" >"
		"<!--\n", of);

	ifstream in;
	string css_fname;
	if (cscout_input_file("style.css", in, css_fname)) {
		int val;
		while ((val = in.get()) != EOF)
			putc(val, of);
	} else
		fputs(
		#include "css.c"
		, of);
	fputs(
		"-->"
		"</style>"
		"</head>", of);
	fprintf(of,
		"<title>%s</title>\n"
		"</head>\n"
		"<body>\n"
		"<h1>%s</h1>\n",
		title.c_str(),
		heading ? heading : title.c_str());
}

// And an HTML file end
void
html_tail(FILE *of)
{
	extern Attributes::size_type current_project;

	if (current_project)
		fprintf(of, "<p> <b>Project %s is currently selected</b>\n", Project::get_projname(current_project).c_str());
	fprintf(of,
		"<p>"
		"<a href=\"index.html\">Main page</a>\n"
		" &mdash; Web: "
		"<a href=\"http://www.spinellis.gr/cscout\">Home</a>\n"
		"<a href=\"http://www.spinellis.gr/cscout/doc/index.html\">Manual</a>\n"
		"<br><hr><div class=\"footer\">CScout %s &mdash; %s",
		Version::get_revision().c_str(),
		Version::get_date().c_str());
#ifdef COMMERCIAL
	fprintf(of, " &mdash; Licensee: %s", licensee);
#else
	fprintf(of, " &mdash; Unsupported version; can only be used on open-source software.");
#endif
	fprintf(of, "</div></body></html>\n");
}

// Return a function's label, based on the user's preferences
string
file_label(Fileid f, bool hyperlink)
{
	string result;
	char buff[256];

	if (hyperlink) {
		snprintf(buff, sizeof(buff), "<a href=\"file.html?id=%d\">", f.get_id());
		result = buff;
	}
	switch (Option::igraph_show->get()) {
	case 'p':			// Show complete paths
		result += f.get_path() + "/";
		/* FALLTHROUGH */
	case 'n':			// Show only file names
		result += f.get_fname();
		break;
	case 'e':			// Show only edges
		result += " ";
		break;
	}
	if (hyperlink)
		result += "</a>";
	return (result);
}

// Return a function's label, based on the user's preferences
string
function_label(Call *f, bool hyperlink)
{
	string result;
	char buff[256];

	if (hyperlink) {
		snprintf(buff, sizeof(buff), "<a href=\"fun.html?f=%p\">", f);
		result = buff;
	}
	if (Option::cgraph_show->get() == 'f')		// Show files
		result += f->get_site().get_fileid().get_fname() + ":";
	else if (Option::cgraph_show->get() == 'p')	// Show paths
		result += f->get_site().get_fileid().get_path() + ":";
	if (Option::cgraph_show->get() != 'e')		// Empty labels
		result += f->get_name();
	if (hyperlink)
		result += "</a>";
	return (result);
}

// Display a system error on the HTML output.
void
html_perror(FILE *of, const string &user_msg, bool svg)
{
	string error_msg(user_msg + ": " + string(strerror(errno)) + "\n");
	fputs(error_msg.c_str(), stderr);
	if (svg)
		fprintf(of, "<?xml version=\"1.0\" ?>\n"
			"<svg>\n"
			"<text  x=\"20\" y=\"50\" >%s</text>\n"
			"</svg>\n", error_msg.c_str());
	else {
		fputs(error_msg.c_str(), of);
		fputs("</p><p>The operation you requested is incomplete.  "
			"Please correct the underlying cause, and return to the "
			"CScout <a href=\"index.html\">main page</a> to retry the operation.</p>", of);
	}
}
