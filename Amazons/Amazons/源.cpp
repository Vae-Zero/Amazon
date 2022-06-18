#include<easyx.h>
#include<conio.h>
#include<graphics.h>
#include<cstdlib>
#include<cstdio>
#include<queue>
#include<windows.h>
#include<ctime>
#include<mmsystem.h>
#pragma comment(lib,"Winmm.lib")
#pragma warning(disable:4996)
#define boardsize 8
#define firstsuper 0.125
#define inaccessibility 16843009
#define infinity 2147483647
#define _CRT_SECURE_NO_WARNINGS
using namespace std;

struct movement {
	int startX, startY, resultX, resultY, obstacleX, obstacleY;
};
int color_player, color_bot;
int x_pixel[10] = { 0,12,86,160,235,308,387,464,540,608 };
int y_pixel[10] = { 0,12,86,160,235,308,387,464,539,606 };
int x_chess, y_chess;
int turnID = 1;
IMAGE imgblack, imgwhite, imggrey, blank, imghelp;
clock_t starttime, currenttime;
int chessboard[boardsize + 1][boardsize + 1];
int Queenmove[boardsize + 1][boardsize + 1];
int Kingmove[boardsize + 1][boardsize + 1];
int RivalQueenmove[boardsize + 1][boardsize + 1];
int RivalKingmove[boardsize + 1][boardsize + 1];
int flag[boardsize + 1][boardsize + 1];
int dx[10] = { 0,1,1,1,0,-1,-1,-1,0,0 };
int dy[10] = { -1,-1,0,1,1,1,0,-1,0,0 };
float territory1, territory2;
float position1, position2;
float blankMobility[boardsize + 1][boardsize + 1];
float mobility, Rivalmobility;
int dispersal, Rivaldispersal;
bool readyforreturn = 0;
FILE* fp;
FILE* fptmp;
FILE* fpsave;
FILE* fpturn;
queue <int> Q;

void initialize_mainscene();
void choicescene();
void initialize_boardscene();
void rules();
void load();
void game();
void save();
void undo();
void help();
void cal_pixel(int x, int y);
movement play_player(int color);
movement play_bot(int color);
void result();
bool legalstep(int x, int y, int xx, int yy);
movement GetMovement(int turnID, int color);
bool inMap(int x, int y);
void takestep(movement move);
void initialize(int color);
void cal_Queen();
void cal_King();
void cal_RivalQueen();
void cal_RivalKing();
void cal_territory1();
void cal_territory2();
void cal_position1();
void cal_position2();
void cal_blankMobility();
void cal_mobility_both();
void cal_dispersal();
movement layer1(int turnID);
float layer2(int threshold, int turnID);
float evaluation(int turn);
int judgewinner();
void takesteponscreen(movement move, int color);
void music_click();

int main() {
	initialize_mainscene();
	return 0;
}

//初始界面
void initialize_mainscene() {
	mciSendString("open ..\\image\\Memories.wav type MPEGVideo", NULL, 0, NULL);
	mciSendString("setaudio ..\\image\\Memories.wav volume to 220", 0, 0, 0);
	mciSendString("play ..\\image\\Memories.wav repeat", NULL, 0, NULL);
	initgraph(1000, 618);
	loadimage(NULL, _T("..\\image\\mainSceneImage.png"));
	MOUSEMSG mouse_mainscene;
	while (true) {
		mouse_mainscene = GetMouseMsg();
		if (mouse_mainscene.uMsg == WM_LBUTTONDOWN) {
			if (mouse_mainscene.x >= 340 && mouse_mainscene.x <= 460 && mouse_mainscene.y >= 375 && mouse_mainscene.y <= 425) {
				choicescene();
			}
			else if (mouse_mainscene.x >= 540 && mouse_mainscene.x <= 670 && mouse_mainscene.y >= 375 && mouse_mainscene.y <= 425) {
				load();
			}
		}
	}
	return;
}

//选择先手、后手或阅读规则
void choicescene() {
	music_click();
	loadimage(NULL, _T("..\\image\\choiceImage.png"));
	MOUSEMSG mouse_choicescene;
	while (true) {
		mouse_choicescene = GetMouseMsg();
		if (mouse_choicescene.uMsg == WM_LBUTTONDOWN) {
			int x = mouse_choicescene.x - 890;
			int y = mouse_choicescene.y - 195;
			if (x * x + y * y <= 900) {
				color_player = 0;
				color_bot = 1;
				initialize_boardscene();
			}
			else if (x * x + (y - 100) * (y - 100) <= 900) {
				color_player = 1;
				color_bot = 0;
				initialize_boardscene();
			}
			else if (x * x + (y - 200) * (y - 200) <= 900) {
				rules();
			}
		}
	}
	return;
}

