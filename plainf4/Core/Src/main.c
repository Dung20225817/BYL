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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
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
#define MAX_BOSS_BULLETS 3000

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
typedef struct { //khai báo kiểu dữ liệu bullet
	int x;
	int y;
	int active;
} Bullet;

typedef struct { // khai báo kiểu dữ liệu máy bay địch
	int x;
	int y;
	int active;
} EnemyPlane;

typedef struct { // khai báo kiểu dữ liệu boss
	int x, y;
	int hp;
	int active;
	int laser_timer;
} Boss;

//đoạn này nhằm khai báo các biến cục bộ
EnemyPlane enemies[MAX_ENEMIES]; //tạo mảng lưu số lượng máy bay địch tối đa
Bullet bullets[MAX_BULLETS]; //tạo mảng lưu trữ số lượng đạn bắn ra tối đa
Boss boss;
Bullet boss_bullets[MAX_BOSS_BULLETS]; // số lượng đạn boss bắn ra tối đa
int boss_bullet_count = 0;
int plane_x = 50;
int plane_y = 60;
int plane_move_flag = 0;
int plane_move_left_flag = 0;     // Dùng cho nút trái
int plane_move_right_flag = 0;    // Dùng cho nút phải
int point = 0;
int pattern = 0;
extern const uint8_t Arial_Narrow8x12[];
int current_enemy_count = 3; // Số địch hiện tại đang được sử dụng
int level = 1;
int a = 1; //biến a thay đổi để tính di chuyển của boss
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
void draw_plane(int x, int y) { // O(x,y) trung điểm của cạnh đáy tam giác
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

void erase_plane(int x, int y) { // hàm xóa máy bay
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

void draw_enemies(int x, int y) { //hàm vẽ enemies
	ILI9341_DrawRectangle(x, y, PLANE_WIDTH, PLANE_HEIGHT, GREEN);
}

void init_enemies() { //hàm khởi tạo enemies
	for (int i = 0; i < current_enemy_count; i++) {
		enemies[i].x = 280 + i * 10;
		enemies[i].y = rand() % 100;
		enemies[i].active = 1;
	}
}

void erase_enemies(int x, int y) { //hàm xóa enemies
	ILI9341_DrawRectangle(x, y, PLANE_WIDTH, PLANE_HEIGHT, WHITE);
}

void update_enemies() { //hàm cập nhật enemies
	for (int i = 0; i < current_enemy_count; i++) {
		if (enemies[i].active) {
			erase_enemies(enemies[i].x, enemies[i].y);
			enemies[i].x -= 2;

			if (enemies[i].x <= 0) {
				enemies[i].x = 320;
				enemies[i].y = (enemies[i].y + 100 + rand() * 3 % 70) % 220;
			}

			draw_enemies(enemies[i].x, enemies[i].y); // vẽ lại enemy
		}
	}
}

void init_boss() { //hàm khởi tạo boss
	boss.x = 280;
	boss.y = 20;
	boss.hp = 10;
	boss.active = 1;
	boss.laser_timer = 0;
}

void draw_boss(int x, int y) { //vẽ boss
	ILI9341_DrawRectangle(x, y, BOSS_WIDTH, BOSS_HEIGHT, RED);
}

void erase_boss(int x, int y) { //xóa boss
	ILI9341_DrawRectangle(x, y, BOSS_WIDTH, BOSS_HEIGHT, WHITE);
}

void fire_boss_laser() { // hàm bắn đạn của boss
	for (int i = 0; i < MAX_BOSS_BULLETS; i += 3) {
		if (!boss_bullets[i].active) {
			boss_bullets[i].x = boss.x;
			boss_bullets[i].y = boss.y;
			boss_bullets[i].active = 1;

			boss_bullets[i + 1].x = boss.x;
			boss_bullets[i + 1].y = boss.y;
			boss_bullets[i + 1].active = 1;

			boss_bullets[i + 2].x = boss.x;
			boss_bullets[i + 2].y = boss.y;
			boss_bullets[i + 2].active = 1;
			break;
		}
	}
}

void boss_shotgun(int i) {
	// Xóa viên đạn cũ
	ILI9341_DrawRectangle(boss_bullets[i].x, boss_bullets[i].y,
	BULLET_HEIGHT, BULLET_WIDTH, WHITE);
	ILI9341_DrawRectangle(boss_bullets[i + 1].x, boss_bullets[i + 1].y,
	BULLET_HEIGHT, BULLET_WIDTH, WHITE);
	ILI9341_DrawRectangle(boss_bullets[i + 2].x, boss_bullets[i + 2].y,
	BULLET_HEIGHT, BULLET_WIDTH, WHITE);

	// Di chuyển đạn xuống
	boss_bullets[i].x -= 2;

	boss_bullets[i + 1].x -= 2;
	boss_bullets[i + 1].y += 1;

	boss_bullets[i + 2].x -= 2;
	boss_bullets[i + 2].y -= 1;

	// Nếu ra khỏi màn hình
//	if (boss_bullets[i].x <= 0) {
//		boss_bullets[i].active = 0;
//		boss_bullets[i + 1].active = 0;
//		boss_bullets[i + 2].active = 0;
//		return;
//	}

	// Vẽ lại viên đạn mới
	ILI9341_DrawRectangle(boss_bullets[i].x, boss_bullets[i].y,
	BULLET_HEIGHT, BULLET_WIDTH, RED);

	ILI9341_DrawRectangle(boss_bullets[i + 1].x, boss_bullets[i + 1].y,
	BULLET_HEIGHT, BULLET_WIDTH, RED);

	ILI9341_DrawRectangle(boss_bullets[i + 2].x, boss_bullets[i + 2].y,
	BULLET_HEIGHT, BULLET_WIDTH, RED);

	check_lose(i);
}

void boss_burst(int i) {
	// Xóa viên đạn cũ
	ILI9341_DrawRectangle(boss_bullets[i].x, boss_bullets[i].y,
	BULLET_HEIGHT, BULLET_WIDTH, WHITE);
	ILI9341_DrawRectangle(boss_bullets[i + 1].x, boss_bullets[i + 1].y,
	BULLET_HEIGHT, BULLET_WIDTH, WHITE);
	ILI9341_DrawRectangle(boss_bullets[i + 2].x, boss_bullets[i + 2].y,
	BULLET_HEIGHT, BULLET_WIDTH, WHITE);

	// Di chuyển đạn xuống
	boss_bullets[i].x -= 2;

	boss_bullets[i + 1].x = boss_bullets[i].x + 32;

	boss_bullets[i + 2].x = boss_bullets[i].x + 64;

	// Nếu ra khỏi màn hình
//	if (boss_bullets[i].x <= 0) {
//		boss_bullets[i].active = 0;
//		boss_bullets[i + 1].active = 0;
//		boss_bullets[i + 2].active = 0;
//		return;
//	}

	// Vẽ lại viên đạn mới
	ILI9341_DrawRectangle(boss_bullets[i].x, boss_bullets[i].y,
	BULLET_HEIGHT, BULLET_WIDTH, RED);

	ILI9341_DrawRectangle(boss_bullets[i + 1].x, boss_bullets[i + 1].y,
	BULLET_HEIGHT, BULLET_WIDTH, RED);

	ILI9341_DrawRectangle(boss_bullets[i + 2].x, boss_bullets[i + 2].y,
	BULLET_HEIGHT, BULLET_WIDTH, RED);

	check_lose(i);
}

void boss_wave(int i) {
	// Xóa viên đạn cũ
	ILI9341_DrawRectangle(boss_bullets[i].x, boss_bullets[i].y,
	BULLET_HEIGHT, BULLET_WIDTH, WHITE);
	ILI9341_DrawRectangle(boss_bullets[i + 1].x, boss_bullets[i + 1].y,
	BULLET_HEIGHT, BULLET_WIDTH, WHITE);
	ILI9341_DrawRectangle(boss_bullets[i + 2].x, boss_bullets[i + 2].y,
	BULLET_HEIGHT, BULLET_WIDTH, WHITE);

	// Di chuyển đạn xuống
	boss_bullets[i].x -= 2;

	boss_bullets[i + 1].x = boss_bullets[i].x + 16;
	boss_bullets[i + 1].y = boss_bullets[i].y + 16;

	boss_bullets[i + 2].x = boss_bullets[i].x + 16;
	boss_bullets[i + 2].y = boss_bullets[i].y - 16;

	// Nếu ra khỏi màn hình
//	if (boss_bullets[i].x <= 0) {
//		boss_bullets[i].active = 0;
//		boss_bullets[i + 1].active = 0;
//		boss_bullets[i + 2].active = 0;
//		return;
//	}

	// Vẽ lại viên đạn mới
	ILI9341_DrawRectangle(boss_bullets[i].x, boss_bullets[i].y,
	BULLET_HEIGHT, BULLET_WIDTH, RED);

	ILI9341_DrawRectangle(boss_bullets[i + 1].x, boss_bullets[i + 1].y,
	BULLET_HEIGHT, BULLET_WIDTH, RED);

	ILI9341_DrawRectangle(boss_bullets[i + 2].x, boss_bullets[i + 2].y,
	BULLET_HEIGHT, BULLET_WIDTH, RED);

	check_lose(i);
}

void update_boss_bullets() { // hàm cập nhật tình trạng đạn của boss
	//nếu viên đạn được active, vẽ lại đường đạn đi xuống, nếu va chạm vào máy bay thì thua
	for (int i = 0; i < MAX_BOSS_BULLETS; i += 3) {
		if (boss_bullets[i].active) {
			if ((i / 3) % 3 == 0) {
				boss_shotgun(i);
			} else if ((i / 3) % 3 == 1) {
				boss_burst(i);
			} else if ((i / 3) % 3 == 2) {
				boss_wave(i);
			}
		}
	}
}

void check_lose(int i) {
// Kiểm tra va chạm với máy bay người chơi
	for (int j = i; j < i + 3; j++) {
		if (boss_bullets[j].x + 4 <= plane_x
				|| boss_bullets[j].x >= plane_x + PLANE_WIDTH
				|| boss_bullets[j].y + 4 <= plane_y - PLANE_WIDTH / 2
				|| boss_bullets[j].y >= plane_y + PLANE_HEIGHT / 2) {
		} else {
			show_game_over_screen();
			while (1)
				; // Dừng game tại đây
		}
	}
}

void update_boss() {
//hàm cập nhật tình trạng của boss, boss di chuyển qua lại màn hình, sau 100 chu kì sẽ bắn đạn 1 lần
	if (!boss.active)
		return;
	erase_boss(boss.x, boss.y);
	if (boss.y <= 0)
		a = 1;
	else if (boss.y >= 210)
		a = -1; // Di chuyển lại từ phải sang trái
	boss.y += a;

	draw_boss(boss.x, boss.y);

	boss.laser_timer++;
	if (boss.laser_timer >= 100) { // Bắn laser sau mỗi 100 chu kỳ
		fire_boss_laser();
		boss.laser_timer = 0;
	}
}

int check_bullet_boss_collision() {
// hàm check va chạm boss với đạn, nếu bắn boss đủ số lượng đạn, boss sẽ chết
	if (!boss.active)
		return 0;

	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!bullets[i].active)
			continue;

		if (bullets[i].x + BULLET_HEIGHT > boss.x&&
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
				return 1;
			}
		}
	}
	return 0;
}

