#include <SDL.h>
#include <SDL_ttf.h>
#include <emscripten.h>
#include <cstdlib>
#include <ctime>

#define WIDTH 800
#define HEIGHT 600
#define PADDLE_WIDTH 10
#define PADDLE_HEIGHT 80
#define BALL_SIZE 15
#define PADDLE_SPEED 4
#define BALL_SPEED 3

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static int ballDirectionX;
static int ballDirectionY;
static SDL_Rect playerPaddle = { .x = 10, .y = HEIGHT / 2 - PADDLE_HEIGHT / 2, .w = PADDLE_WIDTH, .h = PADDLE_HEIGHT };
static SDL_Rect opponentPaddle = { .x = WIDTH - 20, .y = HEIGHT / 2 - PADDLE_HEIGHT / 2, .w = PADDLE_WIDTH, .h = PADDLE_HEIGHT };
static SDL_Rect ball = { .x = WIDTH / 2, .y = HEIGHT / 2, .w = BALL_SIZE, .h = BALL_SIZE };
static TTF_Font* font = nullptr;
static SDL_Color white = {255, 255, 255, 255};
static SDL_Color gray = {128, 128, 128, 255};

bool player1Moved = false;
bool player2Moved = false;

// Game state
enum GameState {
    MENU,
    PVP,
    PVAI
};

GameState currentGameState = MENU;

SDL_Rect pvPButton = { .x = WIDTH / 2 - 100, .y = HEIGHT / 2, .w = 200, .h = 50 };
SDL_Rect pvAIButton = { .x = WIDTH / 2 - 100, .y = HEIGHT / 2 + 60, .w = 200, .h = 50 };

bool isMouseOverButton(SDL_Rect* button) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    SDL_Point point = { x, y };
    return SDL_PointInRect(&point, button);
}

void renderText(const char* message, SDL_Color color, int x, int y) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, message, color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    // Center the text on the given x, y coordinates
    SDL_Rect textRect = {
        .x = x - textSurface->w / 2,
        .y = y - textSurface->h / 2,
        .w = textSurface->w,
        .h = textSurface->h
    };
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void RenderMenu()
{
    // Render the title
    renderText("Jack's Pong Game", white, WIDTH / 2, HEIGHT / 4 + 20);  // Assuming you want the title to be "PONG"

    // Draw background for the PvAI button
    SDL_SetRenderDrawColor(renderer, 76, 76, 76, 255);
    if (isMouseOverButton(&pvAIButton)) {
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255); // Hover color
    }
    SDL_RenderFillRect(renderer, &pvAIButton);

    // Render the PvAI button text
    renderText("Player vs AI", white, pvAIButton.x + pvAIButton.w/2, pvAIButton.y + pvAIButton.h/2);

    // Draw background for the PvP button
    SDL_SetRenderDrawColor(renderer, 76, 76, 76, 255);
    if (isMouseOverButton(&pvPButton)) {
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255); // Hover color
    }
    SDL_RenderFillRect(renderer, &pvPButton);

    // Render the PvP button text
    renderText("Player vs Player", white, pvPButton.x + pvPButton.w/2, pvPButton.y + pvPButton.h/2);
}


void ResetBall() {
    ball.x = WIDTH / 2;
    ball.y = HEIGHT / 2;
    ballDirectionX *= -1;  // Reverse direction after a point is scored.
}

void Render()
{
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (currentGameState == MENU) {
        RenderMenu();
    } else {
        // Draw player paddle
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &playerPaddle);

        // Draw opponent paddle
        SDL_RenderFillRect(renderer, &opponentPaddle);

        // Draw ball
        SDL_RenderFillRect(renderer, &ball);
    }

    // Update screen
    SDL_RenderPresent(renderer);
}

