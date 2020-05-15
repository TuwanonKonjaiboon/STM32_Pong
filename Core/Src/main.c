/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
struct Ball {
	int prevX, prevY; // Previous position
	int posX, posY; // position
	int mvH, mvV; // velocity
};

struct Paddle {
	uint8_t id;
	int prevX, prevY;
	int posX, posY;
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define FORCE 1

#define SCREEN_WIDTH 79
#define SCREEN_HEIGHT 25

// program state
#define NULL_SCREEN -1
#define START_SCREEN 0
#define GAME_SCREEN 1
#define END_SCREEN 10
#define CREDIT_SCREEN 99

// color
#define COLOR_DEFAULT 0
#define COLOR_BLACK 0
#define COLOR_WHITE 15
#define COLOR_RED 196
#define COLOR_BLUE 21
#define COLOR_BLUE_SKY 51
#define COLOR_LIGHT_GREEN 40
#define COLOR_DARK_GREEN 34
#define COLOR_YELLOW 226
#define COLOR_PURPLE 99

// constant
#define PADDLE_SIZE_X 1
#define PADDLE_SZIE_Y 3 // Half
#define PADDLE_SPEED 2
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
int state = START_SCREEN;
int tracker = NULL_SCREEN;
int score1 = 0, score2 = 0;
uint8_t isGameStart = 0, isGameEnd = 0, enableRandom = 0;
uint8_t winner = 0, win_score = 3;
struct Ball ball = {
		.posX = 40,
		.posY = 12,
		.mvH = 2,
		.mvV = 1,
};
struct Paddle paddle1 = {
	.id = 1,
	.posX = 75,
	.posY = 12
};
struct Paddle paddle2 = {
	.id = 2,
	.posX = 5,
	.posY = 12,
};
char rxData[10];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void welcome(void);
void initial(void);
void gameover(void);
void tick(void);
void render(void);
void reset(void);
// drawing
void drawScoreBar(void);
void drawZoneLine(void);
void drawPaddle(struct Paddle *paddle, uint8_t force);
void drawBall(struct Ball *ball);
// update value
void setPaddlePostion(struct Paddle* paddle, int value);
void setBallPosition(struct Ball *ball, int col, int row);
void updateBallPosition(struct Ball *ball);
void resetPosition(void);
void resetVelocity(void);
void resetScore(void);
void reset(void);
uint8_t checkIfScore(void);
// screen related
void clrscr(void);
void print(const char *str);
void set_cursor(uint8_t col, uint8_t row);
// print message
void printGameTitle(uint8_t start_col, uint8_t start_row);
void printHTP(uint8_t start_col, uint8_t start_row);
void printGameOver(uint8_t start_col, uint8_t start_row);
void printCredit(uint8_t start_col, uint8_t start_row);
// utility
int boundMaxMin(int value, int max, int min);
void setBackgroundColor(uint8_t color);
void setForegroundColor(uint8_t color);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  /* here we consume the current value of rx */
  // Debug Blinking
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);
  	switch (state) {
  	case START_SCREEN:
  		switch (rxData[0]) {
  		case 's': case 'S':
  			state = GAME_SCREEN;

  			// Redraw screen to Start New Game
  			resetPosition();
  			resetScore();
  			isGameStart = 0;
  			isGameEnd = 0;
  			enableRandom = 0;
  			initial();

  			break;
  		case '1': win_score = 1; tracker = NULL_SCREEN; break;
  		case '2': win_score = 2; tracker = NULL_SCREEN; break;
  		case '3': win_score = 3; tracker = NULL_SCREEN; break;
  		case '4': win_score = 4; tracker = NULL_SCREEN; break;
  		case '5': win_score = 5; tracker = NULL_SCREEN; break;
  		case '6': win_score = 6; tracker = NULL_SCREEN; break;
  		case '7': win_score = 7; tracker = NULL_SCREEN; break;
  		case '8': win_score = 8; tracker = NULL_SCREEN; break;
  		case '9': win_score = 9; tracker = NULL_SCREEN; break;
  		case '0': win_score = 10; tracker = NULL_SCREEN; break;
  		}
  		break;
  	case GAME_SCREEN:
  		// Any key would trigger this
  		isGameStart = 1;
  		switch(rxData[0]) {
		case 'A':
			setPaddlePostion(&paddle1, paddle1.posY-PADDLE_SPEED);
			break;
		case 'B':
			setPaddlePostion(&paddle1, paddle1.posY+PADDLE_SPEED);
			break;
		case 'w':
			setPaddlePostion(&paddle2, paddle2.posY-PADDLE_SPEED);
			break;
		case 's':
			setPaddlePostion(&paddle2, paddle2.posY+PADDLE_SPEED);
			break;
		case 'x': case 'X':
			state = START_SCREEN;

			resetPosition();
			resetScore();
			break;
		case 'c': case 'C':
			enableRandom = 1;
			set_cursor(65, 24);
			print("Random mode ON!");
			break;
		}
  		break;
  	case END_SCREEN:
  		switch(rxData[0]) {
  		case 'r': case 'R':
  			state = GAME_SCREEN;

			// Redraw screen to Start New Game
			resetPosition();
			resetScore();
			isGameStart = 0;
			isGameEnd = 0;
			enableRandom = 0;
			initial();
  			break;
  		case 'x': case 'X':
  			state = START_SCREEN;
  			break;
  		}
  		break;
  	}


  /* set up to receive another char */
  HAL_UART_Receive_IT(&huart2, rxData, 1);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_UART_Receive_IT(&huart2, (uint8_t*)rxData, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 3500;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	switch (state) {
	case START_SCREEN:
		// Start screen
		welcome();
		break;
	case GAME_SCREEN:
		if (isGameStart) {
			if (!isGameEnd) {
				tick();
				render();
			} else {
				state = END_SCREEN;
				clrscr();
			}
		}
		break;
	case END_SCREEN:
		gameover();
		break;
	case CREDIT_SCREEN:
		break;
	}

	// Change tracker point
	if (tracker != state)
		tracker = state;

	// Toggle LED
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);
}