//void check_boss_bullet_collision_with_player() {
//	// hàm check va chạm của máy bay và đạn cuẩ boss
//	for (int i = 0; i < MAX_BOSS_BULLETS; i++) {
//		if (!boss_bullets[i].active)
//			continue;
//
//		if (boss_bullets[i].x + 4 <= plane_x
//				|| boss_bullets[i].x >= plane_x + PLANE_WIDTH
//				|| boss_bullets[i].y + 4 <= plane_y - PLANE_WIDTH / 2
//				|| boss_bullets[i].y >= plane_y + PLANE_HEIGHT / 2) {
//		} else {
//			ILI9341_FillScreen(WHITE);
//			ILI9341_DrawText("GAME OVER", FONT3, 50, 120, RED, WHITE);
//			while (1)
//				; // Dừng game tại đây
//		}
//	}
//}

void draw_bullet(Bullet *b) {
	ILI9341_DrawRectangle(b->x, b->y, BULLET_HEIGHT, BULLET_WIDTH, BLUE);
}

void erase_bullet(Bullet *b) {
	ILI9341_DrawRectangle(b->x, b->y, BULLET_HEIGHT, BULLET_WIDTH, WHITE);
}

void shoot_bullet() {
//hàm khởi tạo đạn của máy bay
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
//hàm cập nhật tình trạng đạn, nếu viên đạn active thì sẽ di chuyển lên
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

void check_bullet_enemy_collision() { //hàm check máy bay địch chạm đạn
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (!bullets[i].active)
			continue;
		for (int j = 0; j < current_enemy_count; j++) {
			if (!enemies[j].active)
				continue;

			// Kiểm tra va chạm hình chữ nhật
			if (bullets[i].x + BULLET_HEIGHT > enemies[j].x&&
			bullets[i].x < enemies[j].x + PLANE_WIDTH &&
			bullets[i].y + BULLET_WIDTH > enemies[j].y &&
			bullets[i].y < enemies[j].y + PLANE_HEIGHT) {

				// Xóa máy bay địch và đạn
				erase_enemies(enemies[j].x, enemies[j].y);
				erase_bullet(&bullets[i]);
				bullets[i].active = 0;
				enemies[j].active = 0;

				// (Tùy chọn) Reset lại địch sau khi bị bắn( đảm bảo màn hình luốn đủ số lượng địch)
				enemies[j].x = 320;
				enemies[j].y = rand() % 100; // rand chạy từ 0 -> 32767
				enemies[j].active = 1;

				//cộng điểm
				point += 10;

			}
		}
	}
}

