# flag越来越多

对！flag一多，全局变量到处堆，代码乱、难维护，这是初学写单片机非常典型的痛点。 核心思路：**不要一堆独立flag变量，用一个uint8_t位标志变量 + 状态机，把所有事件打包在一起**，就是刚才跟你说的位操作。

## 1. 统一一个标志寄存器（只用一个uint8_t）

```
// 只用这一个！所有触发事件都放这里
uint8_t event_flag = 0x00;

// 分配bit（自己规划，记好）
#define KEY1_EVENT    (1 << 0)  // bit0 按键1事件
#define KEY2_EVENT    (1 << 1)  // bit1 按键2事件
#define LIGHT_EVENT   (1 << 2)  // bit2 光敏DO边沿事件
```

### 回调里面，只置对应的bit，不再新建flag

```
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == KEY1_Pin)
    {
        event_flag |= KEY1_EVENT;
    }
    if(GPIO_Pin == KEY2_Pin)
    {
        event_flag |= KEY2_EVENT;
    }
    if(GPIO_Pin == LightSensor_D0_Pin)
    {
        event_flag |= LIGHT_EVENT;
    }
}
```

✅ 好处：**不再新增一堆key_flag、light_flag**，就一个event_flag。

## 2. 主循环里面，用if或者switch依次检查、处理、清bit

```
while(1)
{
    // 按键1处理
    if(event_flag & KEY1_EVENT)
    {
        HAL_Delay(20);
        if(HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == 0)
        {
            // 按键1业务
        }
        event_flag &= ~KEY1_EVENT; // 单独清除这一位，其他标志不动
    }

    // 按键2处理
    if(event_flag & KEY2_EVENT)
    {
        HAL_Delay(20);
        if(HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == 0)
        {
            // 按键2业务
        }
        event_flag &= ~KEY2_EVENT;
    }

    // 光敏边沿事件
    if(event_flag & LIGHT_EVENT)
    {
        HAL_Delay(20);
        uint8_t light_now = HAL_GPIO_ReadPin(LightSensor_D0_GPIO_Port, LightSensor_D0_Pin);
        static uint8_t light_last = 1;
        if(light_now != light_last)
        {
            if(light_now == GPIO_PIN_RESET)
            {
                // 下降沿：变暗
                HAL_GPIO_WritePin(LED_PB3_GPIO_Port, LED_PB3_Pin, GPIO_PIN_RESET);
            }
            else
            {
                // 上升沿：变亮
                HAL_GPIO_WritePin(LED_PB3_GPIO_Port, LED_PB3_Pin, GPIO_PIN_SET);
            }
            light_last = light_now;
        }
        event_flag &= ~LIGHT_EVENT;
    }
}
```

>  重点：`&=` 只清自己那一位，别的bit保留，不会把别的未处理事件直接抹掉。

## 3. 再升级：区分【事件标志】和【状态变量】（非常关键，很多人混在一起）

两类东西不要塞到同一个变量里：

1. **event_flag（事件，一次性、触发信号）**：发生了一次跳变，处理完就清掉 → 按键、光敏边沿、外部脉冲
2. **state（状态，持续保持）**：灯现在是亮还是灭、当前模式 → 单独变量，不随便清零

```
uint8_t run_state = 0; // 0自动模式 1手动模式，属于状态，不是事件
```

❌ 不要把“灯亮不亮”这种持续状态，放到event_flag里。 event_flag只用来表示：**刚刚发生了一件事，等着我去处理一次**。

## 4. 再往后：状态机（以后项目大了用）

事件只是触发信号，真正动作交给状态机：

```
switch(run_state)
{
    case 0:
        // 自动模式
        break;
    case 1:
        // 手动模式
        break;
}
```

事件（event_flag）用来切换run_state，而不是直接写点灯逻辑。

## 5. 额外小规范，防止越堆越多

-  所有EXTI回调，**只做一件事：置event_flag里面对应的bit，别的一律不写**
-  消抖、读电平、业务点灯，全部放到主循环
-  一个bit对应一个独立触发源，8bit最多支持8个外部中断源，足够初学用
-  以后再加传感器、按键，只新增宏定义，不用新增全局变量

