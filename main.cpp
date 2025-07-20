#include <iostream>
#include <graphics.h>
#include <vector>
#include <fstream>
#include <string>
#include <time.h>
#include <chrono>
#include <thread>
#include <iomanip>
using namespace std;
using namespace std::chrono;

// 通关画面按钮坐标
const int WIN_INCREASE_DIFF_BTN_X = 120;//难度增加按钮X坐标
const int WIN_INCREASE_DIFF_BTN_Y = 500;//Y坐标
const int WIN_EXIT_BTN_X = 300;//退出按钮X坐标
const int WIN_EXIT_BTN_Y = 500;//Y坐标
const int WIN_BUTTON_WIDTH = 180;//按钮宽度
const int WIN_BUTTON_HEIGHT = 60;//按钮高度

// 游戏内按钮位置
const int CONTINUE_BTN_X = 150;//继续按钮X坐标
const int CONTINUE_BTN_Y = 450;//Y坐标
const int INCREASE_DIFF_BTN_X = 150;//难度增加按钮X坐标
const int INCREASE_DIFF_BTN_Y = 520;//Y坐标

IMAGE imgContinue;//继续按钮图片
IMAGE imgIncreaseDiff;//难度增加按钮图片
bool isWaitingClick = false;//是否等待点击
bool useIncreasedDifficulty = false;//是否使用加难模式

// 失败按钮位置
const int RESTART_BTN_X = 50 + 150;///重试按钮X坐标
const int RESTART_BTN_Y = 280 + 200;//Y坐标
const int RESTART_BTN_WIDTH = 300;//按钮宽度
const int RESTART_BTN_HEIGHT = 80;//按钮高度

double actualTimeUsed = 0.0;// 实际用时

class LevelTimer {
public:
    LevelTimer() : timeRemaining(0), timeLimit(0), isRunning(false) {}
    void Start(double limit) {// 开始计时
        timeLimit = limit;
        timeRemaining = timeLimit;
        lastTime = high_resolution_clock::now();
        isRunning = true;
        startTime = high_resolution_clock::now();
    }

    void Update() {// 更新计时
        if (!isRunning) return;
        auto now = high_resolution_clock::now();
        auto elapsed = duration_cast<milliseconds>(now - lastTime).count() / 1000.0;
        timeRemaining = max(timeRemaining - elapsed, 0.0);
        lastTime = now;

        // 更新实际用时
        auto duration = duration_cast<milliseconds>(now - startTime);
        actualTimeUsed = duration.count() / 1000.0;
    }

    bool IsTimeout() const { return timeRemaining <= 0; }// 是否超时
    double GetRemainingTime() const { return timeRemaining; }// 获取剩余时间
    double GetTimeLimit() const { return timeLimit; }// 获取时间限制
    void Pause() { isRunning = false; }// 暂停计时

    void Resume() {// 继续计时
        auto now = high_resolution_clock::now();
        lastTime = now;
        isRunning = true;
    }

    // 添加方法：获取实际用时
    double GetActualTimeUsed() {
        if (isRunning) {
            auto now = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(now - startTime);
            return duration.count() / 1000.0;
        }
        return actualTimeUsed;
    }

private:
    double timeRemaining;// 剩余时间
    double timeLimit;// 时间限制
    bool isRunning;// 是否正在计时
    high_resolution_clock::time_point lastTime;// 上次更新时间
    high_resolution_clock::time_point startTime; // 添加开始时间记录
};



typedef struct { int x, y; } Pos;// 坐标结构体
const auto MAX_LEVEL = 2;//最大关卡数
const auto GROUP_PER_LEVEL = 20;//初始每关卡分组数

const auto WINDOW_WIDTH = 600;//窗口宽度
const auto WINDOW_HEIGHT = 800;//窗口高度
const auto BLOCK_WIDTH = 50;//方块宽度
const auto BLOCK_KINDS = 10;//方块种类数
int cntGroup = 0;//当前分组数
int Level = 1;//当前关卡数

