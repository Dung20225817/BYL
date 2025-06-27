🛩️ Plane Shooting Game on STM32F429 with ILI9341
Một dự án game bắn máy bay đơn giản được lập trình trên dòng vi điều khiển STM32F429, sử dụng màn hình LCD SPI chuẩn ILI9341. Người chơi điều khiển máy bay, bắn đạn tiêu diệt boss (hình nhện hoặc quái vật), tránh đạn từ boss để giành chiến thắng.

🛠️ Công cụ phát triển
Công cụ	Phiên bản được khuyến nghị
STM32CubeIDE	v1.14.0 trở lên
STM32CubeMX	v6.10.0 trở lên
HAL Driver STM32F4	v1.25.0 trở lên
Compiler (GCC)	GCC for ARM Embedded
Git	Git CLI hoặc GitHub

📦 Cài đặt & Build
Clone repository:
git clone https://github.com/your-username/stm32-plane-shooter.git
Mở project bằng STM32CubeIDE:
File → Open Projects from File System...
Chọn thư mục project vừa clone

Kết nối phần cứng:
kết nối 2 nút bấm với các chân PD10 và PD2

Build và Flash:

Click icon 🛠️ để build

Kết nối ST-Link và nhấn "Run" để nạp firmware

🎮 Điều khiển
Hành động	Mô tả
Máy bay bắn đạn	Tự động bắn theo chu kỳ
Máy bay địch	Tăng dần số lượng theo mỗi màn chơi
Boss bắn ra nhiều kiểu	shotgun, burst, wave
Kết thúc	Thắng nếu boss bị tiêu diệt