const uint8_t bst = 93;
const uint8_t bed = 226;
uint8_t bc = 93;

// Tick()
void tick() {
	// Update Ball Position
	updateBallPosition(&ball);

	// Score's
	switch (checkIfScore()) {
	case 1:
		// Player 1 Score!
		score1++;
		reset();
		break;
	case 2:
		// Player 2 Score!
		score2++;
		reset();
		break;
	default:
		// Continue;
		break;
	}

	// Game have ended
	if (score1 >= win_score || score2 >= win_score) {
		isGameEnd = 1;
		isGameStart = 1;
	}
}

// Update()
void render() {
	drawPaddle(&paddle1, 0);
	drawPaddle(&paddle2, 0);
	drawBall(&ball);
}

// Complex &
// Screens
void welcome() {
	if (tracker == state) return;

	// Draw Game Screen for being background
	initial();

	const char buffer[] = "\033[?25l"; // Invisible Cursor

	print(buffer);

	printGameTitle(31, 5);
	printHTP(25, 15);
	printCredit(60, 24);
}

void initial() {
	clrscr();
	drawScoreBar();
	drawZoneLine();
	drawPaddle(&paddle1, 1);
	drawPaddle(&paddle2, 1);

	if (enableRandom) {
		set_cursor(65, 24);
		print("Random mode ON!");
	}

	srand(SysTick->VAL);

	return;
}

void gameover() {
	winner = (score1 > score2) ? 1 : 2;
	printGameOver(22, 9);
}

uint8_t checkIfScore() {
	struct Ball *b = &ball;
	if (b->mvH > 0 && b->posX + b->mvH >= SCREEN_WIDTH) { // Player2 score!
		return 2;
	} else if (b->mvH < 0 && b->posX + b->mvH <= 0) { // Player1 score!
		return 1;
	}
	else {
		return 0;
	}
}