int check_collision(int x1, int y1, int w1, int h1, int x2, int y2, int w2,
		int h2)
// hàm check va chạm máy bay ta với địch
{
	return !(x1 + w1 <= x2 ||  // máy bay ta ở hoàn toàn ở dưới máy bay địch
			x2 + w2 <= x1 ||  // máy bay địch hoàn toàn ở dưới máy bay ta
			y1 + h1 / 2 <= y2 ||  // máy bay ta hoàn toàn bên phải máy bay đich
			y1 - h1 / 2 >= y2 + h2); // máy bay địch ở hoàn toàn bên phải máy bay ta
}

void show_victory_screen() { //màn hình chiến thắng
	ILI9341_FillScreen(BLACK);

// Hiệu ứng "pháo hoa" đơn giản
	for (int r = 0; r < 60; r += 4) {
		ILI9341_DrawHollowCircle(80, 80, r, YELLOW);
		ILI9341_DrawHollowCircle(240, 100, r, GREEN);
		ILI9341_DrawHollowCircle(160, 160, r, RED);
		HAL_Delay(30);
	}

// Tiêu đề chiến thắng
	ILI9341_DrawText("YOU WIN!", FONT3, 90, 100, GREEN, BLACK);
	ILI9341_DrawText("Congratulations", FONT3, 60, 160, WHITE, BLACK);
	ILI9341_DrawText("Press RESET to play again", FONT2, 30, 270, CYAN, BLACK);
}