void Update()
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            emscripten_cancel_main_loop();
            return;
        }
        
        if (currentGameState == MENU && e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            SDL_Point point = { x, y };  // Create SDL_Point struct here
            if (SDL_PointInRect(&point, &pvPButton)) {  // Use the created struct
                currentGameState = PVP;
            } else if (SDL_PointInRect(&point, &pvAIButton)) {  // Use the created struct
                currentGameState = PVAI;
            }
        }
    }

    if (currentGameState != MENU) {
        const Uint8* keys = SDL_GetKeyboardState(NULL);

        // Player controls
        if (keys[SDL_SCANCODE_UP] && playerPaddle.y > 0) {
            playerPaddle.y -= PADDLE_SPEED;
            player1Moved = true;
        }
        if (keys[SDL_SCANCODE_DOWN] && playerPaddle.y < HEIGHT - PADDLE_HEIGHT) {
            playerPaddle.y += PADDLE_SPEED;
            player1Moved = true;
        }
        if (keys[SDL_SCANCODE_W] && opponentPaddle.y > 0) {
            opponentPaddle.y -= PADDLE_SPEED;
            player2Moved = true;
        }
        if (keys[SDL_SCANCODE_S] && opponentPaddle.y < HEIGHT - PADDLE_HEIGHT) {
            opponentPaddle.y += PADDLE_SPEED;
            player2Moved = true;
        }

        // Opponent controls
        if (currentGameState == PVP) {
            if (keys[SDL_SCANCODE_W] && opponentPaddle.y > 0) {
                opponentPaddle.y -= PADDLE_SPEED;
            }
            if (keys[SDL_SCANCODE_S] && opponentPaddle.y < HEIGHT - PADDLE_HEIGHT) {
                opponentPaddle.y += PADDLE_SPEED;
            }
        } else if (currentGameState == PVAI) {
            // Opponent AI - it will simply follow the ball's vertical position.
            if (opponentPaddle.y + PADDLE_HEIGHT / 2 < ball.y) {
                opponentPaddle.y += PADDLE_SPEED;
            } else if (opponentPaddle.y + PADDLE_HEIGHT / 2 > ball.y) {
                opponentPaddle.y -= PADDLE_SPEED;
            }
        }

        if ((currentGameState == PVP && player1Moved && player2Moved) || (currentGameState == PVAI && player1Moved)) {
            // Ball movement
            ball.x += ballDirectionX * BALL_SPEED;
            ball.y += ballDirectionY * BALL_SPEED;

            // Ball collision with top and bottom
            if (ball.y <= 0 || ball.y + BALL_SIZE >= HEIGHT) {
                ballDirectionY = -ballDirectionY;
            }

            // Ball collision with paddles
            if ((ball.x <= playerPaddle.x + PADDLE_WIDTH && ball.y + BALL_SIZE >= playerPaddle.y && ball.y <= playerPaddle.y + PADDLE_HEIGHT) 
            || (ball.x + BALL_SIZE >= opponentPaddle.x && ball.y + BALL_SIZE >= opponentPaddle.y && ball.y <= opponentPaddle.y + PADDLE_HEIGHT)) {
                ballDirectionX = -ballDirectionX;
            }

            // Check for points
            if (ball.x <= 0) {
                // Opponent scores a point.
                ResetBall();
            } else if (ball.x + BALL_SIZE >= WIDTH) {
                // Player scores a point.
                ResetBall();
            }
        }
    }
}

void mainloop() {
    Update();
    Render();
}

int main()
{
    // Random initial directions for the ball.
    srand((unsigned int)time(NULL));
    ballDirectionX = (rand() % 2 ? 1 : -1);
    ballDirectionY = (rand() % 2 ? 1 : -1);

    // Initialise SDL
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Jack's Pong Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);

    TTF_Init();
    font = TTF_OpenFont("assets/Inconsolata.ttf", 24);  // 24 is the font size
    if (font == nullptr) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }


    emscripten_set_main_loop(mainloop, 0, 1);

    // Before calling SDL_Quit() at the end of your main function, add:
    TTF_CloseFont(font);
    TTF_Quit();

    return 0;
}