//展示亚马逊棋的规则
void rules() {
	music_click();
	loadimage(NULL, _T("..\\image\\rules.png"));
	MOUSEMSG mouse_rules;
	while (true) {
		mouse_rules = GetMouseMsg();
		if (mouse_rules.uMsg == WM_LBUTTONDOWN) {
			if (mouse_rules.x >= 750 && mouse_rules.x <= 900 && mouse_rules.y >= 530 && mouse_rules.y <= 555) {
				choicescene();
			}
		}
	}
	return;
}

//在界面显示初始化的棋盘
void initialize_boardscene() {
	music_click();
	loadimage(NULL, _T("..\\image\\chessboardImage.png"));
	loadimage(&imgblack, _T("..\\image\\button_black.png"));
	loadimage(&imgwhite, _T("..\\image\\button_white.png"));
	loadimage(&imggrey, _T("..\\image\\button_grey.png"));
	loadimage(&blank, _T("..\\image\\blank.png"));
	loadimage(&imghelp, _T("..\\image\\button_help.png"));
	putimage(x_pixel[1], y_pixel[3], &imgblack);
	putimage(x_pixel[3], y_pixel[1], &imgblack);
	putimage(x_pixel[6], y_pixel[1], &imgblack);
	putimage(x_pixel[8], y_pixel[3], &imgblack);
	putimage(x_pixel[1], y_pixel[6], &imgwhite);
	putimage(x_pixel[3], y_pixel[8], &imgwhite);
	putimage(x_pixel[6], y_pixel[8], &imgwhite);
	putimage(x_pixel[8], y_pixel[6], &imgwhite);
	initialize(color_bot);
	game();
	return;
}

//游戏进行过程
void game() {
	if (!fptmp) {
		fptmp = fopen("..\\saved\\gametmp.txt", "w");
		fclose(fptmp);
		fptmp = fopen("..\\saved\\gametmp.txt", "a+");
	}
	if (color_player == 0) {
		while (true) {
			movement move = play_player(color_player);
			fprintf(fptmp, "%d %d %d %d %d %d\n", move.startX, move.startY, move.resultX, move.resultY, move.obstacleX, move.obstacleY);
			takesteponscreen(move, color_player);
			result();
			turnID++;
			move = play_bot(color_bot);
			fprintf(fptmp, "%d %d %d %d %d %d\n", move.startX, move.startY, move.resultX, move.resultY, move.obstacleX, move.obstacleY);
			takesteponscreen(move, color_bot);
			music_click();
			takestep(move);
			result();
			turnID++;
		}
	}
	else {
		while (true) {
			movement move = play_bot(color_bot);
			fprintf(fptmp, "%d %d %d %d %d %d\n", move.startX, move.startY, move.resultX, move.resultY, move.obstacleX, move.obstacleY);
			takesteponscreen(move, color_bot);
			music_click();
			takestep(move);
			result();
			turnID++;
			move = play_player(color_player);
			fprintf(fptmp, "%d %d %d %d %d %d\n", move.startX, move.startY, move.resultX, move.resultY, move.obstacleX, move.obstacleY);
			takesteponscreen(move, color_player);
			result();
			turnID++;
		}
	}
	return;
}

//将鼠标的坐标转化为棋盘上的坐标
void cal_pixel(int x, int y) {
	for (int i = 1; i < 9; i++) {
		if (x > x_pixel[i] && x < x_pixel[i + 1]) {
			x_chess = i;
			break;
		}
	}
	for (int i = 1; i < 9; i++) {
		if (y > y_pixel[i] && y < y_pixel[i + 1]) {
			y_chess = i;
			break;
		}
	}
	return;
}

//判断玩家的行棋是否符合皇后的走法
bool legalstep(int x, int y, int xx, int yy) {
	bool flag = 1;
	int tmpx = xx - x, tmpy = yy - y;
	if (tmpx != tmpy && x != xx && y != yy && tmpx != -tmpy) return false;
	if (tmpx != 0) tmpx = tmpx / abs(tmpx);
	if (tmpy != 0) tmpy = tmpy / abs(tmpy);
	for (int k = 1; k < 8; k++) {
		x += tmpx;
		y += tmpy;
		if (chessboard[x][y] != 0) {
			flag = 0;
			break;
		}
		if (x == xx && y == yy) break;
	}
	return flag;
}

