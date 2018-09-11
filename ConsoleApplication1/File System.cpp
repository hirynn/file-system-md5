#include "stdafx.h"
#include "md5.h"
#include <iostream>
#include <regex>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <Windows.h> //to hide password text
using namespace std;

//functions for initialisation
int initializeUser();
string getUsername();
string getUserComment();
string setPassword();
string setUserClearance();
void addToTxt(string, string, string, string); 

//functions for logging in 
int logIn();
string getPassword();
string getSalt(string);
string generateMD5(string, string);
bool checkShadow(string);
string getUserClearance(string);

//functions after log in
string getFileClearance(string);
void createFile();
void readFile(bool, string);
void writeFile();
void listFile();

//utility functions
void showInput(bool);
int generateSalt();
string appendTxt(string);
int convertToInt(string);

//validation functions
void validateData(string &, string); //validation function, checks depending on the type 
bool userExists(string); //returns true if the user exists
bool Captcha();
bool fileExists(string); //returns true if file exists in file.Store
bool confirmPassword(string, string); //checks if password entered is the same

//functions for normal run
void getCommand(string);

//TODO: add checking for this
string userClearance;

int main(int argc, char* argv[]) 
{
	//init value
	int check = -1; 
	string input = ""; 

	//seeding the rand value
	srand((unsigned)time(NULL));

	//TODO: groups/roles, home directory(?)
	if (argc > 1) //argc = 1 is the exe name
	{
		if (strcmp(argv[1], "-i") == 0)
		{
			initializeUser();
		}
	}
	else if (argc == 1) //if no arguments, the user logs in 
	{
		check = logIn();
	}

	if (check == 1) //if successful log in 
	{
		cout << "\nOptions: (C)reate, (R)ead, (W)rite, (L)ist, (S)ave or (E)xit.";
		getline(cin, input);
		validateData(input, "mainmenu");

		while (input != "E")
		{
			if (input == "C")
				createFile();
			else if (input == "R")
				readFile(false, "");
			else if (input == "W")
				writeFile();
			else if (input == "L")
				listFile();
			else if (input == "S")
				saveFile();


			cout << "\nOptions: (C)reate, (R)ead, (W)rite, (L)ist, (S)ave or (E)xit.";
			getline(cin, input);
			validateData(input, "mainmenu");
		}
	}

	system("pause");
    return 0;
}

int initializeUser()
{
	string user = getUsername();
	if (userExists(user))
	{
		cout << "The username you have selected has been taken.\nThe program will now terminate." << endl;
		return 0;
	}
	string comment = getUserComment();
	string password = setPassword();
	string clearance = setUserClearance();
	addToTxt(user, comment, password, clearance);
}

string getUsername()
{
	string input;
	cout << "Username: ";
	getline(cin, input);

	return input;
}

string getUserComment()
{
	string input;
	cout << "Please enter any comment for the user (optional): ";
	getline(cin, input);

	return input;
}

