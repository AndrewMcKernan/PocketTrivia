//using namespace std;
#include "allegro.h"
//#include <string>

//create some colors
#define WHITE makecol(255,255,255)
#define BLACK makecol(0,0,0)
#define RED makecol(255,0,0)
#define GREEN makecol(0,255,0)
#define BLUE makecol(0,0,255)
#define SMOKE makecol(140,130,120)
#define CORRECT 1
#define INCORRECT 0
#define N_A -1

int numberOfCorrectAnswers = 0;
int numberOfIncorrectAnswers = 0;
int mousex, mousey, leftClick;
int currentKey, scancode, ascii;
int startScreenMode = 1;
int mainMenuMode = 0;
int selectChapterMode = 0;
int selectUnitMode = 0;
int selectNumberOfQuestionsMode = 0;
int gameMode = 0;
int inputIndex;
char input[10];
char filename[14];
char *textLine;
PACKFILE *currentFile;

typedef struct BUTTON
{
	char *buttonText;
	int isCorrectAnswer;
	int x1, x2, y1, y2; // area on screen where clicking will select this answer
}BUTTON;

BITMAP *buffer;
BUTTON buttons[20];

void printScore() {
	textprintf_right_ex(buffer, font, SCREEN_W - 5, 1, WHITE, 0,
		"Correct: %d Incorrect: %d ", numberOfCorrectAnswers, numberOfIncorrectAnswers);
}
void correctAnswer() {
	numberOfCorrectAnswers++;
	printScore();
}
void incorrectAnswer() {
	numberOfIncorrectAnswers++;
	printScore();
}

typedef struct QUESTION
{
	char *questionText;
	BUTTON answers[4];
}QUESTION;

QUESTION questions[220];
int questionIndex = 0;

BUTTON drawNewButton(int x1, int x2, int y1, int y2, int colour, char *buttonMessage, int isCorrect) {
	rectfill(buffer, x1, y1, x2, y2, colour);
	//write text next to button
	textout_ex(buffer, font, buttonMessage, (x2 + 5), (y1 + y2)/2, WHITE, 0);
	BUTTON button;
	button.buttonText = buttonMessage;
	button.x1 = x1;
	button.x2 = x2;
	button.y1 = y1;
	button.y2 = y2;
	button.isCorrectAnswer = isCorrect;
	return button;
}

BUTTON drawQuestion(int x1, int x2, int y1, int y2, int colour, BUTTON button) {
	rectfill(buffer, x1, y1, x2, y2, colour);
	//write text next to button
	textout_ex(buffer, font, button.buttonText, (x2 + 5), (y1 + y2) / 2, WHITE, 0);
	button.x1 = x1;
	button.x2 = x2;
	button.y1 = y1;
	button.y2 = y2;
	return button;
}

int isButtonSelected(BUTTON button) {
	if (leftClick && mousex > button.x1 && mousex < button.x2 && mousey > button.y1 && mousey < button.y2) {
		return 1;
	}
	return 0;
}

void updateGameInfo() {
	textout_ex(buffer, font, "Pocket Trivia (ESC to quit)", 0, 1, WHITE, 0);
	printScore();
	rect(buffer, 0, 12, SCREEN_W - 2, SCREEN_H - 2, RED);
}

void clearScreen() {
	rectfill(buffer, 0, 0, 640, 480, BLACK);
}

void getQuestionsFromChapterFile() {
	//load chapter file
	currentFile = pack_fopen(filename, 'r');
	while (!pack_feof(currentFile)) {
		//get question text
		pack_fgets(textLine, 500, currentFile);
		questions[questionIndex].questionText = textLine;
		//get answers to question
		for (int index = 0; index < 4; index++) {
			pack_fgets(textLine, 500, currentFile);
			// ~ is the marker for correct answer, so check if it's there
			if (ustrstr(textLine, '~') != NULL) {
				uremove(textLine, -2); // trim correct marker off of the end
				questions[questionIndex].answers[index].buttonText = textLine;
				questions[questionIndex].answers[index].isCorrectAnswer = CORRECT;
			}
			else {
				questions[questionIndex].answers[index].buttonText = textLine;
				questions[questionIndex].answers[index].isCorrectAnswer = INCORRECT;
			}
		}
		questionIndex++;
	}
}

void getInputFromKeyboard() {
	currentKey = readkey();
	scancode = (currentKey >> 8);
	ascii = scancode_to_ascii(scancode);
	if (ascii > 47 && ascii < 58 && inputIndex < 10) {
		input[inputIndex] = (char)ascii;
		inputIndex++;
	}
	else if (scancode == 63 || scancode == 77) {
		if (inputIndex > 0) {
			inputIndex--;
			input[inputIndex] = 0;
		}
	}
}