IMAGE imgBg, imgst, imgStart;//图片
IMAGE imgWin, imgLose, imgOut;
IMAGE imgBlock[BLOCK_KINDS + 1];
IMAGE imgGray[BLOCK_KINDS + 1];

vector<int> stack;// 牌堆
vector<Pos> cardpos;// 卡牌位置数组
vector<int> map;// 卡牌类型数组
LevelTimer gameTimer;// 关卡计时器



void ShowCoverPage() {
    IMAGE imgCover;
    loadimage(&imgCover, _T("src/Cover.png"));
    int winWidth = WINDOW_WIDTH;//窗口宽度
    int winHeight = WINDOW_HEIGHT;//窗口高度
    int imgWidth = imgCover.getwidth();
    int imgHeight = imgCover.getheight();

    int x = (winWidth - imgWidth) / 2;//居中X坐标
    int y = (winHeight - imgHeight) / 2;//居中Y坐标

    BeginBatchDraw();//开始批处理绘图
    cleardevice();//清除屏幕
    putimage(x, y, &imgCover);//显示图片
    EndBatchDraw();//结束批处理绘图
    FlushBatchDraw();//刷新批处理绘图

    RECT btnRect = {//按钮矩形
        x + 140,
        y + 220,
        x + 430,
        y + 580
    };

    bool clicked = false;//是否点击开始按钮
    while (!clicked) {
        ExMessage msg;
        if (peekmessage(&msg, EX_MOUSE)) {
            if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= btnRect.left && msg.x <= btnRect.right &&
                    msg.y >= btnRect.top && msg.y <= btnRect.bottom) {
                    clicked = true;
                }
            }
            else if (msg.message == WM_CLOSE) {// 关闭窗口
                closegraph();
                exit(0);
            }
        }
        Sleep(0);
    }
}

void Load_Img() {// 加载图片
    loadimage(&imgBg, _T("src/Bg.png"));
    loadimage(&imgOut, _T("src/Out.png"));
    loadimage(&imgStart, _T("src/Cover.png"));
    loadimage(&imgWin, _T("src/Win.png"));
    loadimage(&imgLose, _T("src/Lose.png"));
    loadimage(&imgContinue, _T("src/Continue.png"));
    loadimage(&imgIncreaseDiff, _T("src/IncreaseDifficulty.png"));
    for (int i = 1; i <= BLOCK_KINDS; i++) {// 加载方块图片
        TCHAR path[256];
        _stprintf_s(path, _T("src/block%d.png"), i);
        loadimage(&imgBlock[i], path);
        _stprintf_s(path, _T("src/Gray%d.png"), i);
        loadimage(&imgGray[i], path);
    }
}

void GameInit();
void Reset();
void InitMap();
void InitCardPositions();
void Update();
int OnClick();
bool IsCardCovered(int index);
void Insert(int index);
void Eliminate();
bool IsWin();

