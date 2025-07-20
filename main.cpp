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

// ͨ�ػ��水ť����
const int WIN_INCREASE_DIFF_BTN_X = 120;//�Ѷ����Ӱ�ťX����
const int WIN_INCREASE_DIFF_BTN_Y = 500;//Y����
const int WIN_EXIT_BTN_X = 300;//�˳���ťX����
const int WIN_EXIT_BTN_Y = 500;//Y����
const int WIN_BUTTON_WIDTH = 180;//��ť���
const int WIN_BUTTON_HEIGHT = 60;//��ť�߶�

// ��Ϸ�ڰ�ťλ��
const int CONTINUE_BTN_X = 150;//������ťX����
const int CONTINUE_BTN_Y = 450;//Y����
const int INCREASE_DIFF_BTN_X = 150;//�Ѷ����Ӱ�ťX����
const int INCREASE_DIFF_BTN_Y = 520;//Y����

IMAGE imgContinue;//������ťͼƬ
IMAGE imgIncreaseDiff;//�Ѷ����Ӱ�ťͼƬ
bool isWaitingClick = false;//�Ƿ�ȴ����
bool useIncreasedDifficulty = false;//�Ƿ�ʹ�ü���ģʽ

// ʧ�ܰ�ťλ��
const int RESTART_BTN_X = 50 + 150;///���԰�ťX����
const int RESTART_BTN_Y = 280 + 200;//Y����
const int RESTART_BTN_WIDTH = 300;//��ť���
const int RESTART_BTN_HEIGHT = 80;//��ť�߶�

double actualTimeUsed = 0.0;// ʵ����ʱ

class LevelTimer {
public:
    LevelTimer() : timeRemaining(0), timeLimit(0), isRunning(false) {}
    void Start(double limit) {// ��ʼ��ʱ
        timeLimit = limit;
        timeRemaining = timeLimit;
        lastTime = high_resolution_clock::now();
        isRunning = true;
        startTime = high_resolution_clock::now();
    }

    void Update() {// ���¼�ʱ
        if (!isRunning) return;
        auto now = high_resolution_clock::now();
        auto elapsed = duration_cast<milliseconds>(now - lastTime).count() / 1000.0;
        timeRemaining = max(timeRemaining - elapsed, 0.0);
        lastTime = now;

        // ����ʵ����ʱ
        auto duration = duration_cast<milliseconds>(now - startTime);
        actualTimeUsed = duration.count() / 1000.0;
    }

    bool IsTimeout() const { return timeRemaining <= 0; }// �Ƿ�ʱ
    double GetRemainingTime() const { return timeRemaining; }// ��ȡʣ��ʱ��
    double GetTimeLimit() const { return timeLimit; }// ��ȡʱ������
    void Pause() { isRunning = false; }// ��ͣ��ʱ

    void Resume() {// ������ʱ
        auto now = high_resolution_clock::now();
        lastTime = now;
        isRunning = true;
    }

    // ��ӷ�������ȡʵ����ʱ
    double GetActualTimeUsed() {
        if (isRunning) {
            auto now = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(now - startTime);
            return duration.count() / 1000.0;
        }
        return actualTimeUsed;
    }

private:
    double timeRemaining;// ʣ��ʱ��
    double timeLimit;// ʱ������
    bool isRunning;// �Ƿ����ڼ�ʱ
    high_resolution_clock::time_point lastTime;// �ϴθ���ʱ��
    high_resolution_clock::time_point startTime; // ��ӿ�ʼʱ���¼
};



typedef struct { int x, y; } Pos;// ����ṹ��
const auto MAX_LEVEL = 2;//���ؿ���
const auto GROUP_PER_LEVEL = 20;//��ʼÿ�ؿ�������

const auto WINDOW_WIDTH = 600;//���ڿ��
const auto WINDOW_HEIGHT = 800;//���ڸ߶�
const auto BLOCK_WIDTH = 50;//������
const auto BLOCK_KINDS = 10;//����������
int cntGroup = 0;//��ǰ������
int Level = 1;//��ǰ�ؿ���

