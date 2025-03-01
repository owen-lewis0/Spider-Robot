#include <iostream>
#include "Spider.cpp"

using namespace std;

int main(int argc, char *argv[])
{
    Spider Spider;

	cout << "Spider Init" << endl;
	Spider.Init(); //Moves all legs to retracted position
	
	cout << "Spider Standup" << endl;	
	Spider.Standup(); //Changes knee angle so spider is on its feet

	cout << "Waiting for Command..." << endl;
	bool done = false;
	while (!done) //loops until user tells it to stop
	{
		char cmd_chr;
		cout << "Enter Next Command: ";
		cin >> cmd_chr;
		
		switch (cmd_chr)
		{
			case 'f': //Case for moving forwards
				cout << "CMD_FORDWARD" << endl;
				Spider.MoveForward();
				break;
			case 'b': //Case for moving backwards
				cout << "CMD_BACKWARD" << endl;
				Spider.MoveBackward();
				break;
			case 'l': //Case for left turn
				cout << "CMD_LEFT" << endl;
				Spider.Left();
				break;
			case 'r': //Case for right turn
				cout << "CMD_RIGHT" << endl;
				Spider.Right();
				break;
			case 's': //Ends the program
				cout << "CMD_STOP" << endl;
				done = true;
				break;
			default: //For unknown inputs
				cout << "IDLE or UNKNOWN COMMAND" << endl;
				break;
		}
	}

	return 0;
}
