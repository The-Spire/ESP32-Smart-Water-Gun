## 🎬 视频链接
🔗 [B站视频--功能演示及介绍](https://www.bilibili.com/)

---

## 💡 1. 简介
本项目是一款基于 **ESP32-C3** 的 DIY 智能水枪。

外观采用 APEX 中的 R99 刀刃模型。本人非专业人士，所有知识皆为现学，项目已验证为可用，但可能存在 BUG 和不规范设计，请谨慎学习。错误之处请在评论区指出，我会及时修正。

> 🚨 **[本项目为安全声光玩具，只能滋水，不能发射子弹！]**

---

## 📜 2. 描述
本项目还原了游戏中的屏幕显示、烟雾和射击效果。另外加入了水泵、激光器、音效系统，并开发了完整的 UI 设置界面。



<div align="center">
<img src="https://github.com/user-attachments/assets/019ecceb-e8ff-4b05-bcc4-1fd5ef4be343" width="80%" />
<br /><br />
<img src="https://github.com/user-attachments/assets/f81a58e5-b63a-49d0-84ca-5a3a1c421247" width="80%" />
<br /><br />
<img width="450" height="220" alt="R99白天效果" src="https://github.com/user-attachments/assets/c3323576-fb1f-45bf-824b-d2e1c1d175d4" />
   <br /><br />
<img width="400" height="300" alt="R99水雾效果" src="https://github.com/user-attachments/assets/4d64fe4c-7a8c-4507-b691-a37cf65473fa" />
<img width="400" height="300" alt="R99夜间效果" src="https://github.com/user-attachments/assets/fad40eb7-a437-4e98-85ee-deaff1d403e6" />

</div>

---

## 🧩 3. 结构模型
🔗 **模型开源地址：** https://makerworld.com.cn/zh/my/models/drafts/25847941/edit

---

## 🛠️ 4. 硬件架构
🔗 **硬件电路开源地址：**

<img src="https://github.com/user-attachments/assets/da59e916-b238-45a4-8e91-a446cfbb76f0" width="80%" />
<br /><br />

* 🧠 **MCU：** ESP32-C3
* 📺 **屏幕：** 0.95寸彩色OLED
* 🕹️ **交互：** 编码器输入
* ⚙️ **外设：** 水泵、加湿器、ws2812灯带、语音模块
* 🔋 **供电：** 18650圆柱形锂电池*2、双向快充模块


---

## 💻 5. 软件架构
**💻 软件配置：**
* **IDE：** VScode + platformio
* **框架：** Arduino

🔗 **源代码开源地址：** https://github.com/The-Spire/ESP32-Smart-Water-Gun

### 📂 目录结构
```text
src/
├── assets/          # 静态资源 (字体 .c, 图片 .c)
├── hardware/        # 硬件驱动层 (Input, Output, LedStrip, Voice)
├── logic/           # 业务逻辑层 (核心状态机 WaterGunLogic)
├── storage/         # 数据持久化层 (SettingsManager)
├── ui/              # UI 表现层
│   ├── screens/     # 各个独立子屏幕的实现 (ScreenMain, ScreenFunc...)
│   └── UIManager    # UI 总调度器
├── config.h         # 全局引脚和参数配置
└── main.cpp         # 程序入口，主循环
```

### ⚙️ 设置系统
* **操作逻辑**: 通过旋转编码器导航，短按扳机进入/编辑，长按扳机保存并返回。
<img src="https://github.com/user-attachments/assets/9a84151d-c258-4d61-bf87-12d102303147" width="80%" />
<br /><br />

1. 🔫 **子弹设定**:
    * **子弹类型**: 切换 水/激光。
    * **最大载弹量**: 15-99发可调。
    * **射击/换弹速度**: 1-99 可调 。
    * **换弹延迟**: 自动换弹前的等待时间。

2. 📊 **系统信息**:
    * **性能监视器**: 开关显示 FPS (帧率), RAM (内存占用), L (主循环耗时 us)。
    * **开发者信息**: 滚动查看Logo和开发者名称（具备特殊的列表自动上滚交互）。

3. 🎨 **主题设定**:
    * **双色主题**: 可分别定义“常规色”（文字）和“强调色”（焦点条/图标背景）。
    * **全彩拾取器**: 使用Canvas绘制的彩虹渐变条，支持平滑颜色选择。
    * **实时预览**: 调整颜色时，全系统UI颜色实时生效。

4. 🎵 **声音设定**:
    * **音效开关**: 一键静音。
    * **音量调节**: 0-30级调节，调节时实时反馈音量大小。

5. 💡 **灯光设定**:
    * **呼吸灯速度**: 控制灯带流水速度。
    * **呼吸灯颜色**: 全彩调节灯带主色调，支持实时预览。

6. 🧰 **功能设定**:
    * **硬件开关**: 独立控制水泵、加湿器、灯光、激光器的使能状态。
    * **自动待机**: 设定无操作 X 分钟后自动休眠（0为禁用）。
7. 💡 **666**:
    * **动画**: 进入后会播放一键三连动画。
---

## ⚠️ 6. 注意事项

* FDM打印的模型存在漏水现象，提高打印温度或使用光固化打印可以改善
* 由于模型内部有大量水汽，可能会干扰电路正常运作，建议在电路板上喷涂三防漆
* 系统目前无法实现自动关机，只能休眠降低功耗，长时间不玩建议拔下弹匣
* 模型内部的水箱容量很小，只够加湿器使用，同时喷水的话几下就没水了。我另外设计了水瓶转接器，可以外接瓶子，但是这样会降低颜值...
* 激光枪模式还没有对激光标靶做适配，靶子我还没买，激光标靶需要特定的载波激光触发，单片机理论上可以调制，后边实现了的话会更新到程序中。

---

## 🔧 7. 组装说明

🚧 组装说明施工中...近期更新