IMAGE imgBg, imgst, imgStart;//ͼƬ
IMAGE imgWin, imgLose, imgOut;
IMAGE imgBlock[BLOCK_KINDS + 1];
IMAGE imgGray[BLOCK_KINDS + 1];

vector<int> stack;// �ƶ�
vector<Pos> cardpos;// ����λ������
vector<int> map;// ������������
LevelTimer gameTimer;// �ؿ���ʱ��



void ShowCoverPage() {
    IMAGE imgCover;
    loadimage(&imgCover, _T("src/Cover.png"));
    int winWidth = WINDOW_WIDTH;//���ڿ��
    int winHeight = WINDOW_HEIGHT;//���ڸ߶�
    int imgWidth = imgCover.getwidth();
    int imgHeight = imgCover.getheight();

    int x = (winWidth - imgWidth) / 2;//����X����
    int y = (winHeight - imgHeight) / 2;//����Y����

    BeginBatchDraw();//��ʼ�������ͼ
    cleardevice();//�����Ļ
    putimage(x, y, &imgCover);//��ʾͼƬ
    EndBatchDraw();//�����������ͼ
    FlushBatchDraw();//ˢ���������ͼ

    RECT btnRect = {//��ť����
        x + 140,
        y + 220,
        x + 430,
        y + 580
    };

    bool clicked = false;//�Ƿ�����ʼ��ť
    while (!clicked) {
        ExMessage msg;
        if (peekmessage(&msg, EX_MOUSE)) {
            if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= btnRect.left && msg.x <= btnRect.right &&
                    msg.y >= btnRect.top && msg.y <= btnRect.bottom) {
                    clicked = true;
                }
            }
            else if (msg.message == WM_CLOSE) {// �رմ���
                closegraph();
                exit(0);
            }
        }
        Sleep(0);
    }
}