// Draw functions
void drawScoreBar() {
	char buff[16];
	set_cursor(45, 0);
	setForegroundColor(COLOR_RED);
	HAL_UART_Transmit(&huart2, (uint8_t*)buff, sprintf(buff, "%d", score1), HAL_MAX_DELAY);
	set_cursor(77, 0);
	print("P1");
	set_cursor(35, 0);
	setForegroundColor(COLOR_BLUE);
	HAL_UART_Transmit(&huart2, (uint8_t*)buff, sprintf(buff, "%d", score2), HAL_MAX_DELAY);
	set_cursor(3, 0);
	print("P2");

	setForegroundColor(COLOR_WHITE);
}

void drawPaddle(struct Paddle* paddle, uint8_t force) {
	if (!force && paddle->prevX == paddle->posX && paddle->prevY == paddle->posY) return;

//	char clean = ' ';

	// Clean up Old
	set_cursor(paddle->posX, paddle->prevY-3);
	setBackgroundColor(COLOR_DEFAULT);
	for (uint8_t j = paddle->prevY-3; j <= paddle->prevY+3; ++j) {
		set_cursor(paddle->posX, j);
		print(" ");
	}

	// Set new values to previous position
	paddle->prevX = paddle->posX;
	paddle->prevY = paddle->posY;

	// Render New
	set_cursor(paddle->posX, paddle->posY-3);
	setBackgroundColor(paddle->id == 1 ? COLOR_RED : COLOR_BLUE);
	for (uint8_t j = paddle->posY-3; j <= paddle->posY+3; ++j) {
		set_cursor(paddle->posX, j);
		print(" ");
	}

	setBackgroundColor(COLOR_DEFAULT);
	return;
}

void drawZoneLine() {
	set_cursor(40, 0);
	setBackgroundColor(COLOR_LIGHT_GREEN);
	for (uint8_t i = 0; i <= SCREEN_HEIGHT; ++i) {
		set_cursor(40, i);
		print(" ");
	}
	setBackgroundColor(COLOR_DEFAULT);
	return;
}

void drawBall(struct Ball *ball) {
	if (ball->prevX == ball->posX && ball->prevY == ball->posY) return;

	// Clean up Old
	set_cursor(ball->prevX, ball->prevY);
	setBackgroundColor(ball->prevX == 40 ? COLOR_LIGHT_GREEN : COLOR_DEFAULT);
	print(" ");

	// Update previous position
	ball->prevX = ball->posX;
	ball->prevY = ball->posY;

	// Render New
	set_cursor(ball->posX, ball->posY);
	setBackgroundColor(bc++);
	bc %= bed;
	if (bc == 0) {
		bc = bst;
	}
	print(" ");

	setBackgroundColor(COLOR_DEFAULT);
	return;
}

// update functions
void updateBallPosition(struct Ball *ball) {

	struct Paddle *p1 = &paddle1;
	struct Paddle *p2 = &paddle2;

	if (p1 == NULL || p2 == NULL) return;

	if ((ball->mvH > 0 && ball->posX + ball->mvH >= p1->posX && (ball->posY + ball->mvV <= p1->posY+PADDLE_SZIE_Y && ball->posY + ball->mvV >= p1->posY-PADDLE_SZIE_Y)) ||
		(ball->mvH < 0 && ball->posX + ball->mvH <= p2->posX && (ball->posY + ball->mvV <= p2->posY+PADDLE_SZIE_Y && ball->posY + ball->mvV >= p2->posY-PADDLE_SZIE_Y)))
	{
		if (enableRandom) {
			uint8_t chance = rand() % 101;
			int sign = ball->mvH < 0 ? -1 : 1;
			if (chance > 75) {
				ball->mvH = 2 * sign;
			} else {
				ball->mvH = 3 * sign;
			}

			if (chance > 50) {
				ball->mvV = -ball->mvV;
			}
		}
		ball->mvH = -ball->mvH;
	}

	ball->posX = boundMaxMin(ball->posX+ball->mvH, SCREEN_WIDTH, 0);

	if ((ball->mvV < 0 && ball->posY + ball->mvV <= 0) ||
		(ball->mvV > 0 && ball->posY + ball->mvV >= SCREEN_HEIGHT)) {
		if (enableRandom) {
			uint8_t chance = rand() % 101;
			int sign = ball->mvV < 0 ? -1 : 1;
			if (chance > 20) {
				ball->mvV = 1 * sign;
			} else {
				ball->mvV = 2 * sign;
			}
		}
		ball->mvV = -ball->mvV;
	}

	ball->posY = boundMaxMin(ball->posY+ball->mvV, SCREEN_HEIGHT, 0);
}