int GameLoop() {// 游戏循环
    GameInit();// 初始化游戏
    while (true) {
        if (!isWaitingClick) {
            gameTimer.Update();
            if (gameTimer.IsTimeout()) {
                actualTimeUsed = gameTimer.GetActualTimeUsed();// 超时时记录实际用时
                return -1;
            }
        }
        ExMessage msg;// 接收消息
        if (peekmessage(&msg, EX_MOUSE)) {
            if (msg.message == WM_CLOSE) {
                closegraph();
                exit(0);
            }
            if (isWaitingClick && msg.message == WM_LBUTTONDOWN) {// 点击开始游戏
                if (msg.x >= CONTINUE_BTN_X &&
                    msg.x <= CONTINUE_BTN_X + imgContinue.getwidth() &&
                    msg.y >= CONTINUE_BTN_Y &&
                    msg.y <= CONTINUE_BTN_Y + imgContinue.getheight()) {
                    isWaitingClick = false;
                    Level++;
                    if (Level > MAX_LEVEL) {
                        // 通关时记录实际用时
                        actualTimeUsed = gameTimer.GetActualTimeUsed();
                        return 1;
                    }
                    Reset();
                    GameInit();
                    gameTimer.Resume();
                    continue;
                }
                if (msg.x >= INCREASE_DIFF_BTN_X &&// 难度增加按钮
                    msg.x <= INCREASE_DIFF_BTN_X + imgIncreaseDiff.getwidth() &&
                    msg.y >= INCREASE_DIFF_BTN_Y &&
                    msg.y <= INCREASE_DIFF_BTN_Y + imgIncreaseDiff.getheight()) {

                    useIncreasedDifficulty = true;
                    putimage(50, 600, &imgIncreaseDiff);
                    EndBatchDraw();
                    Sleep(500);
                }
            }

            if (!isWaitingClick && msg.message == WM_LBUTTONDOWN) {
                for (int i = 0; i < cntGroup * 3; i++) {
                    if (map[i] > 0 && !IsCardCovered(i)) {
                        if (msg.x >= cardpos[i].x && msg.x < cardpos[i].x + BLOCK_WIDTH &&
                            msg.y >= cardpos[i].y && msg.y < cardpos[i].y + BLOCK_WIDTH) {
                            Insert(i);
                            Eliminate();

                            if (stack.size() == 7) {
                                actualTimeUsed = gameTimer.GetActualTimeUsed(); // 失败时记录实际用时
                                return 0;
                            }
                            if (IsWin()) {
                                actualTimeUsed = gameTimer.GetActualTimeUsed();// 胜利时记录实际用时
                                if (Level == MAX_LEVEL) return 1;
                                else {
                                    isWaitingClick = true;
                                    gameTimer.Pause();// 暂停计时器
                                }
                            }
                        }
                    }
                }
            }
        }
        Update();
        Sleep(10);
    }
}

double CalculateLevelTimeLimit() {//时间机制
    if (useIncreasedDifficulty)    return 30.0 + (Level - 1) * 5.0;
    else   return 40.0 + (Level - 1) * 5.0;

}

void GameInit() {// 初始化游戏
    cntGroup = 20 + (int)(10 * sqrt(Level - 1));// 第一关卡分组数为20，之后每关增加10
    Reset();// 重置游戏
    InitMap();
    InitCardPositions();
    double timeLimit = CalculateLevelTimeLimit();
    gameTimer.Start(timeLimit);
}

void Reset() {// 重置游戏 clear所有
    stack.clear();
    cardpos.clear();
    map.clear();
}

void InitMap() {//随机生成卡牌
    srand(time(0));// 随机种子
    int kind = min(5 + Level, 10);// 最多5种，最多10种
    for (int i = 0; i < cntGroup; i++) {// 随机生成卡牌
        int type = 1 + i % kind;
        for (int j = 0; j < 3; j++)
            map.emplace_back(type);
    }
    int cnt = cntGroup * 3, t;// 牌堆大小
    for (int i = 0; i < cntGroup * 3; i++) {// 随机打乱牌堆
        int index = rand() % cnt;
        t = map[i];
        map[i] = map[index];
        map[index] = t;
    }
}

void InitCardPositions() {// 随机生成卡牌位置
    vector<Pos> occupied;
    for (int i = 0; i < cntGroup * 3; i++) {
        Pos temp;
        bool overlap;
        int attempts = 0;
        do {
            overlap = false;
            temp.x = 50 + rand() % (WINDOW_WIDTH - BLOCK_WIDTH - 100);
            temp.y = 150 + rand() % (500);

            for (auto& pos : occupied) {
                if (abs(temp.x - pos.x) < BLOCK_WIDTH + 5 &&
                    abs(temp.y - pos.y) < BLOCK_WIDTH + 5) {
                    overlap = true;
                    break;
                }
            }
            attempts++;
        } while (overlap && attempts < 100);

        occupied.push_back(temp);
        cardpos.emplace_back(temp);
    }
}