//玩家进行鼠标操作
movement play_player(int color) {
	movement re = {};
	IMAGE img;
	//re.startX = re.startY = re.resultX = re.resultY = re.obstacleX = re.obstacleY = 0;
	MOUSEMSG mouse_boardscene;
	while (true) {
		mouse_boardscene = GetMouseMsg();
		if (mouse_boardscene.uMsg == WM_LBUTTONDOWN) {
			int xx = mouse_boardscene.x, yy = mouse_boardscene.y;
			if (xx >= 745 && xx <= 885 && yy >= 95 && yy <= 140) {
				save();
			}
			else if (xx >= 745 && xx <= 905 && yy >= 185 && yy <= 238) {
				undo();

			}
			else if (xx >= 745 && xx <= 880 && yy >= 275 && yy <= 338) {
				help();
			}
			else if (xx >= 745 && xx <= 860 && yy >= 390 && yy <= 440) {
				exit(0);
			}
			else if (xx >= x_pixel[0] && xx <= x_pixel[9] && yy >= y_pixel[0] && yy <= y_pixel[9]) {
				cal_pixel(xx, yy);
				if (re.startX == 0) {
					if (chessboard[x_chess][y_chess] == -1) {
						re.startX = x_chess; re.startY = y_chess;
						loadimage(&img, _T("..\\image\\button_grown.png"));
						putimage(x_pixel[re.startX], y_pixel[re.startY], &img);
					}
				}
				else if (re.resultX == 0) {
					if (legalstep(re.startX, re.startY, x_chess, y_chess)) {
						re.resultX = x_chess; re.resultY = y_chess;
						loadimage(&img, _T("..\\image\\blank.png"));
						putimage(x_pixel[re.startX], y_pixel[re.startY], &img);
						if (color_player == 0) {
							loadimage(&img, _T("..\\image\\button_black.png"));
						}
						else {
							loadimage(&img, _T("..\\image\\button_white.png"));
						}
						putimage(x_pixel[re.resultX], y_pixel[re.resultY], &img);
						chessboard[re.resultX][re.resultY] = chessboard[re.startX][re.startY];
						chessboard[re.startX][re.startY] = 0;
					}
				}
				else {
					if (legalstep(re.resultX, re.resultY, x_chess, y_chess)) {
						re.obstacleX = x_chess;
						re.obstacleY = y_chess;
						chessboard[re.obstacleX][re.obstacleY] = 2;
						break;
					}
				}
			}
		}
	}
	return re;
}

//bot进行决策
movement play_bot(int color) {
	movement choice;
	choice = GetMovement(turnID, color);
	return choice;
}

//在界面上实现行棋的动作
void takesteponscreen(movement move, int color) {
	IMAGE img;
	loadimage(&img, _T("..\\image\\blank.png"));
	putimage(x_pixel[move.startX], y_pixel[move.startY], &img);
	if (color == 0) {
		loadimage(&img, _T("..\\image\\button_black.png"));
		putimage(x_pixel[move.resultX], y_pixel[move.resultY], &img);
	}
	if (color == 1) {
		loadimage(&img, _T("..\\image\\button_white.png"));
		putimage(x_pixel[move.resultX], y_pixel[move.resultY], &img);
	}
	for (int i = 0; i < 500000; i++);
	loadimage(&img, _T("..\\image\\button_grey.png"));
	putimage(x_pixel[move.obstacleX], y_pixel[move.obstacleY], &img);
	return;
}

//判断是否有一方的棋子均无法移动
int judgewinner() {
	bool movable = 0, Rivalmovable = 0;
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 1 && !movable) {
				for (int k = 0; k < 8; k++) {
					int x = i + dx[k];
					int y = j + dy[k];
					if (!inMap(x, y)) continue;
					if (chessboard[x][y] == 0) {
						movable = 1;
						break;
					}
				}
			}
			else if (chessboard[i][j] == -1 && !Rivalmovable) {
				for (int k = 0; k < 8; k++) {
					int x = i + dx[k];
					int y = j + dy[k];
					if (!inMap(x, y)) continue;
					if (chessboard[x][y] == 0) {
						Rivalmovable = 1;
						break;
					}
				}
			}
		}
	}
	if (!movable) {
		return 1 - color_bot;
	}
	else if (!Rivalmovable) {
		return color_bot;
	}
	else {
		return -1;
	}
}

