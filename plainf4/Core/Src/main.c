/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ILI9341_GFX.h"
#include "fonts.h"
#include "ILI9341_STM32_Driver.h"
#include "math.h"
#include <stdlib.h>  // cần cho rand() và srand()
#include <time.h>    // cần cho time()
#include "stdio.h"
#define PLANE_WIDTH  20
#define PLANE_HEIGHT 20
#define BULLET_WIDTH 4
#define BULLET_HEIGHT 8
#define MAX_BULLETS 5
#define MAX_ENEMIES 15
#define BOSS_WIDTH 40
#define BOSS_HEIGHT 30
#define MAX_BOSS_BULLETS 50
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi5;
DMA_HandleTypeDef hdma_spi5_tx;

/* USER CODE BEGIN PV */
typedef struct {
  int x;
  int y;
  int active;
} Bullet;

typedef struct {
  int x;
  int y;
  int active;
} EnemyPlane;

typedef struct {
  int x, y;
  int hp;
  int active;
  int laser_timer;
} Boss;

EnemyPlane enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];
Boss boss;
Bullet boss_bullets[MAX_BOSS_BULLETS];
int boss_bullet_count = 0;
int plane_x = 50;
int plane_y = 60;
int plane_move_flag = 0;
int point = 0;
extern const uint8_t Arial_Narrow8x12[];
int current_enemy_count = 3; // Số địch hiện tại đang được sử dụng
int level = 1;
int a=1;
uint32_t last_boss_fire_time = 0;
const uint32_t boss_fire_interval = 3000; // 3 giây
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI5_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Game logic ----------------------------------------------------------------*/


// (x,y) = (0,0) tại góc trái gần nút bấm, x theo chiểu dài, y theo chiều rộng
void draw_plane(int x, int y) {
	  int x1 = x;
	  int y1 = y - PLANE_HEIGHT / 2;

	  int x2 = x;
	  int y2 = y + PLANE_HEIGHT / 2;

	  int x3 = x + PLANE_WIDTH;
	  int y3 = y;

	  // Vẽ viền tam giác
	  ILI9341_DrawLine(x1, y1, x2, y2, BLUE);
	  ILI9341_DrawLine(x2, y2, x3, y3, BLUE);
	  ILI9341_DrawLine(x3, y3, x1, y1, BLUE);
	  // Nếu có hàm tô màu tam giác:
}

void erase_plane(int x, int y) {
	 int x1 = x;
	  int y1 = y - PLANE_HEIGHT / 2;

	  int x2 = x;
	  int y2 = y + PLANE_HEIGHT / 2;

	  int x3 = x + PLANE_WIDTH;
	  int y3 = y;

	  // Vẽ lại bằng màu nền (trắng)
	  ILI9341_DrawLine(x1, y1, x2, y2, WHITE);
	  ILI9341_DrawLine(x2, y2, x3, y3, WHITE);
	  ILI9341_DrawLine(x3, y3, x1, y1, WHITE);
}

void draw_replane(int x, int y) {
  ILI9341_DrawRectangle(x, y, PLANE_WIDTH, PLANE_HEIGHT, GREEN);
}

void init_enemies() {
  for (int i = 0; i < current_enemy_count; i++) {
    enemies[i].x = 280 + i * 10;
    enemies[i].y = rand() % 100;
    enemies[i].active = 1;
  }
}

void erase_replane(int x, int y) {
  ILI9341_DrawRectangle(x, y, PLANE_WIDTH, PLANE_HEIGHT, WHITE);
}
void update_enemies() {
  for (int i = 0; i < current_enemy_count; i++) {
    if (enemies[i].active) {
      erase_replane(enemies[i].x, enemies[i].y);
      enemies[i].x -= 2;

      if (enemies[i].x <= 0) {
        enemies[i].x = 320;
        enemies[i].y = (enemies[i].y + 100 + rand()*3%30) % 220;
      }

      draw_replane(enemies[i].x, enemies[i].y); // vẽ lại enemy
    }
  }
}

void init_boss() {
  boss.x = 280;
  boss.y = 20;
  boss.hp = 10;
  boss.active = 1;
  boss.laser_timer = 0;
}

void draw_boss(int x, int y) {
  ILI9341_DrawRectangle(x, y, BOSS_WIDTH, BOSS_HEIGHT, RED);
}

void erase_boss(int x, int y) {
  ILI9341_DrawRectangle(x, y, BOSS_WIDTH, BOSS_HEIGHT, WHITE);
}

void fire_boss_laser() {
    for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
        if (!boss_bullets[i].active) {
            boss_bullets[i].x = boss.x ;
            boss_bullets[i].y = boss.y;
            boss_bullets[i].active = 1;
            break;
        }
    }
}