void Load_Img() {// ����ͼƬ
    loadimage(&imgBg, _T("src/Bg.png"));
    loadimage(&imgOut, _T("src/Out.png"));
    loadimage(&imgStart, _T("src/Cover.png"));
    loadimage(&imgWin, _T("src/Win.png"));
    loadimage(&imgLose, _T("src/Lose.png"));
    loadimage(&imgContinue, _T("src/Continue.png"));
    loadimage(&imgIncreaseDiff, _T("src/IncreaseDifficulty.png"));
    for (int i = 1; i <= BLOCK_KINDS; i++) {// ���ط���ͼƬ
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

int GameLoop() {// ��Ϸѭ��
    GameInit();// ��ʼ����Ϸ
    while (true) {
        if (!isWaitingClick) {
            gameTimer.Update();
            if (gameTimer.IsTimeout()) {
                actualTimeUsed = gameTimer.GetActualTimeUsed();// ��ʱʱ��¼ʵ����ʱ
                return -1;
            }
        }
        ExMessage msg;// ������Ϣ
        if (peekmessage(&msg, EX_MOUSE)) {
            if (msg.message == WM_CLOSE) {
                closegraph();
                exit(0);
            }
            if (isWaitingClick && msg.message == WM_LBUTTONDOWN) {// �����ʼ��Ϸ
                if (msg.x >= CONTINUE_BTN_X &&
                    msg.x <= CONTINUE_BTN_X + imgContinue.getwidth() &&
                    msg.y >= CONTINUE_BTN_Y &&
                    msg.y <= CONTINUE_BTN_Y + imgContinue.getheight()) {
                    isWaitingClick = false;
                    Level++;
                    if (Level > MAX_LEVEL) {
                        // ͨ��ʱ��¼ʵ����ʱ
                        actualTimeUsed = gameTimer.GetActualTimeUsed();
                        return 1;
                    }
                    Reset();
                    GameInit();
                    gameTimer.Resume();
                    continue;
                }
                if (msg.x >= INCREASE_DIFF_BTN_X &&// �Ѷ����Ӱ�ť
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
                                actualTimeUsed = gameTimer.GetActualTimeUsed(); // ʧ��ʱ��¼ʵ����ʱ
                                return 0;
                            }
                            if (IsWin()) {
                                actualTimeUsed = gameTimer.GetActualTimeUsed();// ʤ��ʱ��¼ʵ����ʱ
                                if (Level == MAX_LEVEL) return 1;
                                else {
                                    isWaitingClick = true;
                                    gameTimer.Pause();// ��ͣ��ʱ��
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

double CalculateLevelTimeLimit() {//ʱ�����
    if (useIncreasedDifficulty)    return 30.0 + (Level - 1) * 5.0;
    else   return 40.0 + (Level - 1) * 5.0;

}

void GameInit() {// ��ʼ����Ϸ
    cntGroup = 20 + (int)(10 * sqrt(Level - 1));// ��һ�ؿ�������Ϊ20��֮��ÿ������10
    Reset();// ������Ϸ
    InitMap();
    InitCardPositions();
    double timeLimit = CalculateLevelTimeLimit();
    gameTimer.Start(timeLimit);
}

void Reset() {// ������Ϸ clear����
    stack.clear();
    cardpos.clear();
    map.clear();
}

void InitMap() {//������ɿ���
    srand(time(0));// �������
    int kind = min(5 + Level, 10);// ���5�֣����10��
    for (int i = 0; i < cntGroup; i++) {// ������ɿ���
        int type = 1 + i % kind;
        for (int j = 0; j < 3; j++)
            map.emplace_back(type);
    }
    int cnt = cntGroup * 3, t;// �ƶѴ�С
    for (int i = 0; i < cntGroup * 3; i++) {// ��������ƶ�
        int index = rand() % cnt;
        t = map[i];
        map[i] = map[index];
        map[index] = t;
    }
}

void InitCardPositions() {// ������ɿ���λ��
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

void Update() {// ������Ϸ���棺��ʾ���������ơ��ƶѡ�ʱ�����ơ�ʣ��ʱ�䡢��ť
    BeginBatchDraw();
    putimage(0, 0, &imgBg);

    TCHAR level[10];
    _stprintf_s(level, _T("%d"), Level);// �ؿ���
    settextstyle(50, 0, _T("Consolas"));
    outtextxy(350, 43, level);
    settextstyle(20, 10, _T("Consolas"));
    settextcolor(0);

    TCHAR limitText[50];// ʱ��������ʾ
    _stprintf_s(limitText, _T("ʱ������: %.1fs"), gameTimer.GetTimeLimit());
    outtextxy(30, 100, limitText);
    if (gameTimer.GetRemainingTime() < 10.0)   settextcolor(RED);
    else   settextcolor(0);

    TCHAR timeText[50];// ʣ��ʱ����ʾ
    _stprintf_s(timeText, _T("ʣ��ʱ��: %.1fs"), gameTimer.GetRemainingTime());
    outtextxy(50, 120, timeText);

    settextcolor(0);// ���ʵ����ʱ��ʾ
    TCHAR actualTimeText[50];
    _stprintf_s(actualTimeText, _T("ʵ����ʱ: %.1fs"), actualTimeUsed);
    outtextxy(400, 120, actualTimeText);

    for (int i = 0; i < cntGroup * 3; i++) {
        if (map[i] <= 0) continue;// �����տ���
        if (IsCardCovered(i)) putimage(cardpos[i].x, cardpos[i].y, &imgGray[map[i]]);// ���Ʊ�����ʱ��ʾ��ɫ����
        else putimage(cardpos[i].x, cardpos[i].y, &imgBlock[map[i]]);// ��ʾ����
    }

    int cnt = stack.size();
    for (int i = 0; i < cnt; i++)
        putimage(95 + i * 60, 700, &imgBlock[-map[stack[i]]]);// ��ʾ�ƶ�
    if (isWaitingClick && Level < MAX_LEVEL) { putimage(50, 280, &imgContinue); } // ��ʾ������ť
    EndBatchDraw();
}

bool IsCardCovered(int index) {// �жϿ����Ƿ񱻸��ǣ����Ʊ��������ܱ�ѡ��
    for (int i = index + 1; i < cntGroup * 3; i++) {
        if (map[i] > 0) {
            if (cardpos[index].x + BLOCK_WIDTH > cardpos[i].x &&
                cardpos[index].x < cardpos[i].x + BLOCK_WIDTH &&
                cardpos[index].y + BLOCK_WIDTH > cardpos[i].y &&
                cardpos[index].y < cardpos[i].y + BLOCK_WIDTH)// ���Ʊ�����
                return true;
        }
    }
    return false;
}

void Insert(int index) {// ���뿨�ƣ������Ʋ����ƶ�
    vector<int> temp;
    for (auto i : stack) {
        temp.emplace_back(i);
        if (map[i] == -map[index]) {
            temp.emplace_back(index);
            map[index] *= -1;
        }
    }
    if (stack.size() == temp.size()) {// �ƶ�δ�����仯
        temp.emplace_back(index);
        map[index] *= -1;
    }
    stack = temp;
}

void Eliminate() {// ����������������ͬ��ɫ��3����
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
// �ж���Ϸ�Ƿ�ʤ�������п��ƶ�����
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
        ShowCoverPage();// ��ʾ����ҳ�棬�ȴ��û������ʼ��Ϸ
        actualTimeUsed = 0.0; // ����ʵ����ʱ
        Level = 1;
        isWaitingClick = false;

        int result = GameLoop();
        cout << "������սʱ�������ǣ�" << CalculateLevelTimeLimit() << " ��  ";
        // ���������ս��ʵ����ʱ������1λС����
        cout << "������սʵ����ʱ: " << fixed << setprecision(1) << actualTimeUsed << "�� " << endl;

        if (result == 1) {// ͨ��
            putimage(50, 280, &imgWin);
            FlushBatchDraw();
            bool restartWithDifficulty = false;
            while (true) {
                ExMessage msg;
                if (peekmessage(&msg, EX_MOUSE)) {// �����û�����¼������¿�ʼ���˳�
                    if (msg.message == WM_LBUTTONDOWN) {// ���¿�ʼ��ť
                        if (msg.x >= WIN_EXIT_BTN_X &&
                            msg.x <= WIN_EXIT_BTN_X + WIN_BUTTON_WIDTH &&
                            msg.y >= WIN_EXIT_BTN_Y &&
                            msg.y <= WIN_EXIT_BTN_Y + WIN_BUTTON_HEIGHT) {
                            closegraph();
                            exit(0);
                        }
                        if (msg.x >= WIN_INCREASE_DIFF_BTN_X &&// �Ѷ����Ӱ�ť
                            msg.x <= WIN_INCREASE_DIFF_BTN_X + WIN_BUTTON_WIDTH &&
                            msg.y >= WIN_INCREASE_DIFF_BTN_Y &&
                            msg.y <= WIN_INCREASE_DIFF_BTN_Y + WIN_BUTTON_HEIGHT) {
                            useIncreasedDifficulty = true;
                            restartWithDifficulty = true;
                            break;
                        }
                    }
                    else if (msg.message == WM_CLOSE) {// �رմ���
                        closegraph();
                        exit(0);
                    }
                }
                Sleep(10);
            }

            if (restartWithDifficulty) {
                int imgX = (WINDOW_WIDTH - imgIncreaseDiff.getwidth()) / 2;// ����X����
                int imgY = (WINDOW_HEIGHT - imgIncreaseDiff.getheight()) / 2;
                BeginBatchDraw();
                cleardevice();
                putimage(imgX, imgY, &imgIncreaseDiff);
                EndBatchDraw();
                FlushBatchDraw();
                cout << "���Ѷ�ģʽ�Ѽ��" << endl;
                cout << "�´���սʱ������: " << fixed << setprecision(1)
                    << CalculateLevelTimeLimit() << "��" << endl;
                Sleep(2000); // ��ʾ2����Զ���ʼ��һ����ս
                continue;
            }
        }
        else if (result == 0) {// ʧ��
            putimage(50, 280, &imgLose);
            FlushBatchDraw();
            bool restartClicked = false;
            while (!restartClicked) {
                ExMessage msg;
                if (peekmessage(&msg, EX_MOUSE)) {// �����û�����¼������¿�ʼ���˳�
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
        else if (result == -1) {// ��ʱ
            putimage(50, 280, &imgOut);
            FlushBatchDraw();
            cout << "��ս��ʱ��ʵ����ʱ: " << fixed << setprecision(2)
                << actualTimeUsed << "��" << endl;
            Sleep(1500);
            break;
        }
    }
    closegraph();
    return 0;
}