//判断游戏是否结束以及输赢情况，并输出结果
void result() {
	IMAGE img;
	int winner = judgewinner();
	if (winner == -1) {
		return;
	}
	else if (winner == color_player) {
		loadimage(&img, _T("..\\image\\win.png"));
		mciSendString("stop ..\\image\\Memories.wav", NULL, 0, NULL);
		mciSendString("close ..\\image\\Memories.wav", NULL, 0, NULL);
		for (int i = 0; i < 100000; i++);
		mciSendString("open ..\\image\\win.mp3", NULL, 0, NULL);
		mciSendString("play ..\\image\\win.mp3", NULL, 0, NULL);
	}
	else {
		loadimage(&img, _T("..\\image\\lose.png"));
		mciSendString("stop ..\\image\\Memories.wav", NULL, 0, NULL);
		mciSendString("close ..\\image\\Memories.wav", NULL, 0, NULL);
		for (int i = 0; i < 100000; i++);
		mciSendString("open ..\\image\\lose.mp3", NULL, 0, NULL);
		mciSendString("play ..\\image\\lose.mp3", NULL, 0, NULL);
	}
	putimage(300, 185, &img);
	MOUSEMSG mouse_return;
	while (true) {
		mouse_return = GetMouseMsg();
		if (mouse_return.uMsg == WM_LBUTTONDOWN) {
			cleardevice();
			initialize_mainscene();
		}
	}
	return;
}

/*以下实现四个功能*/

//读档功能
void load() {
	music_click();
	loadimage(NULL, _T("..\\image\\chessboardImage.png"));
	fp = fopen("..\\saved\\saved.txt", "r");
	fpturn = fopen("..\\saved\\detail_saved.txt", "r");
	fptmp = fopen("..\\saved\\gametmp.txt", "w");
	fclose(fptmp);
	fptmp = fopen("..\\saved\\gametmp.txt", "a+");
	movement tmp;
	if (!fpturn) exit(1);
	if (!fp) exit(2);
	fscanf(fpturn, "%d%d%d", &color_bot, &color_player, &turnID);
	initialize(color_bot);
	int i = 1;
	while (i < turnID) {
		i++;
		fscanf(fp, "%d%d%d%d%d%d\n", &tmp.startX, &tmp.startY, &tmp.resultX, &tmp.resultY, &tmp.obstacleX, &tmp.obstacleY);
		takestep(tmp);
	}
	IMAGE bot, player, obstacle;
	loadimage(&obstacle, _T("..\\image\\button_grey.png"));
	if (color_bot == 0) {
		loadimage(&bot, _T("..\\image\\button_black.png"));
		loadimage(&player, _T("..\\image\\button_white.png"));
	}
	else {
		loadimage(&player, _T("..\\image\\button_black.png"));
		loadimage(&bot, _T("..\\image\\button_white.png"));
	}
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 1) {
				putimage(x_pixel[i], y_pixel[j], &bot);
			}
			if (chessboard[i][j] == -1) {
				putimage(x_pixel[i], y_pixel[j], &player);
			}
			if (chessboard[i][j] == 2) {
				putimage(x_pixel[i], y_pixel[j], &obstacle);
			}
		}
	}
	fclose(fp);
	fclose(fpturn);
	game();
	return;
}

//存档功能
void save() {
	music_click();
	movement tmp;
	fpsave = fopen("..\\saved\\saved.txt", "w");
	rewind(fptmp);
	int i = 1;
	while (i < turnID) {
		i++;
		fscanf(fptmp, "%d%d%d%d%d%d\n", &tmp.startX, &tmp.startY, &tmp.resultX, &tmp.resultY, &tmp.obstacleX, &tmp.obstacleY);
		fprintf(fpsave, "%d %d %d %d %d %d\n", tmp.startX, tmp.startY, tmp.resultX, tmp.resultY, tmp.obstacleX, tmp.obstacleY);
	}
	fpturn = fopen("..\\saved\\detail_saved.txt", "w");
	fprintf(fpturn, "%d %d %d\n", color_bot, color_player, turnID);
	fclose(fpsave);
	fclose(fpturn);
	return;
}

