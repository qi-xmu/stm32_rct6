#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// 自定义头文件
#include <Distance.h> // 超声波测距
#include <Trace.h>    // 循迹
#include <Motor.h>    // 电机控制
#include <Battery.h>  // 电量检测

// 屏幕
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
// *超声波引脚
#define DL_TRIG PC2
#define DL_ECHO PC3
#define DC_TRIG PC0
#define DC_ECHO PC1
#define DR_TRIG PC14
#define DR_ECHO PC15
Dis dl(DL_TRIG, DL_ECHO); // 左超声波
Dis dc(DC_TRIG, DC_ECHO); // 中超声波
Dis dr(DR_TRIG, DR_ECHO); // 右超声波

// *循迹检测
#define LL PA15
#define LC PC10
#define RC PC11
#define RR PC12
Trace t(LL, LC, RC, RR); // 初始化循迹传感器模块

// *左电机 电机检测
#define AL PA2
#define BL PA3
// *右电机 电机检测
#define AR PA4
#define BR PA5
// *左电机
#define PWML PB8  // PWM调速
#define AINL PB10 // 方向控制
#define BINL PB11 // A-B+ 前进 A+B- 后退
// *右电机
#define PWMR PB9
#define AINR PB12
#define BINR PB13

Motor lm(PWML, AINL, BINL, AL, BL); // 初始化左侧电机
Motor rm(PWMR, AINR, BINR, AR, BR); // 初始化右侧电机

Battery bat; // 初始化电池对象
// *初始化速度参数 范围 0 ~ 255
int16_t speedl = 40;
int16_t speedr = speedl * 1.05;  // 右电机补偿
float Kp = 8, Ki = 0.01, Kd = 40; // PID参数
const float MAXI = 30;           // 积分最大值
float P = 0, I = 0, D = 0;       // 比例, 积分, 微分
float pid_val = 0;               // PID修正值
float error = 0, pre_error = 0;  // 误差值, 前误差, 误差积分

void calc_pid(); // 计算PID函数

int num = 0;
void num_add()
{
  num++;
}

void setup()
{
  // *屏幕初始化
  Serial.begin(9600);
  Serial.println("ultrasonic sensor");
  while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("SSD1306 allocation failed");
    delay(1000);
  }
  delay(1000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  attachInterrupt(PB1, num_add, FALLING);
}

void loop()
{
  display.clearDisplay();
  // !修正部分
  error = t.get_state(); // 采集误差
  calc_pid();
  // *PID速度修正
  lm.forward(speedl + pid_val);
  rm.forward(speedr - pid_val);
  // !显示部分
  // *显示时间
  display.setCursor(0, 0);
  display.print("TIME:");
  display.print(millis() / 1000);
  display.println("s");
  // *显示里程数
  display.print("RUN:");
  display.print(num * (21.2 / 20));
  display.println("mm");

  // *显示速度信息
  // display.print("LMS:"); // 左轮速度
  // display.print(lm.speed());
  // display.print(" RMS:"); // 右轮速度
  // display.println(rm.speed());
  // *pid参数+优化
  display.print("PID-VAL:");
  display.println(pid_val);

  // // *显示电量
  // display.print("BAT:");
  // display.print(bat.get_bat());
  // display.print("%");

  display.display();
}

// PID算法
void calc_pid()
{
  P = error;             // 比例
  I += error * 0.2;      // 积分
  D = error - pre_error; // 微分

  pre_error = error;
  I = (I < -MAXI) ? -MAXI : I;
  I = (I > MAXI) ? MAXI : I;
  pid_val = (Kp * P) + (Ki * I) + (Kd * D);
}