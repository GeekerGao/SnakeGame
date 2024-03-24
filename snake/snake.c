#include <curses.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define UP     1
#define DOWN  -1
#define LEFT   2
#define RIGHT -2   

// 定义贪吃蛇的结构体
struct Snake
{
        int row;
        int column;
        struct Snake *next;
};

struct Snake *head = NULL; // 蛇头指针
struct Snake *tail = NULL; // 蛇尾指针
struct Snake food; // 食物的位置

int key; // 保存按键值
int dir; // 蛇的当前方向

// 初始化食物位置的函数
void initFood()
{
        srand((unsigned)time(NULL)); // 用时间作为随机数生成的种子
        int x = rand() % 15 + 3; // 随机生成x坐标
        int y = rand() % 15 + 3; // 随机生成y坐标

        food.row    = x;
        food.column = y;
}

// 检查某个位置是否有蛇身的函数
int allSnakeNode(int i, int j)
{
        struct Snake *p = head;

        while(p != NULL){
                if(p->row == i && p->column == j){
                        return 1; // 该位置有蛇身
                }
                p = p->next;
        }
        return 0; // 该位置没有蛇身
}

// 检查某个位置是否有食物的函数
int setFood(int i, int j)
{
        return (food.row == i && food.column == j) ? 1 : 0;
}

// 绘制游戏地图的函数
void gameMap()
{
        clear(); // 在渲染新帧之前清除屏幕
        int row, column;
        move(0,0); // 将光标移动到屏幕左上角

        for(row = 0; row < 20; row++){
                if(row == 0 || row == 19){
                        for(column = 0; column < 20; column++){
                                printw("--"); // 绘制上下边界
                        }
                        printw("\n");
                } else {
                        for(column = 0; column <= 20; column++){
                                if(column == 0 || column == 20){
                                        printw("|"); // 绘制左右边界
                                } else if(allSnakeNode(row, column)){
                                        printw("[]"); // 绘制蛇身
                                } else if(setFood(row, column)){
                                        printw("##"); // 绘制食物
                                } else {
                                        printw("  "); // 绘制空白区域
                                }
                        }
                        printw("\n");
                }
        }
        printw("Made by ZBB, key=%d, food.row=%d, food.column=%d\n", key, food.row, food.column); // 打印游戏信息
}

// 向蛇身添加节点的函数
void addNode()
{
        struct Snake *new = (struct Snake *)malloc(sizeof(struct Snake));
        new->next = NULL;
        new->row = tail->row;
        new->column = tail->column;

        switch(dir){
                case UP:
                        new->row -= 1;
                        break;
                case DOWN:
                        new->row += 1;
                        break;
                case LEFT:
                        new->column -= 1;
                        break;
                case RIGHT:
                        new->column += 1;
                        break;
        }

        tail->next = new;
        tail = new;
}

// 初始化贪吃蛇的函数
void initSnake()
{
        while(head != NULL){
                struct Snake *p = head;
                head = head->next;
                free(p);
        }

        dir = RIGHT; // 设置蛇的初始方向为右
        initFood(); // 初始化食物位置

        head = (struct Snake *)malloc(sizeof(struct Snake)); // 创建蛇头节点
        head->row = 3;
        head->column = 4;
        head->next = NULL;

        tail = head; // 初始时蛇头即蛇尾

        // 初始状态下蛇身长度为5
        for(int i = 0; i < 4; i++){
                addNode();
        }
}

// 删除蛇尾节点的函数
void deleteNode()
{
        if(head != NULL){
                struct Snake *p = head;
                head = head->next;
                free(p);
        }
}

// 检查蛇是否死亡的函数
int snakeDie()
{
        if(tail->row < 1 || tail->row >= 19 || tail->column < 1 || tail->column >= 20)
        {
                return 1; // 蛇撞墙死亡
        }

        struct Snake *p = head;
        while(p != tail){
                if(p->row == tail->row && p->column == tail->column){
                        return 1; // 蛇撞到自己死亡
                }
                p = p->next;
        }
        return 0;
}

// 控制蛇移动的函数
void moveSnake()
{
        addNode(); // 向蛇身添加节点，即前进一格

        if(setFood(tail->row, tail->column)){
                initFood(); // 如果蛇吃到食物，重新生成食物
        } else {
                deleteNode(); // 如果蛇没有吃到食物，删除蛇尾节点
        }

        if(snakeDie()){
                initSnake(); // 如果蛇死亡，重新初始化蛇
        }
}

// 刷新窗口的线程函数
void* refreshWindow(void* arg)
{
        while(1)
        {
                moveSnake(); // 移动蛇
                gameMap(); // 绘制游戏地图
                refresh(); // 刷新屏幕
                usleep(105000); // 控制游戏速度
        }
}

// 改变蛇移动方向的函数
void turn(int direction)
{
        if(abs(dir) != abs(direction)){
                dir = direction;
        }
}

// 处理按键改变方向的线程函数
void* changeDir(void* arg)
{
        while(1)
        {
                key = getch(); // 获取按键值
                switch(key)
                {
                        case KEY_DOWN:
                                turn(DOWN); // 向下移动
                                break;
                        case KEY_UP:
                                turn(UP); // 向上移动
                                break;
                        case KEY_LEFT:
                                turn(LEFT); // 向左移动
                                break;
                        case KEY_RIGHT:
                                turn(RIGHT); // 向右移动
                                break;
                }
        }
}

// 程序入口点
int main()
{
        pthread_t t1, t2;

        initscr(); // 初始化ncurses
        keypad(stdscr, TRUE); // 启用键盘
        noecho(); // 关闭回显
        curs_set(0); // 隐藏光标

        initSnake(); // 初始化蛇
        gameMap(); // 绘制游戏地图

        pthread_create(&t1, NULL, refreshWindow, NULL); // 创建刷新窗口线程
        pthread_create(&t2, NULL, changeDir, NULL); // 创建处理按键线程
        
        pthread_join(t1, NULL); // 等待线程结束
        pthread_join(t2, NULL);

        getch(); // 等待用户输入以退出
        endwin(); // 结束ncurses模式

        return 0;
}