## 6. 一个坑提醒

不要在中断里直接清标志！ 中断只置1；**标志清零，统一放在主循环处理完之后**。 避免：事件来了还没处理，就被清掉丢信号。

------

如果你想，我可以给你整理一版带宏、注释、可以直接复制进工程的模板，以后再加外设直接加宏就行，不会再flag满天飞。



# 函数封装

## 堆代码的时候，怎么避免直接变成一团乱麻（不用重构也能维持可读性，低成本做法）

你担心 “越来越复杂失控”，这个担心非常对，但**不一定非要上结构体驱动才算解决**。在现在这个工程里，你每次新增功能，只要遵守几条轻量规则，代价很小：

1. 变量分区严格遵守 CubeMX 的 USER 区块：PV 放全局变量、Private defines 放宏、Private variables 放 extern；不乱放。
2. 每个功能块前面加注释块，写清楚它是干嘛的、用到哪些标志：

```
/* --------------------------
 * 流水灯模块
 * 输入：FlowingLight_run_flag
 * 输出：RGB引脚
 * -------------------------- */
if(FlowingLight_run_flag == 1)
{
    ...
}

/* --------------------------
 * 光敏检测模块
 * 独立运行，不阻塞
 * -------------------------- */
```

1. 不要把所有逻辑全部揉在 while 一层，当 if 越来越多之后，可以拆成独立函数：

```
void Key_Scan(void);
void FlowLight_Run(void);
void LightSensor_Scan(void);
```

while 里面就变得清爽：

```
while(1)
{
    Key_Scan();
    FlowLight_Run();
    LightSensor_Scan();
}
```

👉 这个叫**函数级拆分**，是最轻量的整理，算不上完整重构，但是能大幅缓解混乱，你随时可以做，成本很低。这一步优先于结构体封装。

>  重点区分： 单纯拆成函数 ≠ 重构； 改成结构体驱动、bsp/app 分层才是完整重构。 你现在优先做【拆函数】这个低成本整理，性价比最高。

## 三、重构：什么时候做？要不要做？重构的触发条件（非常实用）

记住：重构的目的是**解决痛苦**，不是为了代码好看而重构。 满足下面任意两条，才值得花时间重构；一条都不满足，完全没必要：

1. 移植痛苦：换引脚、换板子，要在代码里到处搜索修改，很容易改错；
2. 复用需求：这个按键 / 流水灯模块，你接下来要在好几个工程反复使用；
3. 维护痛苦：每次加新功能，一不小心就把旧逻辑搞坏，状态纠缠在一起；
4. 你主动想学驱动分层、结构体封装，当做练习目标（学习目的）。

反过来： 只是课程小 Demo，做完这个工程就归档，不会再反复改、不会移植到别的板子 → **完全不用重构，保持能用就行。**

### 时间线放到你现在身上

当前阶段：

1. 先把当前这套所有功能全部调稳，把所有边界测试完（按键中途停、倒计时、光敏并行）；
2. 如果 while 里面 if 堆太多，**先做轻整理：拆成 Key_Scan、FlowLight_Run、LightSensor_Scan 函数**，这个随时可以做，推荐做；
3. 等你哪天觉得：“每次改引脚好麻烦，我想把 GPIO 打包起来，以后复用”，再去做结构体驱动重构；
4. 如果之后这个工程做完就放下，不打算复用，那结构体重构可以直接跳过。

>  一个很重要的认知：**重构是可选优化，不是代码合格线。能稳定运行、逻辑清晰、注释齐全的裸代码，在学习阶段也是合格的。**

# 硬件解耦

## 方案 1：宏定义封装（最适合你现在这个工程，零门槛，马上改）

放到 main.h 的 `Private defines` 区块（本来就是放 #define 的地方，正好用对分区）

