#include "stdafx.h"
#include "md5.h"
#include <iostream>
#include <regex>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <string>
#include <string.h>
#include <stdio.h>
#include <Windows.h> //to hide password text
using namespace std;

//functions for initialisation
string createUser();
string getUserComment();
string getPassword();
string getUserClearance();
void addToTxt(string, string, string, string);

//utility functions
void showInput(bool);
int generateSalt();

//validation functions
void validateData(string &, string); //validation function, checks depending on the type 
bool userExists(string); //returns true if the user exists
bool confirmPassword(string, string); //checks if password entered is the same

									  //functions for normal run
void getCommand(string);

int main(int argc, char* argv[])
{
	//seeding the rand value
	srand((unsigned)time(NULL));

	//TODO: groups/roles, home directory(?)
	if (strcmp(argv[1], "-i") == 0)
	{
		string user = createUser();
		string comment = getUserComment();
		string password = getPassword();
		string clearance = getUserClearance();
		addToTxt(user, comment, password, clearance);
	}

	cin.get();
	return 0;
}

string createUser()
{
	string input;
	cout << "Username: ";
	getline(cin, input);
	if (userExists(input))
		cout << "The username you have selected has been taken.\nThe program will now terminate.";

	return input;
}

string getUserComment()
{
	string input;
	cout << "Please enter any comment for the user (optional): ";
	getline(cin, input);

	return input;
}

string getPassword()
{
	string password;
	string passwordCheck;
	showInput(false);

	cout << "Please set your password now.\nPasswords have to be between 8 to 16 characters and must contain at least 1 digit and 1 symbol." << endl;
	cout << "Password: ";
	getline(cin, password);
	validateData(password, "password");

	cout << endl << "Confirm password: ";
	getline(cin, passwordCheck);
	cout << endl;
	confirmPassword(password, passwordCheck);

	showInput(true);

	return password;
}

string getUserClearance()
{
	string clearance;
	cout << "User clearance (0, 1 or 2): ";
	getline(cin, clearance);
	validateData(clearance, "clearance");

	return clearance;
}

//adds data to passwd, shadow and salt
void addToTxt(string user, string comment, string password, string clearance)
{
	MD5 md5;
	ofstream file;
	ifstream inFile;
	string passSalt, line;
	char *passSaltHash;
	int salt = generateSalt();
	int userCounter = 0;

	//salt.txt
	file.open("salt.txt", ofstream::app | ofstream::out);

	file << user + ":" + to_string(salt);

	file.close();

	//TODO: maybe add some stuff about passwords
	//shadow.txt
	file.open("shadow.txt", ofstream::app | ofstream::out);

	passSalt = password + to_string(salt);
	passSaltHash = new char[passSalt.length() + 1];
	strcpy_s(passSaltHash, sizeof(passSalt.length() + 1), passSalt.c_str());
	passSaltHash = md5.digestString(passSaltHash); //TODO: git gud

	file << user + ":" + passSaltHash + ":" + clearance;

	file.close();
	delete[] passSaltHash;

	//passwd.txt
	inFile.open("passwd.txt");

	while (inFile.good())
	{
		while (getline(inFile, line))
		{
			userCounter++;
		}
	}
	inFile.close();

	file.open("passwd.txt", ofstream::app | ofstream::out);
	//TODO: add group ID and home directory
	file << user + ":" + "x" + ":" + to_string(1000 + userCounter) /*+ ":" + groupID +*/ + ":" + comment;/*+ ":" + homeDirectory +*/

	file.close();
}

//hides input in console
void showInput(bool enable = true)
{
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);

	if (!enable)
		mode &= ~ENABLE_ECHO_INPUT;
	else
		mode |= ENABLE_ECHO_INPUT;

	SetConsoleMode(hStdin, mode);
}

//generates a salt value for a user
int generateSalt()
{
	//TODO: maybe change the logic for optimisation(?)
	return (rand() % 90000000 + 10000000);
}

//validates string based on the type of validation needed
void validateData(string &input, string type)
{
	string pattern;

	if (type == "password")
	{
		pattern = "[\\w$-/:-?{-~!\"^_`\\[\\]]{8,16}";
		while (!(regex_match(input, regex(pattern))))
		{
			if (input.length() > 16 || input.length() < 8)
			{
				cout << "\nYou have entered a password that is less than or more than the character limit.\nPlease enter a new password that is within the character limit." << endl;
				cout << "Password: ";
				getline(cin, input);
			}
			else
			{
				cout << "\nYou have entered a password that contains an invalid character.\nAllowed characters are alphanumeric and symbols. The password must be 8 to 16 characters." << endl;
				cout << "Password: ";
				getline(cin, input);
			}
		}
	}
	else if (type == "clearance")
	{
		pattern = "[0-2]";
		while (!(regex_match(input, regex(pattern))))
		{
			cout << "\nYou have entered a incorrect user clearance value.\nPlease enter 0, 1 or 2: ";
			getline(cin, input);
		}
	}
}

//checks if the user exists
bool userExists(string user)
{
	ifstream file;
	string line;
	//TODO: add checking for if passwd doesn't exist (assuming passwd doesn't exist at the beginning)
	file.open("passwd.txt", ifstream::app);

	while (file.good())
	{
		while (!file.eof())
		{
			getline(file, line, ':');
			if (line == user)
			{
				file.close();
				return true;
			}
		}
	}

	file.close();
	return false;
}

//valid password characters are alphanumeric and symbols within 8 to 16 characters

bool confirmPassword(string password, string passwordCheck)
{
	while (password != passwordCheck)
	{
		cout << endl << "Passwords do not match!" << endl;
		cout << "Confirm password: " << endl;
		getline(cin, passwordCheck);
	}

	return true;
}

void getCommand(string input)
{
	getline(cin, input);

	//TODO: add types of commands
}