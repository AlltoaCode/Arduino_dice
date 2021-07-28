#include "funshield.h"

constexpr int buttonPins[] { button1_pin, button2_pin, button3_pin };
constexpr int buttonPinsCount = sizeof(buttonPins) / sizeof(buttonPins[0]);

class Display
{
	public: 

	void DisplayMultiplexMessage()
	{
		/*
		* Displays a 4 character message, taken from the class variable `message`. Needs to be called in loop() as it only shows one character at a time
		*/

		this->ChangeDisplaySegment();
		this->ChangeDisplayLetter(this->GetCurrentDisplay(), this->message[this->GetCurrentDisplay()]);
	}

	void ChangeMessage(char newMessage[])
	{
		/*
		* Method that sets newMessage to the message variable, to be displayed.
		*/

		for(int i = 0; i < this->numberOfSegments; ++i)
		{
			this->message[this->numberOfSegments -1 - i] = newMessage[i]; 
		}

	}


	private:
	//--- Config Vars ---

	int numberOfSegments = 4;
	uint8_t positions[4] = {8, 4, 2 , 1};
	char alphabet[37] = {' ','A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
	uint8_t alphabetLength = 37;
        byte glyphMap[37] {
		0b11111111,   // OFF
                0b10001000,   // A
                0b10000011,   // b
                0b11000110,   // C
                0b10100001,   // d
                0b10000110,   // E
                0b10001110,   // F
                0b10000010,   // G
                0b10001001,   // H
                0b11111001,   // I
                0b11100001,   // J
                0b10000101,   // K
                0b11000111,   // L
                0b11001000,   // M
                0b10101011,   // n
                0b10100011,   // o
                0b10001100,   // P
                0b10011000,   // q
                0b10101111,   // r
                0b10010010,   // S
                0b10000111,   // t
                0b11000001,   // U
                0b11100011,   // v
                0b10000001,   // W
                0b10110110,   // ksi
                0b10010001,   // Y
                0b10100100,   // Z
                0b11111001,   // 1
                0b10100100,   // 2
                0b10110000,   // 3
                0b10011001,   // 4
                0b10010010,   // 5
                0b10000010,   // 6
                0b11111000,   // 7
                0b10000000,   // 8
                0b10010000,   // 9
		0b11000000    // 0
	};

	//--- Support Vars ---

	uint8_t currentDisplay = 0;
	char message[4];

	// --- Methods ---

	void ChangeDisplaySegment(int value = 1)
	{
		/*
		* This method switched which segment is on, as to achieve multiplexing. If not supplied with arg, it switches by one.
		*/

		this->currentDisplay += value;
		this->currentDisplay = this->currentDisplay % this->numberOfSegments;
	}

	uint8_t GetCurrentDisplay()
        {
		/*
		* Methot that returns a segment, that is currently on.
		*/

                return this->currentDisplay;
        }

	void ChangeDisplayLetter(uint8_t displayPosition, char letter)
        {
		/*
		* Method that finds the position of the char in the alphabet var (with FindAlphabetPosition) and calls WriteGlyph() to write the char to the specified DisplayPosition.
		*/

                        int position = this->FindAlphabetPosition(letter);
                        this->WriteGlyph(this->glyphMap[position], this->positions[displayPosition]);

        }

	int FindAlphabetPosition(char character)
        {
		/*
		* Finds position of char in the alphabet array, if not found returns -1
		*/

                character = toupper(character);
                for(int i = 0; i < alphabetLength; i++)
                {
                        if(this->alphabet[i] == character) return i;
                }
                return -1;
        }

	void WriteGlyph(byte glyph, byte position)
        {
		/*
		* Method for "hardware" writing glyphs to the arduino display.
		*/

                digitalWrite(latch_pin, LOW); // zavřít -začátek zápisu
                shiftOut(data_pin, clock_pin, MSBFIRST, glyph); // pošli glyph
                shiftOut(data_pin, clock_pin, MSBFIRST, position);// pošli pozice znaků
                digitalWrite(latch_pin, HIGH);// otevřít -konec zápisu
        }

};

class Button
{
	public:
        Button(int pin)
        {
                this->pin = pin;
        }

        void Setup()
	{
		/*
		* Method to be called in the setup() arduino function.
		*/
        
                pinMode(this->pin, INPUT);
        }

        bool Read()
	{
		/*
		* Method that returs T/F upon reading the state of the Button.
		*/
        
                int state = digitalRead(this->pin);
                if(!state) return true;
                return false;
        }

        private:
        int pin;	 // the Buttons' pin
};

class Timer
{
        public:
        bool IsTime(unsigned long timeInterval)
        {
	/* 
	* Check the difference between lastTime and currentTime to determine if the given timeInterval has already passed
	*/
                unsigned long currentTime = millis();

                if(currentTime - this->previousTime  >= timeInterval)
                {
                        this->previousTime = currentTime;
                        return true;
                }
                return false;
        }

