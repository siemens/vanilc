// Copyright (c) 2015 Siemens AG, Author: Andreas Weinlich
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>

#include <map>

#include "vanilcDefinitions.h"

namespace vanilc {

using namespace std;
using namespace cv;

class GenericParameter {
public:
	GenericParameter(char letter, string description) : letter(letter), description(description) {};
	virtual string getValue() = 0;
	char getLetter() const { return letter; };
	string getDescription() const { return description; };
	virtual void parse(CommandLineParser p, string key) = 0;
	virtual void read(FileStorage s, string key) = 0;
private:
	const char letter;
	const string description;
};

template <typename T>
class Parameter : public GenericParameter {
public:
	typedef T type;
	Parameter(T value, char letter = 0, string description = "") : GenericParameter(letter, description), value(value) {};
	void set(T value) { this->value = value; };
	T get() const { return value; };
	string getValue() { return static_cast<ostringstream*>( &(ostringstream() << value) )->str(); };
	void parse(CommandLineParser p, string key) { value = p.get<T>(key); };
	void read(FileStorage s, string key) { if(!s[key].empty()) s[key] >> value; };
private:
	T value;
};

template <>
class Parameter<string> : public GenericParameter {
public:
	typedef string type;
	Parameter(string value, char letter = 0, string description = "") : GenericParameter(letter, description), value(value) {};
	void set(string value) { this->value = value; };
	string get() const { return value; };
	string getValue() { return value; };
	void parse(CommandLineParser p, string key) { value = p.get<string>(key); };
	void read(FileStorage s, string key) { if(!s[key].empty()) s[key] >> value; };
private:
	string value;
};

template <>
class Parameter<bool> : public GenericParameter {
public:
	typedef bool type;
	Parameter(bool value, char letter = 0, string description = "") : GenericParameter(letter, description), value(value) {};
	void set(bool value) { this->value = value; };
	bool get() const { return value; };
	string getValue() { if(value) return "true"; else return "false"; };
	void parse(CommandLineParser p, string key) { value = p.get<bool>(key); };
	void read(FileStorage s, string key) { if(!s[key].empty()) s[key] >> value; };
private:
	bool value;
};

class Config {
public:
	Config();
	~Config();
	void parse(int argc, char* argv[]);
	void printConfig();
	template<typename T> T get(string key) { return dynamic_cast< Parameter<T>* >(parameters[key])->get(); };
	template<typename T> void set(string key, T value) { dynamic_cast< Parameter<T>* >(parameters[key])->set(value); };

private:
	map<string, GenericParameter*> parameters;

	void insertMoreParameters();
	void checkConfig();
}; // end class Config

class InfoShownException : public Exception { };

class ConfigNotValidException : public Exception {
	virtual const char* what() const throw() { return "Configuration not accepted. Aborting."; }
};

} // end namespace vanilc

