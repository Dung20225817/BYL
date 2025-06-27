# 🛩️ Plane Shooting Game on STM32F429 with ILI9341

Một dự án game bắn máy bay đơn giản được lập trình trên dòng vi điều khiển **STM32F429**, sử dụng màn hình LCD SPI chuẩn **ILI9341**.  
Bạn là một tu sĩ trẻ tuổi của phái Thái Hư Kiếm Tông, người được truyền thụ phi kiếm thần thông hiếm có trong tam giới. Trong lần bế quan ngộ đạo, thiên tượng dị biến, linh khí hỗn loạn, yêu tà từ Dị Giới lợi dụng khe nứt không gian tràn vào nhân gian.

Nhận sứ mệnh từ chưởng môn, bạn cưỡi phi kiếm, xuyên qua ba tầng kết giới để trấn áp dị loại. Trên đường đi, từng đợt dị thú, tà linh, và yêu vật kéo đến tấn công liên tục. Chỉ cần trụ vững một khoảng thời gian, Linh Trận Hộ Thể sẽ khai mở, cho phép bạn dịch chuyển đến cửa tiếp theo.

Qua ba tầng kết giới, bạn đối mặt với Ma Vương Bát Túc – một con Ác Quỷ cổ xưa bị phong ấn ngàn năm dưới Vô Danh Cốc. Hắn điều khiển huyết khí tà đạn, tấn công dồn dập mọi phía.

🔸 Để giành chiến thắng:

Luân phiên dùng Phi Kiếm tấn công

Tránh các loại chiêu thức yêu pháp như: Huyết Trảo, Tà Châm, và Huyễn Vụ

Tiêu diệt Boss trước khi pháp lực cạn kiệt!

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
2. **Kết nối phần cứng**:
- Kết nối KIT STM32f429zit6 với các nút bấm bằng chân PD8 và PD10
3. **Mở project bằng STM32CubeIDE**:
- File → Open Projects from File System... 
- Chọn thư mục project vừa clone
- Kết nối ST-Link và nhấn "Run" để nạp firmware
## 🎮 Điều khiển
|   Hành động	  |            Mô tả                        |
|----------------|-----------------------------------------|
|Máy bay bắn đạn |Tự động bắn theo chu kỳ, điều khiển bằng nút bấm           |
|Máy bay địch	  |  Tăng dần số lượng theo mỗi màn chơi    |
|Boss            |Bắn đạn nhiều kiểu: shotgun, burst, wave  |
|Kết thúc	     |Thắng nếu boss bị tiêu diệt              |
  