//悔棋功能
void undo() {
	music_click();
	movement tmp;
	rewind(fptmp);
	initialize(color_bot);
	int i = 1;
	while (i < turnID - 2) {
		i++;
		fscanf(fptmp, "%d%d%d%d%d%d\n", &tmp.startX, &tmp.startY, &tmp.resultX, &tmp.resultY, &tmp.obstacleX, &tmp.obstacleY);
		takestep(tmp);
	}
	turnID -= 2;
	IMAGE bot, player, obstacle, blank;
	loadimage(&blank, _T("..\\image\\blank.png"));
	loadimage(&obstacle, _T("..\\image\\button_grey.png"));
	if (color_bot == 0) {//black
		loadimage(&bot, _T("..\\image\\button_black.png"));
		loadimage(&player, _T("..\\image\\button_white.png"));
	}
	else {
		loadimage(&player, _T("..\\image\\button_black.png"));
		loadimage(&bot, _T("..\\image\\button_white.png"));
	}
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 1) {
				putimage(x_pixel[i], y_pixel[j], &bot);
			}
			if (chessboard[i][j] == -1) {
				putimage(x_pixel[i], y_pixel[j], &player);
			}
			if (chessboard[i][j] == 2) {
				putimage(x_pixel[i], y_pixel[j], &obstacle);
			}
			if (chessboard[i][j] == 0) {
				putimage(x_pixel[i], y_pixel[j], &blank);
			}
		}
	}
	return;
}

//提示功能
void help() {
	music_click();
	movement choice;
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 1 || chessboard[i][j] == -1) {
				chessboard[i][j] *= -1;
			}
		}
	}
	choice = GetMovement(turnID, color_player);
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 1 || chessboard[i][j] == -1) {
				chessboard[i][j] *= -1;
			}
		}
	}
	IMAGE img;
	loadimage(&img, _T("..\\image\\button_grown.png"));
	putimage(x_pixel[choice.startX], y_pixel[choice.startY], &img);
	for (int i = 0; i < 550000000; i++);
	loadimage(&img, _T("..\\image\\button_help.png"));
	putimage(x_pixel[choice.resultX], y_pixel[choice.resultY], &img);
	for (int i = 0; i < 550000000; i++);
	loadimage(&img, _T("..\\image\\button_grey.png"));
	putimage(x_pixel[choice.obstacleX], y_pixel[choice.obstacleY], &img);
	for (int i = 0; i < 750000000; i++);
	loadimage(&img, _T("..\\image\\blank.png"));
	putimage(x_pixel[choice.resultX], y_pixel[choice.resultY], &img);
	putimage(x_pixel[choice.obstacleX], y_pixel[choice.obstacleY], &img);
	if (color_player == 0) {
		loadimage(&img, _T("..\\image\\button_black.png"));
	}
	else {
		loadimage(&img, _T("..\\image\\button_white.png"));
	}
	putimage(x_pixel[choice.startX], y_pixel[choice.startY], &img);
	return;
}

//音效
void music_click() {
	mciSendString("open ..\\image\\Untitled.wav type MPEGVideo", NULL, 0, NULL);
	mciSendString("close ..\\image\\Untitled.wav", NULL, 0, NULL);
	mciSendString("open ..\\image\\Untitled.wav type MPEGVideo", NULL, 0, NULL);
	mciSendString("play ..\\image\\Untitled.wav", NULL, 0, NULL);
	return;
}

/*以下为AI的决策部分*/

//棋子判断是否在棋盘内
bool inMap(int x, int y) {
	if (x < 1 || x > boardsize || y < 1 || y > boardsize) {
		return false;
	}
	return true;
}

//执行一次下棋的操作
void takestep(movement move) {
	chessboard[move.resultX][move.resultY] = chessboard[move.startX][move.startY];
	chessboard[move.startX][move.startY] = 0;
	chessboard[move.obstacleX][move.obstacleY] = 2;
	return;
}

//初始化棋盘
void initialize(int color) {
	memset(chessboard, 0, sizeof(chessboard));
	if (color == 0) {
		chessboard[1][3] = chessboard[3][1] = chessboard[6][1] = chessboard[8][3] = 1;
		chessboard[1][6] = chessboard[3][8] = chessboard[6][8] = chessboard[8][6] = -1;
	}
	else {
		chessboard[1][3] = chessboard[3][1] = chessboard[6][1] = chessboard[8][3] = -1;
		chessboard[1][6] = chessboard[3][8] = chessboard[6][8] = chessboard[8][6] = 1;
	}
	return;
}