void Update() {// 更新游戏画面：显示背景、卡牌、牌堆、时间限制、剩余时间、按钮
    BeginBatchDraw();
    putimage(0, 0, &imgBg);

    TCHAR level[10];
    _stprintf_s(level, _T("%d"), Level);// 关卡数
    settextstyle(50, 0, _T("Consolas"));
    outtextxy(350, 43, level);
    settextstyle(20, 10, _T("Consolas"));
    settextcolor(0);

    TCHAR limitText[50];// 时间限制显示
    _stprintf_s(limitText, _T("时间限制: %.1fs"), gameTimer.GetTimeLimit());
    outtextxy(30, 100, limitText);
    if (gameTimer.GetRemainingTime() < 10.0)   settextcolor(RED);
    else   settextcolor(0);

    TCHAR timeText[50];// 剩余时间显示
    _stprintf_s(timeText, _T("剩余时间: %.1fs"), gameTimer.GetRemainingTime());
    outtextxy(50, 120, timeText);

    settextcolor(0);// 添加实际用时显示
    TCHAR actualTimeText[50];
    _stprintf_s(actualTimeText, _T("实际用时: %.1fs"), actualTimeUsed);
    outtextxy(400, 120, actualTimeText);

    for (int i = 0; i < cntGroup * 3; i++) {
        if (map[i] <= 0) continue;// 跳过空卡牌
        if (IsCardCovered(i)) putimage(cardpos[i].x, cardpos[i].y, &imgGray[map[i]]);// 卡牌被覆盖时显示灰色方块
        else putimage(cardpos[i].x, cardpos[i].y, &imgBlock[map[i]]);// 显示方块
    }

    int cnt = stack.size();
    for (int i = 0; i < cnt; i++)
        putimage(95 + i * 60, 700, &imgBlock[-map[stack[i]]]);// 显示牌堆
    if (isWaitingClick && Level < MAX_LEVEL) { putimage(50, 280, &imgContinue); } // 显示继续按钮
    EndBatchDraw();
}

bool IsCardCovered(int index) {// 判断卡牌是否被覆盖：卡牌被覆盖则不能被选中
    for (int i = index + 1; i < cntGroup * 3; i++) {
        if (map[i] > 0) {
            if (cardpos[index].x + BLOCK_WIDTH > cardpos[i].x &&
                cardpos[index].x < cardpos[i].x + BLOCK_WIDTH &&
                cardpos[index].y + BLOCK_WIDTH > cardpos[i].y &&
                cardpos[index].y < cardpos[i].y + BLOCK_WIDTH)// 卡牌被覆盖
                return true;
        }
    }
    return false;
}

void Insert(int index) {// 插入卡牌：将卡牌插入牌堆
    vector<int> temp;
    for (auto i : stack) {
        temp.emplace_back(i);
        if (map[i] == -map[index]) {
            temp.emplace_back(index);
            map[index] *= -1;
        }
    }
    if (stack.size() == temp.size()) {// 牌堆未发生变化
        temp.emplace_back(index);
        map[index] *= -1;
    }
    stack = temp;
}

void Eliminate() {// 卡牌消除：消除相同颜色的3张牌
    int cnt[BLOCK_KINDS + 1] = { 0 };
    for (auto i : stack) cnt[-map[i]]++;
    for (int i = 1; i <= BLOCK_KINDS; i++) {
        if (cnt[i] >= 3) {
            vector<int> temp;
            for (auto j : stack)
                if (-map[j] != i) temp.emplace_back(j);
                else map[j] = 0;
            stack = temp;
            break;
        }
    }
}
// 判断游戏是否胜利：所有卡牌都消除
bool IsWin() {
    for (auto i : map)
        if (i != 0)
            return false;
    return true;
}