void resetPosition() {
	// Reset Position Paddle
	setPaddlePostion(&paddle1, 12);
	setPaddlePostion(&paddle2, 12);
	// reset Postion Ball
	setBallPosition(&ball, 40, 12);
}

void resetVelocity() {
	ball.mvH = 2;
	ball.mvV = 1;
}

void resetScore() {
	score1 = score2 = 0;
}

void reset() {
	resetPosition();
	resetVelocity();
	// No reset Score <-- the difference

	// Stop the game til user input any
	isGameStart = 0;

	// Re draw all
	initial();
}

// Handle User's input functions
void setPaddlePostion(struct Paddle* paddle, int value) {
	paddle->posY = boundMaxMin(value, SCREEN_HEIGHT-3, 4);
}

void setBallPosition(struct Ball *ball, int col, int row) {
	ball->posX = boundMaxMin(col, SCREEN_WIDTH, 0);
	ball->posY = boundMaxMin(row, SCREEN_HEIGHT, 0);
}

// Utillity functions
int boundMaxMin(int value, int max, int min) {
	value = value > max ? max : value;
	value = value < min ? min : value;
	return value;
}

void print(const char *str) {
	char buffer[55];
	HAL_UART_Transmit(&huart2, buffer, sprintf(buffer, "%s", str), HAL_MAX_DELAY);
	return;
}

void set_cursor(uint8_t col, uint8_t row) {
	char cmd[50];
	sprintf(cmd, "\033[%d;%dH", row, col); // [{row};{col}H
	HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
}

void clrscr(void) {
	const char clear[] = "\033[2J";
	const char jump_to_home[] = "\033[H\r";
	HAL_UART_Transmit(&huart2, (uint8_t*)clear, sizeof(clear), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, (uint8_t*)jump_to_home, sizeof(jump_to_home), HAL_MAX_DELAY);
}

void printGameTitle(uint8_t start_col, uint8_t start_row) {
	uint8_t col = start_col, row = start_row;
	// Print P
	set_cursor(col, row);
	setForegroundColor(COLOR_RED);
	print("## "); set_cursor(col, row+1);
	print("# #"); set_cursor(col, row+2);
	print("## "); set_cursor(col, row+3);
	print("#  "); set_cursor(col, row+4);
	print("#  "); set_cursor(col, row+5);
	// Print O
	set_cursor(col+4, row);
	setForegroundColor(COLOR_YELLOW);
	print(" ## "); set_cursor(col+4, row+1);
	print("#  #"); set_cursor(col+4, row+2);
	print("#  #"); set_cursor(col+4, row+3);
	print("#  #"); set_cursor(col+4, row+4);
	print(" ## "); set_cursor(col+4, row+5);
	set_cursor(col+9, row);
	setForegroundColor(COLOR_LIGHT_GREEN);
	print("#   #"); set_cursor(col+9, row+1);
	print("##  #"); set_cursor(col+9, row+2);
	print("# # #"); set_cursor(col+9, row+3);
	print("#  ##"); set_cursor(col+9, row+4);
	print("#   #"); set_cursor(col+9, row+5);
	set_cursor(col+15, row);
	setForegroundColor(COLOR_PURPLE);
	print(" ## "); set_cursor(col+15, row+1);
	print("#  #"); set_cursor(col+15, row+2);
	print("# ##"); set_cursor(col+15, row+3);
	print("#  #"); set_cursor(col+15, row+4);
	print(" ## "); set_cursor(col+15, row+5);
	// Print N
	// Print G

	setBackgroundColor(COLOR_DEFAULT);
	setForegroundColor(COLOR_WHITE);
}

