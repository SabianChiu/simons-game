/*
Author: Sabian Chiu
Student ID: 99656258
Email: sabian-chiu@alumni.ubc.ca
Date: 27 November 2017
Class: APSC160
Lab Section: L1J

Purpose: Program is a Simon Game that randomly generates a series of LED projections,	
	where a sequence of colors are shown to the player. The player should memorize those
	and reproduce the same sequence by pushing the corresponding switches. The sequence 
	begins with a single color, and successful inputs from the player will add another 
	randomly chosen color to the end of the sequence and this new sequence is displayed,
	for the player to once again memorize and input. This repeats and the game continues 
	until the player makes a mistake, or if the player ends when the pattern reaches the
	pre-set maximum length of the sequence. The program will start a new game right after.
*/


#define _CRT_SECURE_NO_WARNINGS


#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <DAQlib.h>
#include <time.h>

#define ON 1
#define OFF 0

#define TRUE 1
#define FALSE 0

#define GREENLED 0
#define REDLED 1
#define YELLOWLED 2
#define BLUELED 3

#define GREENBUTTON 0
#define REDBUTTON 1
#define YELLOWBUTTON 2
#define BLUEBUTTON 3

#define ONE_SECOND 1000
#define FLASH_TIME 200


#define SEQUENCE_SIZE 5 /*Edit SEQUENCE_SIZE to change maximum level of game*/
#define NUM_LEDs 4
#define NUM_FLASHES 3


void runSimon(void);
void generateSequence(int sequence[]);
void displayLED(int array);
void buttonInput(int sequence[], int input[], int N);
int compareInput(int sequence[], int input[], int N);
void displaySequence(int sequence[], int N);
void flashLEDs(void);
void flash_GR_LEDs(int winLose);

/*Main function runs DAQ module upon successful user entry of setup configuration and initializes random number 
generator, then runs Simon Game function. If user entry of setup configuration is unsuccessful, function will print an error message*/
int main(void) {
	int setupNum;
	int count = 0;

	printf("Enter setup configuration (6 to initialize simulator):\n");
	scanf("%d", &setupNum);

	if (setupDAQ(setupNum) == TRUE) {
		
		/*Generates 4 random values and assigns them
		to an array*/
		srand((unsigned)time(NULL));

		runSimon();
	}

	else
		printf("Error: Cannot Initialize.\n");

	system("PAUSE");
	return 0;
}

/* runSimon function begins by explaining the rules to the game. Function prompts user to enter '1' to initialize the game. 
Once game is initialized, function will call upon displaySequence to display a random series of colors on LED displays by
assigning random LED values to a sequence array.
It will call upon flashLEDs to notify the user that the program is waiting for user input. buttonInput will read user 
input in regards to push buttons on DAQ module, and assign user input to an input array. compareInput will compare the
sequence array with the user array, and return TRUE if they match, and FALSE otherwise. If TRUE is returned, this function 
will call upon flash_GR_LEDs to flash the green LED three times, and increment the level of the game until the level has
reached the maximum level, at which point the user has won the game and is given the option to play a new game and restart
the loop, or exit the program. If FALSE is returned, the user has lost the game, at which point the option will be given to
the user to start a new game or to exit the program. */
void runSimon(void) {
	int N = 1;
	int sequence[SEQUENCE_SIZE];
	int input[SEQUENCE_SIZE] = { -1, -1, -1, -1 };
	int n;
	int beginVal;
	int RESTART = TRUE;
	int winLose;

	/*Explain rules*/
	printf("Welcome to the SIMON Game.\n\n");
	printf("A sequence of colors will flash on the LEDs.\n");
	printf("Your objective is to memorize the sequence and then reproduce the sequence once the sequence ends.\n");
	printf("Rules:\n");
	printf("	1. You can enter your sequence once the LEDs all flash once, signifying that the sequence display is over.\n");
	printf("	2. If you enter the sequence correctly, another randomly chosen color is added to the end of the sequence,");
	printf("			and this new sequence is displayed. It gets longer and more difficult each time!\n");
	printf("	3. If you are able to achieve the maximum sequence length, the green LED will blink three times,\n");
	printf("		and you have won!\n");
	printf("	4. If you make a mistake, the red LED will blink three times,\n");
	printf("		and you have lost!\n");
	printf("	5. Enter '1' to start the game.\n");
	scanf("%d", &beginVal);

	if (beginVal == TRUE) {
		
		Sleep(ONE_SECOND);
		generateSequence(sequence);

		while (continueSuperLoop() == TRUE && RESTART == TRUE) {

			/*Displays sequence on LEDs*/
			displaySequence(sequence, N);

			/*Flashes LEDs to notify user to begin*/
			flashLEDs();

			/*Read user input and assign to input array until enough values are added to array*/
			buttonInput(sequence, input, N);

			/*Compares input array with sequence array to determine if user passes round
			or loses round*/
			winLose = compareInput(sequence, input, N);

			if (winLose == TRUE) {

				/*If size of input array matches sequence length, user wins game (game ends)*/
				if (N == SEQUENCE_SIZE) {
					printf("Congratulations! You have won.\n");
					flash_GR_LEDs(winLose);
					printf("Would you like to play another round? Enter '1' to continue, and any other key to quit.\n");
					scanf("%d", &RESTART);

					if (RESTART != TRUE) {
						printf("Thank you for playing!\n");
						break;
					}

					else {
						N = 0;
						generateSequence(sequence);
					}
				}


				/*If size of input array does not match sequence length, game advances to next level*/
				else if (N != SEQUENCE_SIZE) {
					N++;
					Sleep(ONE_SECOND);
				}

			}

			/*Flash Red LED 3 times (LED1), restart game*/
			else {
				flash_GR_LEDs(winLose);
				printf("You have pressed the wrong button, you have lost!\n");
				printf("Would you like to play another round? Enter '1' to continue, and any other key to quit.\n");
				scanf("%d", &RESTART);

				if (RESTART != TRUE) {
					printf("Thank you for playing!\n");
					break;
				}

				else {
					N = 0;
					generateSequence(sequence);
				}
			}

			


		}
	}
}


