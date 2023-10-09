#include <SDL.h>
#include <SDL_ttf.h>
#include <emscripten.h>
#include <cstdlib>
#include <ctime>

const int WIDTH = 800, HEIGHT = 600;
const int PADDLE_WIDTH = 10, PADDLE_HEIGHT = 80;
const int BALL_SIZE = 8, PADDLE_SPEED = 4, BALL_SPEED = 3;
const int BORDER_SIZE = 5;

static SDL_Window* window = nullptr;
static SDL_Renderer* renderer = nullptr;
static int ballDirectionX, ballDirectionY;
static SDL_Rect playerPaddle = {10, HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT};
static SDL_Rect opponentPaddle = {WIDTH - 20, HEIGHT / 2 - PADDLE_HEIGHT / 2, PADDLE_WIDTH, PADDLE_HEIGHT};
static SDL_Rect ball = {WIDTH / 2, HEIGHT / 2, BALL_SIZE, BALL_SIZE};
static TTF_Font* font = nullptr;
static SDL_Color white = {255, 255, 255, 255};

enum GameState { MENU, PVP, PVAI, PAUSED };
GameState currentGameState = MENU;

bool player1Moved = false;
bool player2Moved = false;
int playerScore = 0;
int opponentScore = 0;

SDL_Rect pvPButton = {WIDTH / 2 - 125, HEIGHT / 2, 250, 50};
SDL_Rect pvAIButton = {WIDTH / 2 - 125, HEIGHT / 2 + 60, 250, 50};

bool isMouseOverButton(SDL_Rect* button) {
    int x, y;
    SDL_GetMouseState(&x, &y);
    SDL_Point point = {x, y};
    return SDL_PointInRect(&point, button);
}

void renderText(const char* message, SDL_Color color, int x, int y) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, message, color);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x - textSurface->w / 2, y - textSurface->h / 2, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderButton(SDL_Rect& button, const char* text) {
    SDL_SetRenderDrawColor(renderer, 76, 76, 76, 255);
    if (isMouseOverButton(&button)) {
               SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255); // Hover color
    }
    SDL_RenderFillRect(renderer, &button);

    // Render button text
    renderText(text, white, button.x + button.w/2, button.y + button.h/2);
}

void RenderMenuBorder() {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set color to white
    
    int borderWidth = 5; // 5 pixel border, you can adjust this as needed

    // Draw top border
    SDL_Rect topBorder = {0, 0, WIDTH, borderWidth};
    SDL_RenderFillRect(renderer, &topBorder);

    // Draw bottom border
    SDL_Rect bottomBorder = {0, HEIGHT - borderWidth, WIDTH, borderWidth};
    SDL_RenderFillRect(renderer, &bottomBorder);

    // Draw left border
    SDL_Rect leftBorder = {0, 0, borderWidth, HEIGHT};
    SDL_RenderFillRect(renderer, &leftBorder);

    // Draw right border
    SDL_Rect rightBorder = {WIDTH - borderWidth, 0, borderWidth, HEIGHT};
    SDL_RenderFillRect(renderer, &rightBorder);
}

void RenderMenu() {
    renderText("Jack's Pong Game", white, WIDTH / 2, HEIGHT / 4 + 20);
    renderButton(pvPButton, "Player vs Player");
    renderButton(pvAIButton, "Player vs AI");
    RenderMenuBorder(); // Add the menu border rendering here
}

void RenderDottedLine() {
    SDL_SetRenderDrawColor(renderer, 240, 240, 240, 240);  // White color

    int dotHeight = 10;  // Height of each dot
    int gapHeight = 4;   // Gap between dots
    int totalHeight = dotHeight + gapHeight;
    int xPosition = WIDTH / 2;

    for (int y = 0; y < HEIGHT; y += totalHeight) {
        SDL_Rect dotRect = {xPosition, y, 2, dotHeight};  // 2 is the width of each dot
        SDL_RenderFillRect(renderer, &dotRect);
    }
}

void ResetBall() {
    ball.x = WIDTH / 2;
    ball.y = HEIGHT / 2;
    ballDirectionX *= -1;  // Reverse direction after a point is scored.
}

void Render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (currentGameState == MENU) {
        RenderMenu();
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Rendering scores
        char scoreText[100];
        sprintf(scoreText, "%d", playerScore);
        renderText(scoreText, white, WIDTH * 1/4, 30);

        sprintf(scoreText, "%d", opponentScore);
        renderText(scoreText, white, WIDTH * 3/4, 30);

        RenderDottedLine();
        SDL_RenderFillRect(renderer, &playerPaddle);
        SDL_RenderFillRect(renderer, &opponentPaddle);
        SDL_RenderFillRect(renderer, &ball);
    }
    SDL_RenderPresent(renderer);
}

void Update() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            emscripten_cancel_main_loop();
            return;
        }
        
        if (currentGameState == MENU && e.type == SDL_MOUSEBUTTONDOWN) {
            int x, y;
            SDL_GetMouseState(&x, &y);
            SDL_Point point = { x, y };
            if (SDL_PointInRect(&point, &pvPButton)) {
                currentGameState = PVP;
            } else if (SDL_PointInRect(&point, &pvAIButton)) {
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
        // if (keys[SDL_SCANCODE_W] && opponentPaddle.y > 0) {
        //     opponentPaddle.y -= PADDLE_SPEED;
        //     player2Moved = true;
        // }
        // if (keys[SDL_SCANCODE_S] && opponentPaddle.y < HEIGHT - PADDLE_HEIGHT) {
        //     opponentPaddle.y += PADDLE_SPEED;
        //     player2Moved = true;
        // }

        // Opponent controls
        if (currentGameState == PVP) {
            if (keys[SDL_SCANCODE_W] && opponentPaddle.y > 0) {
                opponentPaddle.y -= PADDLE_SPEED;
                player2Moved = true;
            }
            if (keys[SDL_SCANCODE_S] && opponentPaddle.y < HEIGHT - PADDLE_HEIGHT) {
                opponentPaddle.y += PADDLE_SPEED;
                player2Moved = true;
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
                opponentScore++;
                ResetBall();
            } else if (ball.x + BALL_SIZE >= WIDTH) {
                // Player scores a point.
                playerScore++;
                ResetBall();
            }
        }
    }
}

void mainloop() {
    Update();
    Render();
}

int main() {
    srand((unsigned int)time(NULL));
    ballDirectionX = (rand() % 2 ? 1 : -1);
    ballDirectionY = (rand() % 2 ? 1 : -1);

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

    // Cleanup before exiting
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