void printHTP(uint8_t start_col, uint8_t start_row) {
	uint8_t col = start_col, row = start_row;
	setForegroundColor(COLOR_RED);
	set_cursor(col, row++);
	print("*Note, Do not hold the button!");
	set_cursor(col, row++);
	print("   Use SPAMING instead...");
	set_cursor(col, row++);
	set_cursor(col, row++);
	setForegroundColor(COLOR_YELLOW);
	char buffer[50];
	sprintf(buffer, "Who got %d point before, win!", win_score);
	print(buffer);
	set_cursor(col, row++);
	print("paddle 1 (right) use arrow \'up\' & \'down\'");
	set_cursor(col, row++);
	print("paddle 2 (left) use key \'w\' & \'s\'");
	set_cursor(col, row++);
	set_cursor(col, row++);
	setForegroundColor(COLOR_WHITE);
	print("Press 's' to start the game");
}

void printGameOver(uint8_t start_col, uint8_t start_row) {
	const uint8_t colors[] = { 40, 81, 220, 202, 127 };
	uint8_t col = start_col, row = start_row;
	char *msg1[5] = {
			"#   # ### #   # #   # #### ###      #   ",
			"#   #  #  ##  # ##  # #    #  #   ###   ",
			"# # #  #  # # # # # # #### ###      #   ",
			"## ##  #  #  ## #  ## #    #  #     #   ",
			"#   # ### #   # #   # #### #  #    ###  "
	};
	char *msg2[5] = {
			"#   # ### #   # #   # #### ###    ###   ",
			"#   #  #  ##  # ##  # #    #  #     #   ",
			"# # #  #  # # # # # # #### ###    ###   ",
			"## ##  #  #  ## #  ## #    #  #   #     ",
			"#   # ### #   # #   # #### #  #   ###   "
		};

	set_cursor(col, row++); print("Press 'x' go to START SCREEN");
	set_cursor(col, row++); print("Press 'r' restart the match!");

	set_cursor(col, row++); set_cursor(col, row++);

	switch (winner) {
	case 1:
		setForegroundColor(colors[0]); set_cursor(col, row++); print(msg1[0]);
		setForegroundColor(colors[1]); set_cursor(col, row++); print(msg1[1]);
		setForegroundColor(colors[2]); set_cursor(col, row++); print(msg1[2]);
		setForegroundColor(colors[3]); set_cursor(col, row++); print(msg1[3]);
		setForegroundColor(colors[4]); set_cursor(col, row++); print(msg1[4]);
		break;
	case 2:
		setForegroundColor(colors[0]); set_cursor(col, row++); print(msg2[0]);
		setForegroundColor(colors[1]); set_cursor(col, row++); print(msg2[1]);
		setForegroundColor(colors[2]); set_cursor(col, row++); print(msg2[2]);
		setForegroundColor(colors[3]); set_cursor(col, row++); print(msg2[3]);
		setForegroundColor(colors[4]); set_cursor(col, row++); print(msg2[4]);
		break;
	}

	setForegroundColor(COLOR_WHITE);
	return;
}

void printCredit(uint8_t start_col, uint8_t start_row) {
	set_cursor(start_col, start_row);
	char cpr = 184;
	HAL_UART_Transmit(&huart2, &cpr, 1, HAL_MAX_DELAY);
	print("2020 KoFeeBriAN");
}
/* USER CODE END 4 */

void setBackgroundColor(uint8_t color) {
	char buffer[50];
	HAL_UART_Transmit(&huart2, buffer, sprintf(buffer, "\033[48:5:%dm", color), HAL_MAX_DELAY);
	return;
}

void setForegroundColor(uint8_t color) {
	char buffer[50];
	HAL_UART_Transmit(&huart2, buffer, sprintf(buffer, "\033[38:5:%dm", color), HAL_MAX_DELAY);
	return;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