//计算我方棋子按照皇后的走法到达每一空格所需的最小步数
void cal_Queen() {
	while (!Q.empty()) {
		Q.pop();
	}
	memset(flag, 0, sizeof(flag));
	memset(Queenmove, 1, sizeof(Queenmove));
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 1) {
				flag[i][j] = -1;
				Queenmove[i][j] = 0;
				Q.push(i);
				Q.push(j);
			}
			if (chessboard[i][j] == -1 || chessboard[i][j] == 2) {
				flag[i][j] = -1;
			}
		}
	}
	while (!Q.empty()) {
		int x = Q.front();
		Q.pop();
		int y = Q.front();
		Q.pop();
		for (int k = 0; k < 8; k++) {
			for (int m = 1; m < boardsize; m++) {
				int xx = x + m * dx[k];
				int yy = y + m * dy[k];
				if (!inMap(xx, yy)) break;
				if (flag[xx][yy] == -1) break;
				if (Queenmove[xx][yy] > Queenmove[x][y] + 1) {
					Queenmove[xx][yy] = Queenmove[x][y] + 1;
					if (flag[xx][yy] == 0) {
						Q.push(xx);
						Q.push(yy);
						flag[xx][yy] = 1;
					}
				}
			}
		}
		flag[x][y] = 0;
	}
	return;
}

//计算敌方棋子按照皇后的走法到达每一空格所需的最小步数
void cal_RivalQueen() {
	while (!Q.empty()) {
		Q.pop();
	}
	memset(flag, 0, sizeof(flag));
	memset(RivalQueenmove, 1, sizeof(RivalQueenmove));
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == -1) {
				flag[i][j] = -1;
				RivalQueenmove[i][j] = 0;
				Q.push(i);
				Q.push(j);
			}
			if (chessboard[i][j] == 1 || chessboard[i][j] == 2) {
				flag[i][j] = -1;
			}
		}
	}
	while (!Q.empty()) {
		int x = Q.front();
		Q.pop();
		int y = Q.front();
		Q.pop();
		for (int k = 0; k < 8; k++) {
			for (int m = 1; m < boardsize; m++) {
				int xx = x + m * dx[k];
				int yy = y + m * dy[k];
				if (!inMap(xx, yy)) break;
				if (flag[xx][yy] == -1) break;
				if (RivalQueenmove[xx][yy] > RivalQueenmove[x][y] + 1) {
					RivalQueenmove[xx][yy] = RivalQueenmove[x][y] + 1;
					if (flag[xx][yy] == 0) {
						Q.push(xx);
						Q.push(yy);
						flag[xx][yy] = 1;
					}
				}
			}
		}
		flag[x][y] = 0;
	}
	return;
}

//计算我方棋子按照国王的走法到达每一空格所需的最小步数
void cal_King() {
	while (!Q.empty()) {
		Q.pop();
	}
	memset(flag, 0, sizeof(flag));
	memset(Kingmove, 1, sizeof(Kingmove));
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 1) {
				flag[i][j] = -1;
				Kingmove[i][j] = 0;
				Q.push(i);
				Q.push(j);
			}
			if (chessboard[i][j] == -1 || chessboard[i][j] == 2) {
				flag[i][j] = -1;
			}
		}
	}
	while (!Q.empty()) {
		int x = Q.front();
		Q.pop();
		int y = Q.front();
		Q.pop();
		for (int k = 0; k < 8; k++) {
			int xx = x + dx[k];
			int yy = y + dy[k];
			if (inMap(xx, yy)) {
				if (Kingmove[xx][yy] > Kingmove[x][y] + 1) {
					Kingmove[xx][yy] = Kingmove[x][y] + 1;
					if (flag[xx][yy] == 0) {
						Q.push(xx);
						Q.push(yy);
						flag[xx][yy] = 1;
					}
				}
			}
		}
		flag[x][y] = 0;
	}
	return;
}

//计算敌方棋子按照国王的走法到达每一空格所需的最小步数
void cal_RivalKing() {
	while (!Q.empty()) {
		Q.pop();
	}
	memset(flag, 0, sizeof(flag));
	memset(RivalKingmove, 1, sizeof(RivalKingmove));
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == -1) {
				flag[i][j] = -1;
				RivalKingmove[i][j] = 0;
				Q.push(i);
				Q.push(j);
			}
			if (chessboard[i][j] == 1 || chessboard[i][j] == 2) {
				flag[i][j] = -1;
			}
		}
	}
	while (!Q.empty()) {
		int x = Q.front();
		Q.pop();
		int y = Q.front();
		Q.pop();
		for (int k = 0; k < 8; k++) {
			int xx = x + dx[k];
			int yy = y + dy[k];
			if (inMap(xx, yy)) {
				if (RivalKingmove[xx][yy] > RivalKingmove[x][y] + 1) {
					RivalKingmove[xx][yy] = RivalKingmove[x][y] + 1;
					if (flag[xx][yy] == 0) {
						Q.push(xx);
						Q.push(yy);
						flag[xx][yy] = 1;
					}
				}
			}
		}
		flag[x][y] = 0;
	}
	return;
}