void show_game_over_screen() {
	ILI9341_FillScreen(BLACK);

	ILI9341_DrawText("GAME OVER", FONT3, 80, 80, RED, BLACK);
	ILI9341_DrawText("Thanks for playing!", FONT3, 60, 140, WHITE, BLACK);

	for (int r = 0; r < 50; r += 5) {
		ILI9341_DrawFilledCircle(160, 240, r, YELLOW);
		HAL_Delay(30);
	}

	ILI9341_DrawText("Press RESET to play again", FONT2, 40, 270, BLUE, BLACK);
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

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
	ILI9341_FillScreen(WHITE); //xóa màn hình cũ
	ILI9341_SetRotation(SCREEN_HORIZONTAL_2); // khởi tạo trục Oxy

	srand(time(NULL)); // lệnh này khởi tạo "seed" (hạt giống) cho bộ sinh số ngẫu nhiên rand()

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		/* Infinite loop */
		for (int i = 0; i < MAX_BULLETS; i++)
			bullets[i].active = 0;

		draw_plane(plane_x, plane_y);
		init_enemies(); // ← Thêm dòng này
		draw_score(point);

		while (1) {

			draw_score(point);
			update_bullets();
			if (level <= 2) {
				update_enemies(); // ← Thêm dòng này
				for (int i = 0; i < current_enemy_count; i++) {
					if (enemies[i].active
							&& check_collision(plane_x, plane_y, PLANE_WIDTH,
							PLANE_HEIGHT, enemies[i].x, enemies[i].y,
							PLANE_WIDTH, PLANE_HEIGHT)) {
						// Hiển thị "Game Over"
						show_game_over_screen();
						while (1)
							; // Dừng game tại đây
					}
				}

				check_bullet_enemy_collision();
//			if (plane_move_flag) {
//				plane_move_flag = 0;
//				int old_y = plane_y;
//				plane_y = (plane_y + 5) % 220;
//				erase_plane(plane_x, old_y);
//				draw_plane(plane_x, plane_y);
//				shoot_bullet();
//			}
				if (plane_move_flag) {
					plane_move_flag = 0;
					shoot_bullet();  // chỉ bắn, không di chuyển
				}

				if (plane_move_left_flag) {
					plane_move_left_flag = 0;
//				int old_x = plane_x;
//				plane_x -= 5;
//				if (plane_x < 0) plane_x = 0;
//				erase_plane(old_x, plane_y);
//				draw_plane(plane_x, plane_y);
					int old_y = plane_y;
					plane_y = (plane_y + 5) % 220;
					erase_plane(plane_x, old_y);
					draw_plane(plane_x, plane_y);
				}

				if (plane_move_right_flag) {
					plane_move_right_flag = 0;
					int old_y = plane_y;
					plane_y = (plane_y - 5) % 220;
					erase_plane(plane_x, old_y);
					draw_plane(plane_x, plane_y);
				}
				if (point >= level * 100) {
					level++;
					current_enemy_count += 2;
					init_enemies();
					point = 0;

					// Hiển thị thông báo level up
					ILI9341_FillScreen(WHITE);
					char msg[30];
					sprintf(msg, "LEVEL %d", level);
					ILI9341_DrawText(msg, FONT3, 50, 120, BLUE, WHITE);
					HAL_Delay(1000);  // Hiển thị 1 giây
					ILI9341_FillScreen(WHITE);  // Dọn lại màn hình
				}
			} else if (level == 3) {
				// Hiển thị thông báo level up

				ILI9341_FillScreen(WHITE);
				char msg[30];
				sprintf(msg, "FINAL BOSS", level);
				ILI9341_DrawText(msg, FONT3, 50, 120, BLUE, WHITE);
				HAL_Delay(2000);  // Hiển thị 1 giây
				ILI9341_FillScreen(WHITE);
				init_boss();
				draw_boss(boss.x, boss.y);
				uint32_t now = HAL_GetTick(); // Lấy thời gian hiện tại (milis)

				while (1) {
					if (now - last_boss_fire_time >= boss_fire_interval) {
						fire_boss_laser();
						last_boss_fire_time = now;
					}
					update_bullets();
					HAL_Delay(10);
					update_boss();
					update_boss_bullets();
					if (check_bullet_boss_collision() == 1) {
						show_victory_screen();
						while (1)
							;  // Dừng trò chơi
					}
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
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI
			| RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 180;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void) {

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
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
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
static void MX_SPI5_Init(void) {

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
	if (HAL_SPI_Init(&hspi5) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI5_Init 2 */

	/* USER CODE END SPI5_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void) {

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
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
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
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_13, GPIO_PIN_RESET);

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

	/*Configure GPIO pins : PD8 PD10 */
	GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pins : PD12 PD13 */
	GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
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

	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
