#include "raylib.h"
#include "stdlib.h"

#define MAX_INPUT_CHARS     9

typedef struct GameState {
    bool mouseOnText;

    size_t letterCount;
    size_t framesCounter;

    Rectangle textBox;
    char *name;
} GameState;


GameState update(GameState g) {
    //----------------------------------------------------------------------------------
    // Update
    if (CheckCollisionPointRec(GetMousePosition(), g.textBox)) g.mouseOnText = true;
    else g.mouseOnText = false;

    if (g.mouseOnText)
    {
        // Set the window's cursor to the I-Beam
        SetMouseCursor(MOUSE_CURSOR_IBEAM);

        // Get char pressed (unicode character) on the queue
        int key = GetCharPressed();

        // Check if more characters have been pressed on the same frame
        while (key > 0)
        {
            // NOTE: Only allow keys in range [32..125]
            if ((key >= 32) && (key <= 125) && (g.letterCount < MAX_INPUT_CHARS))
            {
                g.name[g.letterCount] = (char)key;
                g.name[g.letterCount+1] = '\0'; // Add null terminator at the end of the string.
                g.letterCount++;
            }

            key = GetCharPressed();  // Check next character in the queue
        }

        if (IsKeyPressed(KEY_BACKSPACE))
        {
            if (g.letterCount == 0) g.letterCount = 1;
            g.letterCount--;
            g.name[g.letterCount] = '\0';
        }
    }
    else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

    if (g.mouseOnText) g.framesCounter++;
    else g.framesCounter = 0;

    return g;
}

void draw(GameState g) {
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(RAYWHITE);

    DrawText("PLACE MOUSE OVER INPUT BOX!", 240, 140, 20, GRAY);

    DrawRectangleRec(g.textBox, LIGHTGRAY);
    if (g.mouseOnText) DrawRectangleLines((int)g.textBox.x, (int)g.textBox.y, (int)g.textBox.width, (int)g.textBox.height, RED);
    else DrawRectangleLines((int)g.textBox.x, (int)g.textBox.y, (int)g.textBox.width, (int)g.textBox.height, DARKGRAY);

    DrawText(g.name, (int)g.textBox.x + 5, (int)g.textBox.y + 8, 40, MAROON);

    DrawText(TextFormat("INPUT CHARS: %i/%i", g.letterCount, MAX_INPUT_CHARS), 315, 250, 20, DARKGRAY);

    if (g.mouseOnText)
    {
        if (g.letterCount < MAX_INPUT_CHARS)
        {
            // Draw blinking underscore char
            if (((g.framesCounter/20)%2) == 0) DrawText("_", (int)g.textBox.x + 8 + MeasureText(g.name, 40), (int)g.textBox.y + 12, 40, MAROON);
        }
        else DrawText("Press BACKSPACE to delete chars...", 230, 300, 20, GRAY);
    }

    EndDrawing();
}


int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib [text] example - input box");

    char name[MAX_INPUT_CHARS + 1] = "\0";      // NOTE: One extra space required for null terminator char '\0'
    Rectangle textBox = { screenWidth/2.0f - 100, 180, 225, 50 };

    GameState gameState = {
        .name = name,
        .textBox = textBox,
        .letterCount = 0,
        .mouseOnText = false,
        .framesCounter = 0,
    };

    SetTargetFPS(10);               // Set our game to run at 10 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        //----------------------------------------------------------------------------------
        gameState = update(gameState);
        draw(gameState);
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Check if any key is pressed
// NOTE: We limit keys check to keys between 32 (KEY_SPACE) and 126
bool IsAnyKeyPressed()
{
    bool keyPressed = false;
    int key = GetKeyPressed();

    if ((key >= 32) && (key <= 126)) keyPressed = true;

    return keyPressed;
}