//计算按照皇后的走法空格的控制权归属
void cal_territory1() {
	territory1 = 0;
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (Queenmove[i][j] == RivalQueenmove[i][j] && Queenmove[i][j] < inaccessibility) {
				territory1 += firstsuper;
			}
			else if (Queenmove[i][j] < RivalQueenmove[i][j]) {
				territory1++;
			}
			else if (Queenmove[i][j] > RivalQueenmove[i][j]) {
				territory1--;
			}
		}
	}
	return;
}

//计算按照国王的走法空格的控制权归属
void cal_territory2() {
	territory2 = 0;
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (Kingmove[i][j] == RivalKingmove[i][j] && Kingmove[i][j] < inaccessibility) {
				territory2 += firstsuper;
			}
			else if (Kingmove[i][j] < RivalKingmove[i][j]) {
				territory2++;
			}
			else if (Kingmove[i][j] > RivalKingmove[i][j]) {
				territory2--;
			}
		}
	}
	return;
}

//计算按照皇后的走法控制权的位置优势大小
void cal_position1() {
	position1 = 0;
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 0) {
				float tmp1 = pow(2, -Queenmove[i][j]);
				float tmp2 = pow(2, -RivalQueenmove[i][j]);
				position1 += tmp1 - tmp2;
			}
		}
	}
	position1 *= 2;
	return;
}

//计算按照皇后的走法控制权的位置优势大小
void cal_position2() {
	position2 = 0;
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 0) {
				float tmp = min(1, max(-1, (RivalKingmove[i][j] - Kingmove[i][j]) / 6));
				position2 += tmp;
			}
		}
	}
	return;
}

//计算每个空格的灵活度（即周边的空格数）
void cal_blankMobility() {
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			blankMobility[i][j] = 0;
			for (int k = 0; k < 8; k++) {
				int xx = i + dx[k];
				int yy = j + dy[k];
				if (inMap(xx, yy) && chessboard[xx][yy] == 0) {
					blankMobility[i][j]++;
				}
			}
		}
	}
	return;
}

//计算敌我双方棋子的灵活度
void cal_mobility_both() {
	mobility = 0;
	Rivalmobility = 0;
	for (int x = 1; x <= boardsize; x++) {
		for (int y = 1; y <= boardsize; y++) {
			if (chessboard[x][y] == 1) {
				for (int k = 0; k < 8; k++) {
					for (int m = 1; m < boardsize; m++) {
						int xx = x + m * dx[k];
						int yy = y + m * dy[k];
						if (!inMap(xx, yy)) break;
						if (chessboard[xx][yy] != 0) break;
						if (Kingmove[xx][yy] == 0) break;
						mobility += (float)blankMobility[xx][yy] / (float)Kingmove[xx][yy];
					}
				}
			}
			else if (chessboard[x][y] == -1) {
				for (int k = 0; k < 8; k++) {
					for (int m = 1; m < boardsize; m++) {
						int xx = x + m * dx[k];
						int yy = y + m * dy[k];
						if (!inMap(xx, yy)) break;
						if (chessboard[xx][yy] != 0) break;
						if (RivalKingmove[xx][yy] == 0) break;
						Rivalmobility += (float)blankMobility[xx][yy] / (float)RivalKingmove[xx][yy];
					}
				}
			}
		}
	}
	return;
}

//计算我方棋子在棋盘的分散程度
void cal_dispersal() {
	int pos = 0, tmpx[4] = {}, tmpy[4] = {};
	for (int i = 1; i <= boardsize; i++) {
		for (int j = 1; j <= boardsize; j++) {
			if (chessboard[i][j] == 1) {
				tmpx[pos] = i;
				tmpy[pos] = j;
				pos++;
			}
		}
	}
	dispersal = infinity;
	for (int i = 0; i < 4; i++) {
		for (int j = i + 1; j < 4; j++) {
			int gapx = tmpx[i] - tmpx[j];
			int gapy = tmpy[i] - tmpy[j];
			int tmp = gapx * gapx + gapy * gapy;
			dispersal = min(dispersal, tmp);
		}
	}
	return;
}

