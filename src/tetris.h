
#pragma once

#include <string>
#include <functional>
#include <thread>
#include <chrono>

#define M 10
#define N 20
#define MC M+5//16
#define NC N+11//32

struct t_data
{
	int s, st, t, f1;
};
struct tetris
{
	int _pool[MC][NC];
	int(*pool)[NC];
	int	tmap[8][4][16];
	int x, y, s, st;
};
/*

	s是方块类型有7种
	st是变换有4种
*/
class Tetris
{
public:
	struct iv2
	{
		int x, y;
	};
	enum state
	{
		GAME_OVER = 0,
		GAME_START = 1,
		GAME_PAUSE = 2
	};

	// 分数ui
	void* _score = 0;
	// 分数
	int64_t _inc_score = 0;
	int64_t _max_score = 0;
	// 速率
	int t1 = 500;
	// 游戏时间/秒
	double gtime = 0;
	// 间隔触发按钮时间
	double _keyts = 0.1;
	double jt = 1;
	// 存档
	njson saven;
	std::string savepath;

	std::map<std::string, void* > msound;
	std::vector<uint32_t> dcolor = { 0, 0xff0000ff, 0xff00ff00, 0xffff0000, 0xff0080ff, 0xffffff00, 0xffff6a73, 0xff7174e7 };
	// 加载存档
	njson sn;

	int cr_score = 0;

	bool pause = false;
	bool _pause = false;
	bool _has_save = false;
	bool _auto_save = false;
	bool _is_load = false;
private:
	tetris gt = {};
	//std::string ostr;
	int _key = 0;
	// 运行状态
	int _rt = 0;
	int _state = -1;
	t_data _temp = {};
	//char gcText[] = " 1LJTSZ#";
	std::vector<iv2> cvs2;
	// 随机池
	std::vector<int> rpool = {};
	std::vector<int> rpool1 = {};
	// 难度模式：0，1，2
	int _mode = 0;
	bool isrun = true;
	bool isstart = false;
public:
	Tetris();
	~Tetris();
	bool load(njson n);
	void save(njson& o, bool is);
	// 0开始，1结束
	int start(int idx);
	int onkey(int key);
	int run(double s);
	std::function<void(char, int, int)>  _display = nullptr;
	std::function<void(char, int, int)>  _displayPre = nullptr;
	std::function<void()>  game_over_cb = nullptr;
	std::function<void()>  game_start_cb = nullptr;
	std::function<void(int n)>  pay_sound_cb = nullptr;
	void(*set_ui_str)(void* ui, const char* str) = nullptr;

	void show_str();
private:
	void trsInit();

	void show();
	void make();

	int trsCopy(int sp[], int x, int y, int c);

	int trsScene(double s);

};

uint64_t get_micro1();