void update_boss_bullets() {
	for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
	    if (boss_bullets[i].active) {
	      // Xóa viên đạn cũ
	    	ILI9341_DrawRectangle(boss_bullets[i].x, boss_bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, WHITE);

	      // Di chuyển đạn xuống
	      boss_bullets[i].x -= 2;

	      // Nếu ra khỏi màn hình
	      if (boss_bullets[i].x<=0 ) {
	        boss_bullets[i].active = 0;
	        continue;
	      }

	      // Vẽ lại viên đạn mới
	      ILI9341_DrawRectangle(boss_bullets[i].x, boss_bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT, RED);

	      // Kiểm tra va chạm với máy bay người chơi
	      if (boss_bullets[i].x < plane_x + PLANE_WIDTH &&
	          boss_bullets[i].x + BULLET_WIDTH > plane_x &&
	          boss_bullets[i].y < plane_y + PLANE_HEIGHT &&
	          boss_bullets[i].y + BULLET_HEIGHT > plane_y) {
	        // Va chạm: Game over
	    	  ILI9341_FillScreen(WHITE);
	    	      	           ILI9341_DrawText("GAME OVER", FONT3, 50, 120, RED, WHITE);
	    	      	           while (1);
	      }
	    }
	  }
}


void update_boss() {
  if (!boss.active) return;
  erase_boss(boss.x, boss.y);
  if (boss.y <= 0) a=1;
  else if (boss.y >=210) a=-1; // Di chuyển lại từ phải sang trái
  boss.y += a;

  draw_boss(boss.x, boss.y);

  boss.laser_timer++;
  if (boss.laser_timer >= 100) { // Bắn laser sau mỗi 50 chu kỳ
    fire_boss_laser();
    boss.laser_timer = 0;
  }
}

void check_bullet_boss_collision() {
  if (!boss.active) return;

  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;

    if (bullets[i].x + BULLET_HEIGHT > boss.x &&
        bullets[i].x < boss.x + BOSS_WIDTH &&
        bullets[i].y + BULLET_WIDTH > boss.y &&
        bullets[i].y < boss.y + BOSS_HEIGHT) {

      bullets[i].active = 0;
      ILI9341_DrawRectangle(bullets[i].x, bullets[i].y,
                                  BULLET_HEIGHT, BULLET_WIDTH, WHITE);

      boss.hp--;
      if (boss.hp <= 0) {
        erase_boss(boss.x, boss.y);
        boss.active = 0;
        point += 50; // thưởng điểm
      }
    }
  }
}

void check_boss_bullet_collision_with_player() {
  for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
    if (!boss_bullets[i].active) continue;

    if (boss_bullets[i].x + 4 <= plane_x ||
        boss_bullets[i].x >=plane_x + PLANE_WIDTH ||
        boss_bullets[i].y + 4 <= plane_y - PLANE_WIDTH/2 ||
        boss_bullets[i].y >= plane_y + PLANE_HEIGHT/2) {
    }
    else {
    	ILI9341_FillScreen(WHITE);
    	           ILI9341_DrawText("GAME OVER", FONT3, 50, 120, RED, WHITE);
    	           while (1); // Dừng game tại đây
    }
  }
}

void draw_bullet(Bullet *b) {
  ILI9341_DrawRectangle(b->x, b->y,BULLET_HEIGHT , BULLET_WIDTH, RED);
}

void erase_bullet(Bullet *b) {
  ILI9341_DrawRectangle(b->x, b->y, BULLET_HEIGHT, BULLET_WIDTH, WHITE);
}

void shoot_bullet() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) {
      bullets[i].x = plane_x + PLANE_WIDTH;
      bullets[i].y = plane_y;
      bullets[i].active = 1;
      break;
    }
  }
}

void update_bullets() {
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (bullets[i].active) {
      erase_bullet(&bullets[i]);
      bullets[i].x += 5;
      if (bullets[i].x >= 320) {
        bullets[i].active = 0;
      } else {
        draw_bullet(&bullets[i]);
      }
    }
  }
}

void draw_score(int point) {
    char buffer[20];
    sprintf(buffer, "Score: %d", point);

    // Xóa vùng cũ (giả sử vùng rộng 120px, cao 18px)
    ILI9341_DrawRectangle(0, 0, 120, 18, WHITE);

    // Vẽ chuỗi mới (x=0, y=0), dùng font 11x18
    ILI9341_DrawText(buffer, Arial_Narrow8x12, 0, 0, BLACK, WHITE);
}

void check_bullet_enemy_collision() { //hàm check máy bay chạm đạn
  for (int i = 0; i < MAX_BULLETS; i++) {
    if (!bullets[i].active) continue;
    for (int j = 0; j < current_enemy_count; j++) {
      if (!enemies[j].active) continue;

      // Kiểm tra va chạm hình chữ nhật
      if (bullets[i].x + BULLET_HEIGHT > enemies[j].x &&
          bullets[i].x < enemies[j].x + PLANE_WIDTH &&
          bullets[i].y + BULLET_WIDTH > enemies[j].y &&
          bullets[i].y < enemies[j].y + PLANE_HEIGHT) {

        // Xóa máy bay địch và đạn
        erase_replane(enemies[j].x, enemies[j].y);
        erase_bullet(&bullets[i]);
        bullets[i].active = 0;
        enemies[j].active = 0;

        // (Tùy chọn) Reset lại địch sau khi bị bắn
        enemies[j].x = 320;
        enemies[j].y = rand() % 100;
        enemies[j].active = 1;

        //cộng điểm
        point += 10;

      }
    }
  }
}