//评估函数
float evaluation(int turn) {
	cal_Queen();
	cal_King();
	cal_RivalQueen();
	cal_RivalKing();
	cal_territory1();
	cal_territory2();
	cal_position1();
	cal_position2();
	cal_blankMobility();
	cal_mobility_both();
	cal_dispersal();
	float a, b, c, d, e, f;
	if (turn <= 6) {
		a = 0.70; b = 0.30; c = 0.13; d = 0.13; e = 0.25;
	}
	else if (turn > 6 && turn <= 20) {
		a = 0.80; b = 0.80; c = 0.15; d = 0.15; e = 0.20;
	}
	else if (turn > 20 && turn <= 60) {
		a = 0.70; b = 0.60; c = 0.30; d = 0.30; e = 0.10;
	}
	else {
		a = 0.90; b = 0.50; c = 0.20; d = 0.20; e = 0.05;
	}
	if (turn <= 10) f = 1.80;
	else f = 0;
	return a * territory1 + b * territory2 + c * position1 + d * position2 + e * (mobility - Rivalmobility) + f * dispersal;
}

//寻找对方行棋后最差情况中最优的一个
movement layer1(int turnID) {
	movement choice;
	int threshold = -infinity;
	for (int x = 1; x <= boardsize; x++) {
		for (int y = 1; y <= boardsize; y++) {
			if (chessboard[x][y] == 1) {
				for (int k = 0; k < 8; k++) {
					for (int m = 1; m < boardsize; m++) {
						currenttime = clock();
						if (currenttime - starttime > 20 * CLOCKS_PER_SEC) {
							return choice;
						}
						int xx = x + m * dx[k];
						int yy = y + m * dy[k];
						if (!inMap(xx, yy)) break;
						if (chessboard[xx][yy] != 0) break;
						chessboard[xx][yy] = 1;
						chessboard[x][y] = 0;
						for (int i = 0; i < 8; i++) {
							for (int j = 1; j < boardsize; j++) {
								int xxx = xx + j * dx[i];
								int yyy = yy + j * dy[i];
								if (!inMap(xxx, yyy)) break;
								if (chessboard[xxx][yyy] != 0) break;
								chessboard[xxx][yyy] = 2;
								float tmp = layer2(threshold, turnID);
								if (tmp > threshold) {
									threshold = tmp;
									choice.startX = x;
									choice.startY = y;
									choice.resultX = xx;
									choice.resultY = yy;
									choice.obstacleX = xxx;
									choice.obstacleY = yyy;
								}
								chessboard[xxx][yyy] = 0;
							}
						}
						chessboard[xx][yy] = 0;
						chessboard[x][y] = 1;
					}
				}
			}
		}
	}
	return choice;
}

//敌方采用最优策略行棋后造成我方最差情况
float layer2(int threshold, int turnID) {
	float now = infinity;
	for (int x = 1; x <= boardsize; x++) {
		for (int y = 1; y <= boardsize; y++) {
			if (chessboard[x][y] == -1) {
				for (int k = 0; k < 8; k++) {
					for (int m = 1; m < boardsize; m++) {
						int xx = x + m * dx[k];
						int yy = y + m * dy[k];
						if (!inMap(xx, yy)) break;
						if (chessboard[xx][yy] != 0) break;
						chessboard[xx][yy] = -1;
						chessboard[x][y] = 0;
						for (int i = 0; i < 8; i++) {
							for (int j = 1; j < boardsize; j++) {
								int xxx = xx + j * dx[i];
								int yyy = yy + j * dy[i];
								if (!inMap(xxx, yyy)) break;
								if (chessboard[xxx][yyy] != 0) break;
								chessboard[xxx][yyy] = 2;
								float tmp = evaluation(turnID);
								now = min(now, tmp);
								if (now <= threshold) {
									chessboard[xxx][yyy] = 0;
									chessboard[xx][yy] = 0;
									chessboard[x][y] = -1;
									return now;
								}
								chessboard[xxx][yyy] = 0;
							}
						}
						chessboard[xx][yy] = 0;
						chessboard[x][y] = -1;
						if (now <= threshold) {
							return now;
						}
					}
				}
			}
		}
	}
	return now;
}

//bot做出决策
movement GetMovement(int turnID, int color) {
	movement Bot_choice;
	if (turnID == 1 && color == 0) {
		for (int i = 0; i < 500000000; i++);
		Bot_choice.startX = 6;
		Bot_choice.startY = 1;
		Bot_choice.resultX = 6;
		Bot_choice.resultY = 7;
		Bot_choice.obstacleX = 3;
		Bot_choice.obstacleY = 4;
		return Bot_choice;
	}
	starttime = clock();
	Bot_choice = layer1(turnID);
	return Bot_choice;
}
