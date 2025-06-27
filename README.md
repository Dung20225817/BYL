# 🛩️ Plane Shooting Game on STM32F429 with ILI9341

Một dự án game bắn máy bay đơn giản được lập trình trên dòng vi điều khiển **STM32F429**, sử dụng màn hình LCD SPI chuẩn **ILI9341**.  
Người chơi điều khiển máy bay, bắn đạn tiêu diệt boss (hình nhện hoặc quái vật), tránh đạn từ boss để giành chiến thắng.

---

## 🛠️ Công cụ phát triển

| Công cụ             | Phiên bản được khuyến nghị       |
|---------------------|----------------------------------|
| STM32CubeIDE        | v1.14.0 trở lên                  |
| STM32CubeMX         | v6.10.0 trở lên                  |
| STM32 HAL Driver F4 | v1.25.0 trở lên                  |
| Compiler            | GCC for ARM Embedded             |
| Git                 | Git CLI hoặc GitHub              |

---

## 📦 Cài đặt & Build

1. **Clone repository**:
   ```bash
   git clone https://github.com/Dung20225817/BYL.git
2. **Mở project bằng STM32CubeIDE**:

- File → Open Projects from File System... 
- Chọn thư mục project vừa clone
- Kết nối ST-Link và nhấn "Run" để nạp firmware
## 🎮 Điều khiển
|   Hành động	  |            Mô tả                        |
|----------------|-----------------------------------------|
|Máy bay bắn đạn |Tự động bắn theo chu kỳ, điều khiển bằng nút bấm           |
|Máy bay địch	  |  Tăng dần số lượng theo mỗi màn chơi    |
|Boss            |Bắn ra nhiều kiểu: shotgun, burst, wave  |
|Kết thúc	     |Thắng nếu boss bị tiêu diệt              |
  