int check_collision(int x1, int y1, int w1, int h1,
                    int x2, int y2, int w2, int h2) {
    return !(x1 + w1 <= x2 ||  // máy bay 1 ở hoàn toàn bên trái máy bay 2
             x2 + w2 <= x1 ||  // máy bay 2 ở hoàn toàn bên trái máy bay 1
             y1 + h1/2 <= y2 ||  // máy bay 1 ở hoàn toàn trên máy bay 2
             y1 - h1/2 >= y2+h2);   // máy bay 2 ở hoàn toàn trên máy bay 1
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
    MX_DMA_Init();
    MX_RTC_Init();
    MX_SPI5_Init();
    /* USER CODE BEGIN 2 */
    ILI9341_Init();
    ILI9341_FillScreen(WHITE);
    ILI9341_SetRotation(SCREEN_HORIZONTAL_2);

    srand(time(NULL));

    /* USER CODE END 2 */
    /* Infinite loop */
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = 0;

     draw_plane(plane_x, plane_y);
     init_enemies(); // ← Thêm dòng này
     draw_score(point);


     while (1) {

    	 draw_score(point);
       update_bullets();
       if(level <= 2){
       update_enemies(); // ← Thêm dòng này
       for (int i = 0; i <current_enemy_count ; i++) {
         if (enemies[i].active && check_collision(plane_x, plane_y, PLANE_WIDTH, PLANE_HEIGHT,
                                                  enemies[i].x, enemies[i].y, PLANE_WIDTH, PLANE_HEIGHT)) {
           // Hiển thị "Game Over"
           ILI9341_FillScreen(WHITE);
           ILI9341_DrawText("GAME OVER", FONT3, 50, 120, RED, WHITE);
           while (1); // Dừng game tại đây
         }
       }
       if (point >= level * 100 && current_enemy_count + 2 <= 10) {
                   level++;
                   current_enemy_count += 2;
                   init_enemies();

                   // Hiển thị thông báo level up
                   ILI9341_FillScreen(WHITE);
                   char msg[30];
                   sprintf(msg, "LEVEL %d", level);
                   ILI9341_DrawText(msg, FONT3, 50, 120, BLUE, WHITE);
                   HAL_Delay(1000);  // Hiển thị 1 giây
                   ILI9341_FillScreen(WHITE);  // Dọn lại màn hình
               }
       check_bullet_enemy_collision();
              if (plane_move_flag) {
                  plane_move_flag = 0;
                  int old_y = plane_y;
                  plane_y = (plane_y + 5) % 220;
                  erase_plane(plane_x, old_y);
                  draw_plane(plane_x, plane_y);
                  shoot_bullet();
              }
       }
       else if(level == 3){
              	 // Hiển thị thông báo level up

				  ILI9341_FillScreen(WHITE);
				  char msg[30];
				  sprintf(msg, "FINAL BOSS", level);
				  ILI9341_DrawText(msg, FONT3, 50, 120, BLUE, WHITE);
				  HAL_Delay(1000);  // Hiển thị 1 giây
				  ILI9341_FillScreen(WHITE);
				  init_boss();
				  draw_boss(boss.x, boss.y);
					  uint32_t now = HAL_GetTick(); // Lấy thời gian hiện tại (milis)

					  while(1){
					  if (now - last_boss_fire_time >= boss_fire_interval) {
					              fire_boss_laser();
					              last_boss_fire_time = now;
					          }
					  update_bullets();
					  HAL_Delay(10);
					          update_boss();
					          update_boss_bullets();
					          check_bullet_boss_collision();
					                 if (plane_move_flag) {
					                     plane_move_flag = 0;
					                     int old_y = plane_y;
					                     plane_y = (plane_y + 5) % 220;
					                     erase_plane(plane_x, old_y);
					                     draw_plane(plane_x, plane_y);
					                     shoot_bullet();
					                 }
					  }
				  }
               }

       HAL_Delay(200); // Điều chỉnh tốc độ trò chơi
     }

  /* USER CODE END 3 */

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI5_Init(void)
{

  /* USER CODE BEGIN SPI5_Init 0 */

  /* USER CODE END SPI5_Init 0 */

  /* USER CODE BEGIN SPI5_Init 1 */

  /* USER CODE END SPI5_Init 1 */
  /* SPI5 parameter configuration*/
  hspi5.Instance = SPI5;
  hspi5.Init.Mode = SPI_MODE_MASTER;
  hspi5.Init.Direction = SPI_DIRECTION_2LINES;
  hspi5.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi5.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi5.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi5.Init.NSS = SPI_NSS_SOFT;
  hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi5.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi5.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi5.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi5.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi5) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI5_Init 2 */

  /* USER CODE END SPI5_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12|GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PD12 PD13 */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PG13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

//}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