int main() {
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    setbkmode(TRANSPARENT);
    Load_Img();
    useIncreasedDifficulty = false;
    while (true) {
        ShowCoverPage();// 显示封面页面，等待用户点击开始游戏
        actualTimeUsed = 0.0; // 重置实际用时
        Level = 1;
        isWaitingClick = false;

        int result = GameLoop();
        cout << "本次挑战时间限制是：" << CalculateLevelTimeLimit() << " 秒  ";
        // 输出本次挑战的实际用时（保留1位小数）
        cout << "本次挑战实际用时: " << fixed << setprecision(1) << actualTimeUsed << "秒 " << endl;

        if (result == 1) {// 通关
            putimage(50, 280, &imgWin);
            FlushBatchDraw();
            bool restartWithDifficulty = false;
            while (true) {
                ExMessage msg;
                if (peekmessage(&msg, EX_MOUSE)) {// 处理用户点击事件：重新开始或退出
                    if (msg.message == WM_LBUTTONDOWN) {// 重新开始按钮
                        if (msg.x >= WIN_EXIT_BTN_X &&
                            msg.x <= WIN_EXIT_BTN_X + WIN_BUTTON_WIDTH &&
                            msg.y >= WIN_EXIT_BTN_Y &&
                            msg.y <= WIN_EXIT_BTN_Y + WIN_BUTTON_HEIGHT) {
                            closegraph();
                            exit(0);
                        }
                        if (msg.x >= WIN_INCREASE_DIFF_BTN_X &&// 难度增加按钮
                            msg.x <= WIN_INCREASE_DIFF_BTN_X + WIN_BUTTON_WIDTH &&
                            msg.y >= WIN_INCREASE_DIFF_BTN_Y &&
                            msg.y <= WIN_INCREASE_DIFF_BTN_Y + WIN_BUTTON_HEIGHT) {
                            useIncreasedDifficulty = true;
                            restartWithDifficulty = true;
                            break;
                        }
                    }
                    else if (msg.message == WM_CLOSE) {// 关闭窗口
                        closegraph();
                        exit(0);
                    }
                }
                Sleep(10);
            }

            if (restartWithDifficulty) {
                int imgX = (WINDOW_WIDTH - imgIncreaseDiff.getwidth()) / 2;// 居中X坐标
                int imgY = (WINDOW_HEIGHT - imgIncreaseDiff.getheight()) / 2;
                BeginBatchDraw();
                cleardevice();
                putimage(imgX, imgY, &imgIncreaseDiff);
                EndBatchDraw();
                FlushBatchDraw();
                cout << "高难度模式已激活！" << endl;
                cout << "下次挑战时间限制: " << fixed << setprecision(1)
                    << CalculateLevelTimeLimit() << "秒" << endl;
                Sleep(2000); // 显示2秒后自动开始下一轮挑战
                continue;
            }
        }
        else if (result == 0) {// 失败
            putimage(50, 280, &imgLose);
            FlushBatchDraw();
            bool restartClicked = false;
            while (!restartClicked) {
                ExMessage msg;
                if (peekmessage(&msg, EX_MOUSE)) {// 处理用户点击事件：重新开始或退出
                    if (msg.message == WM_LBUTTONDOWN) {
                        if (msg.x >= RESTART_BTN_X &&
                            msg.x <= RESTART_BTN_X + RESTART_BTN_WIDTH &&
                            msg.y >= RESTART_BTN_Y &&
                            msg.y <= RESTART_BTN_Y + RESTART_BTN_HEIGHT) {
                            restartClicked = true;
                        }
                    }
                    else if (msg.message == WM_CLOSE) {
                        closegraph();
                        exit(0);
                    }
                }
                Sleep(10);
            }
        }
        else if (result == -1) {// 超时
            putimage(50, 280, &imgOut);
            FlushBatchDraw();
            cout << "挑战超时！实际用时: " << fixed << setprecision(2)
                << actualTimeUsed << "秒" << endl;
            Sleep(1500);
            break;
        }
    }
    closegraph();
    return 0;
}