int main() 
{
	allegro_init();
	set_color_depth(16);
	set_gfx_mode(GFX_AUTODETECT_WINDOWED, 640, 480, 0, 0);
	install_keyboard();
	install_mouse();
	install_timer();
	srand(time(NULL));
	buffer = create_bitmap(640, 480);
	set_mouse_sprite_focus(15, 15);
	show_mouse(screen);
	while (!key[KEY_ESC]) {
		clearScreen();
		updateGameInfo();
		mousex = mouse_x;
		mousey = mouse_y;
		leftClick = (mouse_b & 1);
		//if in start screen mode, do start screen behavior
		if (startScreenMode == 1) {
			//print title screen
			textprintf_justify(buffer, font, SCREEN_W / 2 - 200, SCREEN_W / 2 + 200, SCREEN_H / 2 - 100, 400, RED, "Pocket Trivia!");
			//textprintf_centre_ex(buffer, font, "This is a quiz game designed to help you study the textbook used in COMP 369. You will be able to quiz yourself based on Chapters, Units, or just the entire thing. Enter the number of questions you want, and watch as your score changes with each answered question. Try to get them all correct!", SCREEN_W / 2, SCREEN_H / 2, RED, BLACK);
			textout_centre_ex(buffer, font, "Press ENTER to begin", SCREEN_W / 2,SCREEN_H/2 + 100,RED,WHITE);
			if (key[KEY_ENTER]) {
				startScreenMode = 0;
				mainMenuMode = 1;
				clearScreen();
			}
		}

		//else if in main menu mode, do main menu behavior
		else if (mainMenuMode == 1) {
			//draw button for units
			buttons[0] = drawNewButton(20, 120, 50, 150, GREEN, "Quiz By Units", N_A);
			//draw button for chapters
			buttons[1] = drawNewButton(20, 120, 170, 270, GREEN, "Quiz By Chapters", N_A);
			//draw button for everything
			buttons[2] = drawNewButton(20, 120, 290, 390, GREEN, "Quiz By All", N_A);
			//if units, switch to select units mode
			if (isButtonSelected(buttons[0])) {
				mainMenuMode = 0;
				selectUnitMode = 1;
				clearScreen();
			}
			//if chapters, switch to select chapter mode
			if (isButtonSelected(buttons[1])) {
				mainMenuMode = 0;
				selectChapterMode = 1;
				clearScreen();
			}
			//if everything, switch to enter number of questions mode
			if (isButtonSelected(buttons[2])) {
				mainMenuMode = 0;
				selectNumberOfQuestionsMode = 1;
				clearScreen();
			}
		}

		else if (selectChapterMode == 1) {
			//get input for desired 
			if (keypressed()) {
				getInputFromKeyboard();
				if (scancode == 67) {
					ustrzcpy(filename, 40, input);
					ustrzcat(filename, 16, ".txt");
					if (exists(filename)) {
						//getQuestionsFromChapterFile();
						selectChapterMode = 0;
						selectNumberOfQuestionsMode = 1;
					}
					else {
						textout_centre_ex(buffer, font, "Invalid chapter. Please enter a chapter between 1 and 22.", SCREEN_W / 2, SCREEN_H / 2 + 100, RED, BLACK);
						textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2 + 200, RED, BLACK, "%s", filename);
					}
				}
			}
			textout_centre_ex(buffer, font, "Type in desired chapter number and press Enter to continue (1-22).", SCREEN_W / 2, SCREEN_H / 2 - 100, RED, BLACK);
			textprintf_centre_ex(buffer, font, SCREEN_W / 2, SCREEN_H / 2, RED, BLACK, "%s", input);
		}
		
		else if (selectUnitMode == 1) {

		}

		else if (selectNumberOfQuestionsMode == 1) {

		}

		//else if in game mode, do game behavior
		else if (gameMode == 1) {
			//
			//	update score text
			//	display question text from questionarray[index]
			//	draw graphic?
			//	draw buttons for answers
			//	if answer is selected
			//		if correct
			//			increment correct answer count
			//		if incorrect
			//			increment wrong answer count
			//		index++
			//		if index > length of questionarray
			//			switch to score screen
			//		else
			//			keep on going kid
		}
		blit(buffer, screen, 0, 0, 0, 0, 640, 480);
		//clearScreen();
		rest(10);
	}
	set_mouse_sprite(NULL);
	allegro_exit();
}
END_OF_MAIN()


