#include "speech.h"

#include "globals.h"
#include "hardware.h"

/**
 * Draw the speech bubble background.
 */
static void draw_speech_bubble();

/**
 * Erase the speech bubble.
 */
static void erase_speech_bubble();

/**
 * Draw a single line of the speech bubble.
 * @param line The text to display
 * @param which If TOP, the first line; if BOTTOM, the second line.
 */
#define TOP    0
#define BOTTOM 1
static void draw_speech_line(const char* line, int which);

/**
 * Delay until it is time to scroll.
 */
static void speech_bubble_wait();

void draw_speech_bubble()
{
    uLCD.filled_rectangle(0, 94, 127, 127, WHITE);
    uLCD.rectangle(0, 94, 127, 127, BLUE);
}

void erase_speech_bubble()
{
    uLCD.filled_rectangle(0, 94, 127, 127, BLACK);
}

void draw_speech_line(const char* line, int which)
{
    uLCD.locate(1,13+which);
    uLCD.color(DGREY);
    uLCD.set_font(FONT_7X8);
    uLCD.text_mode(TRANSPARENT);
    uLCD.printf(line);
}

void speech_bubble_wait()
{
    GameInputs inputs;
    do {
        inputs = read_inputs();
        wait_ms(20);
    } while (!inputs.b4);
}

void speech(const char* line1, const char* line2)
{
    draw_speech_bubble();
    draw_speech_line(line1, TOP);
    draw_speech_line(line2, BOTTOM);
    speech_bubble_wait();
    erase_speech_bubble();
}

void long_speech(const char* lines[], int n)
{
    for (int i = 0; i < n; i += 2) {
        speech(lines[i], (i+1 == n ? "" : lines[i+1]));
    }
}