        private:
        unsigned int currentTime;
        unsigned long previousTime;

};

// ##### Object creation #####

Display MyDisplay;
Button Button1(buttonPins[0]);
Button Button2(buttonPins[1]);
Button Button3(buttonPins[2]);
Timer AnimationTimer;
Timer IncrementTimer;

// ##### Logic #####

// --- Support vars ---
char message[] = {'1', 'd', '0', '4'};
unsigned long firstPushTime = 0;
bool firstPushButton1 = 0;

// --- Configuration vars ---
constexpr unsigned int numberOfDigits = 4;		// Number of digits the display can hold
constexpr unsigned int base = 10;			// Number base to work with
constexpr uint8_t maxThrows = 9;			// Max number of dice throws
constexpr uint8_t maxSides = 100;			// Max number of dice sides
constexpr unsigned int numberOfSidesDigits = 2;		// Number of dice sice digits the display can hold. Base = 2, meaning last two display digits are used
constexpr uint8_t minSides = 4;				// Number of lowest possible sides of a dice
constexpr uint8_t incrementTime = 150;			// Time in ms of how often the increments happen
constexpr uint8_t animationTime = 150;			// Time in ms of how often do the animations switch

// --- Animation vars ----
bool isAnimation1 = 1;
char animation1[] = {'0','x', 'x', '0'};
char animation2[] = {'e','0', '0', '3'};

// --- Functions ---

void IncrementThrows(int times = 1) 
{
	/*
	 * Increments the number of times the dice is thrown, by reading it from the first postiton of the message char array, incrementing and writing it back.
	 */

	char currentThrows = message[0] - '0';
	int newNumber = ((currentThrows % maxThrows) + times);
	message[0] = newNumber + '0';

	MyDisplay.ChangeMessage(message);
}

void IncrementSides(int times = 1)
{
	/*
	 * Increments the number of sides the dice has, by reading the current number from the message char array, incrementing and writing it back to message.
	 */

	unsigned int currentSides = 0;
	currentSides += message[numberOfDigits - 1] - '0';
	for(unsigned int i = 1; i < numberOfSidesDigits; i++)
	{	
		currentSides += (message[numberOfDigits - 1 - i] - '0') * (i * base);
	}

	int newNumber = ((currentSides + times) % maxSides);
	if(newNumber > 0 && newNumber < minSides) newNumber = minSides;
	char array[4];
	ToArray(newNumber, array);

	message[2] = array[2];
	message[3] = array[3];

	MyDisplay.ChangeMessage(message);
	
}

void PlayAnimation()
{
	/*
	 * Plays an animation by switching between two messagen on the display.
	 */


	if(AnimationTimer.IsTime(animationTime))
	{
		if(isAnimation1)
		{
			MyDisplay.ChangeMessage(animation2);
			isAnimation1 = 0;
		}
		else
		{
			MyDisplay.ChangeMessage(animation1);
			isAnimation1 = 1;
		}
	}
	
}

int Power(int x, int n)
{
	/*
	 * Rasies x to the power of n.
	 */

	int number = 1;
	for(int i = 0; i < n; ++i)
	{
		number *= x;
	}
	return number;
}

unsigned int ThrowDice()
{
	/*
	 * Uses the random() built-in func to simulate dice throwing. Generates a number in range 1 to number of sides.
	 */

	unsigned int sides = 0;

	sides += message[numberOfDigits - 1] - '0';
	for(unsigned int i = 1; i < numberOfSidesDigits; i++)
	{	
		sides += (message[numberOfDigits - 1 - i] - '0') * (i * base);
	}

	if(sides == 0) sides = Power(base, numberOfSidesDigits);

	unsigned int randomNumber = random();
	return (randomNumber % sides) + 1;
	
}

unsigned int Generate(unsigned long seed)
{
	/*
	 * Uses the ThrowDice() func to repeatedly throw dice, then returns the sum of all the throws. 
	 * Sets the seed for the random() built-in func as the amount of time Button1 was pressed.
	 */

	randomSeed(seed);	
	unsigned int sum = 0;
	unsigned int throws = message[0] - '0';

	for(unsigned int i = 0; i < throws; ++i)
	{
		sum += ThrowDice();
	}
	return sum;
}

void Display(unsigned int number)
{
	/*
	 * Displays given number, by converting the integer to an array of chars.
	 */

	char arrayToDisplay[numberOfDigits];
	ToArray(number, arrayToDisplay);
	
	MyDisplay.ChangeMessage(arrayToDisplay);

	/* DEBUG - sends array members to Serial
	for(int i = 0; i<5; ++i)
	{
		Serial.print(arrayToDisplay[i]);
	}
	Serial.println();
	*/
}

void ToArray(unsigned int number, char (&array)[numberOfDigits])
{
	/*
	 * Converts a given integer to an array of chars.
	 */

	for(int i = numberOfDigits - 1 ; i >= 0;  --i, number/= base)
	{ 
		array[i] = (number % base) + '0';
	}
}

void CheckButton1()
{
	if(Button1.Read())
	{
		PlayAnimation();

		if(firstPushButton1 == 0)
		{
			firstPushTime = millis();
			firstPushButton1 = 1;
		}
	} 
	else
	{
		if(firstPushButton1 != 0)
		{
			Display(Generate(millis() - firstPushTime));
			firstPushTime = 0;
			firstPushButton1 = 0;
		}
	}

}

void CheckButton2()
{
	if(Button2.Read() && IncrementTimer.IsTime(incrementTime))
	{
		IncrementThrows();
	}
}

void CheckButton3()
{
	if(Button3.Read() && IncrementTimer.IsTime(incrementTime))
	{
		IncrementSides();
	}
}

// ##### Arduino funcs ######

void setup() 
{
	for (int i = 0; i < buttonPinsCount; ++i) 
	{
                pinMode(buttonPins[i], INPUT);
        }

        pinMode(latch_pin, OUTPUT);
        pinMode(clock_pin, OUTPUT);
        pinMode(data_pin, OUTPUT);

	MyDisplay.ChangeMessage(message);

	/* DEBUG - init Serial
	Serial.begin(9600);
	*/
}

void loop() 
{
	MyDisplay.DisplayMultiplexMessage();
	CheckButton1();
	CheckButton2();
	CheckButton3();
}