/*Generates a random sequence*/
void generateSequence(int sequence[]) {
	int n;

	for (n = 0; n < SEQUENCE_SIZE; n++) {
		sequence[n] = rand() % NUM_LEDs;
	}
}

/*Read user input and assign to input array until enough values are added to array*/
void buttonInput(int sequence[], int input[], int N) {
	int n;
	int green_button, red_button, yellow_button, blue_button;

	for (n = 0; n < N; n++) {

		/*Waits until user presses button*/
		do {
			green_button = digitalRead(GREENBUTTON);
			red_button = digitalRead(REDBUTTON);
			yellow_button = digitalRead(YELLOWBUTTON);
			blue_button = digitalRead(BLUEBUTTON);
			if (continueSuperLoop() == FALSE)
				break;
		} while (green_button == OFF && red_button == OFF && yellow_button == OFF && blue_button == OFF);

		if (continueSuperLoop() == FALSE)
			break;

		/*Reads user input and assigns user button click to array*/
		if (green_button == ON)
			input[n] = 0;

		else if (red_button == ON)
			input[n] = 1;

		else if (yellow_button == ON)
			input[n] = 2;

		else if (blue_button == ON)
			input[n] = 3;

		/*Waits until user releases button*/
		do {
			green_button = digitalRead(GREENBUTTON);
			red_button = digitalRead(REDBUTTON);
			yellow_button = digitalRead(YELLOWBUTTON);
			blue_button = digitalRead(BLUEBUTTON);
			if (continueSuperLoop() == FALSE)
				break;
		} while (green_button != OFF || red_button != OFF || yellow_button != OFF || blue_button != OFF);
	}
}


/*Compares the value of input array and sequence array, function returns TRUE if arrays match,
otherwise returns FALSE if arrays do not match*/
int compareInput(int sequence[], int input[], int N) {
	int n;
	int count = 0;

	for (n = 0; n < N; n++) {
		
		/*If input matches sequence, add one to count*/
		if (input[n] == sequence[n])
			count++;
		
		/*If input doesn't match sequence, do nothing*/
	}
	
	/*If count matches round number N, then user completes around and function returns TRUE. Otherwise,
	return FALSE.*/
	if (count == N)
		return TRUE;

	else
		return FALSE;
}

/*Displays sequence on LED display*/
void displaySequence(int sequence[], int N) {
	int n;
	for (n = 0; n < N; n++) {
		digitalWrite(sequence[n], ON);
		Sleep(ONE_SECOND / 1.5);
		digitalWrite(sequence[n], OFF);
		Sleep(ONE_SECOND / 2);
	}
}

/*Flashes LEDs in quick succession to notify user that they are able to input*/
void flashLEDs(void) {
	int n;
	
	for (n = 0; n < NUM_LEDs; n++) {
		digitalWrite(n, ON);
	}

	Sleep(FLASH_TIME);
	
	for (n = 0; n < NUM_LEDs; n++){
		digitalWrite(n, OFF);
	}
}

void flash_GR_LEDs(int winLose){
	int n, pos;

	if (winLose == TRUE)
		pos = GREENLED;

	else
		pos = REDLED;

	for (n = 0; n < NUM_FLASHES; n++) {
		digitalWrite(pos, ON);
		Sleep(ONE_SECOND / 6);
		digitalWrite(pos, OFF);
		Sleep(ONE_SECOND / 6);
	}


}