string setPassword()
{
	string password;
	string passwordCheck;
	showInput(false);
	
	cout << "Please set your password now.\nPasswords have to be between 8 to 16 characters and must contain at least 1 digit or 1 symbol." << endl;
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

string setUserClearance()
{
	string clearance;
	cout << "User clearance (0, 1 or 2): ";
	getline(cin, clearance);
	validateData(clearance, "clearance");

	return clearance;
}

void createFile()
{
	string fileName;
	string fileClearance = userClearance; //only for init purposes

	cout << "Filename: ";
	getline(cin, fileName);
	fileName = appendTxt(fileName);

	do
	{
		if (fileClearance != userClearance)
		{
			cout << "You can only create a file that has the same security clearance level as your account." << endl;
			cout << "Your security clearance level is " << userClearance << endl;
		}

		cout << "Security level (0, 1 or 2): ";
		getline(cin, fileClearance);
		validateData(fileClearance, "clearance");
	} while (fileClearance != userClearance);

	

	//if file doesn't exist
	if (!fileExists(fileName))
	{
		//add file to file.Store
		ofstream file;
		file.open("file.Store", ofstream::out | ofstream::app);

		file << fileName << ":" << fileClearance << endl;

		file.close();
	}
	else
		cout << "The file " << fileName << " already exists.\nThe program will now return to the main menu." << endl;
}

void readFile(bool isBorrowed, string filename)
{
	string line;
	int counter = 0;
	
	if (!isBorrowed)
	{
		cout << "Filename to read: ";
		getline(cin, filename);
		filename = appendTxt(filename);
	}

	string fileClearance = getFileClearance(filename);

	if (fileClearance >= userClearance)
	{
		cout << "\nNow displaying contents on console..." << endl;

		if (fileExists(filename))
		{
			ifstream file;

			file.open(filename, ifstream::in);

			if (file.good())
			{
				cout << endl;
				while (getline(file, line))
				{
					if (isBorrowed)
						cout << ++counter << ". "; //line number

					cout << line << endl;
				}
			}

			file.close();
		}
		else
		{
			cout << "File " << filename << " does not exist!" << endl;
		}
	}
	else if (userClearance > fileClearance) //if user has less clearance than file
	{
		cout << "You do not have the clearance level needed to read this file." << endl;
		cout << "Your clearance level: " << userClearance << endl;
		cout << "Clearance level needed to read " << filename << ": " << fileClearance;
	}
}

//edit a file
void writeFile() 
{
	string stringBuilder;
	string filename, line, input;
	int lineNum = 0, selectedLine = 0, totalLineNum = 0;
	bool appendAtEnd;

	cout << "Filename to write: ";
	getline(cin, filename);
	filename = appendTxt(filename);

	if (fileExists(filename))
	{
		string fileClearance = getFileClearance(filename);

		if (fileClearance == userClearance)
		{
			cout << "Do you want to append to the end of file or edit a selected line?\n(Enter \"y\" for append to end of file, \"n\" to select a line to edit.\n->";
			getline(cin, input);
			validateData(input, "choice");
			if (input == "y")
				appendAtEnd = true;
			else
				appendAtEnd = false;

			cout << "Do you want to display the contents of " << filename << " on console? (y/n)\n->";
			getline(cin, input);
			validateData(input, "choice");
			if (input == "y")
				readFile(true, filename); //display 

			//read total line number
			ifstream inFile;
			inFile.open(filename, ifstream::in);

			if (inFile.good())
			{
				while (getline(inFile, line))
				{
					totalLineNum++;
				}
			}

			inFile.close();

			line = ""; //reset for safety

			//open file for edit
			ofstream outFile;
			inFile.open(filename, ifstream::in);
			if (!appendAtEnd)
				outFile.open(filename, ofstream::out);
			else if (appendAtEnd)
				outFile.open(filename, ofstream::out | ofstream::app);

			//if select specific line to edit
			if (!appendAtEnd)
			{
				do
				{
					do
					{
						if (selectedLine > totalLineNum)
							cout << "You have selected a line that is over the line number in the file." << endl;

						cout << "Please select a line to edit (enter -1 to stop): ";
						getline(cin, input);
						validateData(input, "number");
						selectedLine = convertToInt(input);
					} while (selectedLine > totalLineNum);

					if (selectedLine == -1)
						goto exit;

					//edit specific line
					while (getline(inFile, line))
					{
						lineNum++;
						if (selectedLine == lineNum)
						{
							cout << "Enter your new data\n->";
							getline(cin, line);
							line += "\n";
						}
						outFile << line;
					}

					cout << "Do you want to continue? (y/n)\n->";
					getline(cin, input);
					validateData(input, "choice");
				} while (input != "n");
			}
			else //if append at end
			{
				do
				{
					cout << "Enter your new data\n->";
					getline(cin, line);
					outFile << line;

					cout << "Do you want to continue? (y/n)\n->";
					getline(cin, input);
					validateData(input, "choice");
				} while (input != "n");
			}

			inFile.close();
			outFile.close();
		}
		else if (fileClearance < userClearance)
		{
			cout << "You do not have the clearance level needed to write to this file." << endl;
			cout << "Your clearance level: " << userClearance << endl;
			cout << "Clearance level needed to read " << filename << ": " << fileClearance;
		}
		else if (fileClearance > userClearance)
		{
			cout << "You are not allowed to write to lower level files in accordance to the Strong Star Property." << endl;
		}
	}
	else
	{
		cout << "The file does not exist.\nPlease check the filename or (C)reate it first in the main menu.\nThe program will now return to the main menu." << endl;
	}

exit:
	true;
}

void listFile()
{
	int counter = 0;
	string line;
	ifstream file;
	file.open("file.Store", ifstream::in);

	if (file.good())
	{
		cout << "Displaying files" << "\n=================" << endl;

		while (getline(file, line, ':'))
		{
			counter++;
			cout << counter << ". " << line << endl;
		}
	}
	file.close();
}

//adds data to passwd, shadow and salt
void addToTxt(string user, string comment, string password, string clearance)
{
	MD5 md5;
	ofstream file;
	ifstream inFile;
	string passSalt, line, psh;
	int salt = generateSalt();
	int userCounter = 0;

	//salt.txt
	file.open("salt.txt", ofstream::app | ofstream::out);

	file << user + ":" + to_string(salt) << endl;

	file.close();

	//TODO: maybe add some stuff about passwords
	//shadow.txt
	file.open("shadow.txt", ofstream::app | ofstream::out);

	psh = generateMD5(password, to_string(salt));

	file << user + ":" + psh + ":" + clearance << endl;

	file.close();

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
	file << user + ":" + "x" + ":" + to_string(1000 + userCounter) /*+ ":" + groupID +*/ + ":" + comment << endl;/*+ ":" + homeDirectory +*/

	file.close();
}

int logIn()
{
	string user = getUsername();
	string salt = getSalt(user);
	if (userExists(user) && salt != "")
	{
		string password = getPassword();
		if (Captcha())
		{
			cout << "\nBob found in salt.txt and passwd.txt" << endl;
			cout << "salt retrieved: " << salt << endl;
			cout << "hashing" << endl;
			string md5Check = generateMD5(password, salt);
			cout << "Hash value: " << md5Check << endl;

			if (checkShadow(md5Check)) //if passwords match
			{
				cout << "Authentication for " << user << " is complete." << endl;
				cout << "The clearance level for " << user << " is " << getUserClearance(user) << endl;
				userClearance = getUserClearance(user);
				return 1;
			}
			else
			{
				cout << "You have entered the wrong password.\nThe program will now terminate." << endl;
				return 0;
			}
		}
		else //if captcha does not match
		{
			cout << "You have entered the wrong captcha too many times.\nThe program will now terminate.";
		}
	}
	else
	{
		cout << "User " << user << " does not exist." << endl;
		return 0;
	}
}

//TODO: validation
string getPassword()
{
	string password;
	showInput(false);

	cout << "Password: ";
	getline(cin, password);

	showInput(true);
	return password;
}

string generateMD5(string password, string salt)
{
	MD5 md5;
	string passSalt = password + salt;
	char *passSaltHash;

	passSaltHash = new char[passSalt.length() + 1];
	for (int i = 0; i < passSalt.length(); i++)
	{
		*(passSaltHash + i) = passSalt[i];
	}
	*(passSaltHash + (passSalt.length())) = '\0';
	string psh(md5.digestString(passSaltHash));

	delete[] passSaltHash;
	return psh;
}

string getSalt(string username)
{
	ifstream file;
	int found;
	string line, salt = "";
	file.open("salt.txt", ifstream::in);

	if (file.good())
	{
		while (getline(file, line))
		{
			if (line.substr(0, line.find(':')) == username)
			{
				found = line.find(':');

				salt = line.substr(found + 1);
			}
		}
	}

	file.close();

	return salt;
}

bool checkShadow(string md5)
{
	ifstream file;
	string line;
	int found;
	string check;
	file.open("shadow.txt", ifstream::in);

	if (file.good())
	{
		while (!file.eof())
		{
			for (int i = 0; i < 3; i++)
			{
				getline(file, line, ':');
				if (line == md5)
					return true;
			}
		}
	}

	file.close();
	return false;
}

string getUserClearance(string user)
{
	ifstream file;
	string line;
	file.open("shadow.txt", ifstream::in);

	if (file.good())
	{
		while (!file.eof())
		{
			getline(file, line, ':'); //user
			if (line == user)
			{
				for (int i = 0; i < 2; i++)
				{
					getline(file, line, ':');
					if (i == 1)
					{
						return line; //user clearance
					}
				}
			}

		}
	}

	file.close();
	return "";
}

string getFileClearance(string filename)
{
	ifstream file;
	string line;
	file.open("file.Store", ifstream::in);

	if (file.good())
	{
		while (!file.eof())
		{
			getline(file, line, ':'); //user
			if (line == filename)
			{
				getline(file, line, '\n'); //clearance
				return line;
			}
		}
	}

	file.close();
	return "";
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
	int salt = 0;

	for (int i = 0; i < 7; i++)
	{
		salt += ((rand() % 10) * pow(10, i));
	}

	salt += (rand() % 10 + 1) * pow(10, 8);

	return salt;
}

string appendTxt(string filename)
{
	if (filename.length() >= 4)
	{
		if (filename.substr(filename.length() - 4, 4) != ".txt")
			filename += ".txt";
	}
	else
		filename += ".txt";

	return filename;
}

//utility functions that converts string to and returns int
int convertToInt(string input)
{
	int data;
	stringstream convert(input);

	convert >> data;

	return data;
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
	else if (type == "mainmenu")
	{
		pattern = "C|W|R|L|S|E";

		if (input.length() > 1 || !(regex_match(input, regex(pattern))))
		{
			cout << "You have entered an invalid option.\nPlease reenter your option: ";
			getline(cin, input);
		}
	}
	else if (type == "choice")
	{
		pattern = "(y)|(n)";
		while (!(regex_match(input, regex(pattern))))
		{
			cout << "\nYou have entered a incorrect choice.\nPlease reenter (y/n): ";
			getline(cin, input);
		}
	}
	else if (type == "number")
	{
		pattern = "[0-9]+ | -1";
		while (!(regex_match(input, regex(pattern))))
		{
			cout << "\nYou have entered an invalid line number.\nPlease reenter (y/n): ";
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

	if (file.good())
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

bool Captcha()
{
	char* charList = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int randNum;
	string captcha = "";
	string input;

	for (int i = 0; i < 6; i++)
	{
		randNum = (rand() % 62);

		captcha += charList[randNum];
	}

	cout << "Please enter the captcha below." << endl;
	cout << "Captcha: " << captcha << endl;
	cout << "Enter the captcha: ";
	getline(cin, input);

	for (int i = 0; i < 2; i++)
	{
		if (input != captcha)
		{
			cout << "\nCapthca not matched with user input." << endl;

			captcha = ""; //reset captcha

			for (int i = 0; i < 6; i++)
			{
				randNum = (rand() % 62);

				captcha += charList[randNum];
			}
			cout << "\nPlease enter the captcha below." << endl;
			cout << "Captcha: " << captcha << endl;
			cout << "Please reenter the captcha: ";
			getline(cin, input);
		}
		else //if captcha matches
			return true;
	}
	
	return false;
}

bool fileExists(string filename)
{
	string line;

	//create file.Store if it doesn't exist
	ofstream fileCheck;
	fileCheck.open("file.Store", ofstream::out | ofstream::app);

	fileCheck.close();

	//check if filename exists in file.Store
	ifstream file;
	file.open("file.Store", ifstream::in);

	if (file.good())
	{
		while (getline(file, line, ':'))
		{
			if (line == filename) //if file is found
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