```
/* USER CODE BEGIN Private defines */
// 按键
#define KEY1_PIN    GPIO_PIN_0
#define KEY1_PORT   GPIOA
#define KEY2_PIN    GPIO_PIN_1
#define KEY2_PORT   GPIOA

// 光敏
#define LIGHT_SENSOR_PIN    GPIO_PIN_2
#define LIGHT_SENSOR_PORT   GPIOA

// RGB流水灯
#define LED_R_PIN    GPIO_PIN_3
#define LED_R_PORT   GPIOA
#define LED_G_PIN    GPIO_PIN_4
#define LED_G_PORT   GPIOA
#define LED_B_PIN    GPIO_PIN_5
#define LED_B_PORT   GPIOA

// 单独输出LED
#define LED_OUT_PIN    GPIO_PIN_6
#define LED_OUT_PORT   GPIOA
/* USER CODE END Private defines */
```

然后你业务代码里全部改用宏：

```
if(HAL_GPIO_ReadPin(KEY1_PORT, KEY1_PIN) == GPIO_PIN_SET)
{
    FlowingLight_run_flag = 1;
    count = 0;
    timer_en = 1;
}
```

流水灯：

```
HAL_GPIO_WritePin(LED_R_PORT, LED_R_PIN, GPIO_PIN_RESET);
HAL_GPIO_WritePin(LED_G_PORT, LED_G_PIN | LED_B_PIN, GPIO_PIN_SET);
```

✅ 好处： 以后线换引脚了：**只需要在这一块宏里面改一次，所有业务代码一行不动**。 哪怕 KEY1 从 PA0 换到 PB1，只改：

```
#define KEY1_PIN    GPIO_PIN_1
#define KEY1_PORT   GPIOB
```

>  注意：**CubeMX 这边也要同步配置这个新引脚为输入 / 输出**，这个是绕不开的。宏只是把代码里所有引用收拢到一处，减少改错的概率。 如果你 CubeMX 不重新配置新引脚：RCC 时钟没开、模式不对，改了宏也读 / 写不了。

------

## 方案 2：用结构体封装（进阶，适合模块多、引脚很多的项目）

把一个外设的端口、引脚、有效电平打包成一个结构体，适合做成驱动。

```
// main.h
typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    GPIO_PinState active_level; // 有效电平：按下是高还是低
} GPIO_HandleTypeDef;

// 实例
#define KEY1  ((GPIO_HandleTypeDef){GPIOA, GPIO_PIN_0, GPIO_PIN_SET})
#define KEY2  ((GPIO_HandleTypeDef){GPIOA, GPIO_PIN_1, GPIO_PIN_SET})
```

再封装简单读写函数：

```
static inline GPIO_PinState GPIO_Read(GPIO_HandleTypeDef gpio)
{
    return HAL_GPIO_ReadPin(gpio.port, gpio.pin);
}
static inline void GPIO_Write(GPIO_HandleTypeDef gpio, GPIO_PinState state)
{
    HAL_GPIO_WritePin(gpio.port, gpio.pin, state);
}
```

业务代码就变成：

```
if(GPIO_Read(KEY1) == KEY1.active_level)
```

这个写法的优势：以后做驱动（按键、传感器），可以统一接口，甚至支持多个同类型设备，适合后面写库。

# git分支管理

### 方案 A：一个仓库，多个分支（推荐，适合同一系列实验）

仓库名字比如 `stm32-learning`

-  `main`：放最干净、稳定的基准版本（你现在调好的这一版）
-  `feature/flowlight-5s`：就是你当前这个 5s 倒计时版本
-  `feature/flowlight-uart`：在它基础上加串口的变种
-  `feature/pwm-breath`：呼吸灯独立分支

好处：所有相关实验在一个仓库，方便回看对比；想做变种，直接从 main 分支切出新分支，相当于软复制，不用复制一堆文件。不想保留随时删分支。

### 方案 B：每个独立工程一个独立仓库

适合完全不相关项目：比如一个仓库是 STM32 灯光，另一个是云服务器 demo，另一个是大模型部署 demo。 缺点：仓库变多，管理分散；小实验不推荐。

>  如果你只是复制一份本地文件夹做实验，**不要直接把两个几乎一样的文件夹同时 push 到同一个分支**。要么开分支，要么副本只在本地跑、不提交，验证完觉得有留存价值再整理提交。 提交习惯建议：每调通一个关键功能做一次 commit，写清楚备注：`feat: 增加SysTick 5s自动关闭，修复count不重置bug`。以后回头看，你能看到自己踩坑的全过程。