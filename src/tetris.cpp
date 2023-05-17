#include<random> 
#include "tetris.h" 
 
// 消除行数奖励分数
int score[5] = { 0, 1, 3, 6, 10 };
void sleep1(int s)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(s));
}
Tetris::Tetris()
{
}

Tetris::~Tetris()
{
	isrun = false;
}
void mk_pool(std::vector<int>& rp, int* p)
{
	rp.clear();
	for (size_t i = 0; i < 7; i++)
	{
		int n = p[i];
		rp.push_back(i + 1);
		for (int k = 0; k < n; k++)
			rp.push_back(i + 1);
	}
}
bool Tetris::load(njson n)
{
	if (n.size())
		_max_score = std::max(_max_score, toInt(n["max_score"], 0));
	_mode = 2;
	int mvs[7] = {};//" 1LJTSZ#";
	switch (_mode)
	{
	case 0:
		mvs[0] = 5;	// 1
		mvs[1] = 2;	// L
		mvs[2] = 2;	// J
		mvs[3] = 2;	// T
		mvs[4] = 1;	// S
		mvs[5] = 1;	// S
		mvs[6] = 2;	// #
		break;
	case 1:
		mvs[0] = 3;	// 1
		mvs[1] = 2;	// L
		mvs[2] = 2;	// J
		mvs[3] = 2;	// T
		mvs[4] = 1;	// S
		mvs[5] = 1;	// S
		mvs[6] = 2;	// #
		break;
	case 2:
		mvs[0] = 1;	// 1
		mvs[1] = 1;	// L
		mvs[2] = 1;	// J
		mvs[3] = 1;	// T
		mvs[4] = 1;	// S
		mvs[5] = 1;	// S
		mvs[6] = 1;	// #
		break;
	default:
		break;
	}
	mk_pool(rpool, mvs);
	bool r = toBool(n["gstate"]);
	_is_load = r;
	if (r)
	{
		dcolor = toVector<uint32_t>(n["color"]);


		_inc_score = toInt(n["score"], 0);
		gtime = toDouble(n["time"], 0.0);
		pause = true;
		_pause = true;
		_state = GAME_PAUSE;
		glm::ivec4* ps = (glm::ivec4*)&gt.x;
		glm::ivec2* pt = (glm::ivec2*)&_temp.t;
		glm::ivec2* last = (glm::ivec2*)&_temp.s;
		*ps = toiVec4(n["ps"]);
		*pt = toiVec2(n["pt"]);
		*last = toiVec2(n["last"]);
		njson& mp = n["m"];
		for (int y = 0; gt.pool[0][y] == 0 && y < NC; ++y)
		{
			auto& d = mp[y];
			for (int x = 0; gt.pool[x][0] == 0 && x < MC; ++x)
				gt.pool[x][y] = toInt(d[x], 0);
		}
		r = true;
	}
	return r;
}
void Tetris::save(njson& o, bool is)
{
	njson e;
	e.swap(o);
	o["max_score"] = _max_score;
	o["time"] = gtime;
	o["gstate"] = is; 
	if (is)
	{
		o["score"] = _inc_score;
		o["ps"] = { gt.x, gt.y,  gt.s,  gt.st };
		o["last"] = { _temp.s,_temp.st };

		njson& mp = o["m"];
		for (int y = 0; gt.pool[0][y] >= 0 && y < NC; ++y)
		{
			auto& d = mp[y];
			for (int x = 0; gt.pool[x][0] >= 0 && x < MC; ++x)
				d.push_back(gt.pool[x][y]);
		}
	}
}
int Tetris::start(int idx)
{
	if (idx == 1)
	{
		_rt = 3;
	}
	else if (idx == 2)
	{
		_key = 27;
		_rt = 2;
	}
	else {
		_rt = 1;
	}
	 
	return 0;
}
void Tetris::trsInit()
{
	// 7种形状，每种形状4个状态保存旋转
	int sp[8][4] = { { 3840/*15*/, 8738/*4369*/, 0, 0 }, { 23, 785, 116, 547 }, { 71, 275, 113, 802 },
	{ 39, 305, 114, 562 }, { 54, 561, 0, 0 }, { 99, 306, 0, 0}, { 51, 51 ,0,0}, { -1 } };
	int* p, i, j, b;
	for (p = sp[0]; *p >= 0; ++p) if (*p == 0) *p = p[-2];
	gt.pool = &gt._pool[4];
	for (j = 0; j < 7; ++j)
		for (i = 0; i < 4; ++i)
			for (b = 0; b < 16; ++b)
				gt.tmap[j + 1][i][b] = (sp[j][i] & 1) * (j + 1),
				sp[j][i] >>= 1;
	memset(gt._pool, -1, sizeof(gt._pool));
	for (i = 0; i < M; ++i)
		memset(&gt.pool[i], 0, sizeof(int[N]));
	_inc_score = 0;
	gtime = 0;
	return;
}

int Tetris::trsCopy(int sp[], int x, int y, int c)
{
	int i, cx, cy;
	int ret = 1;
	for (i = 0; i < 16; ++i)
	{
		if (sp[i])
		{
			cx = x + (i & 3), cy = y + (i >> 2);
			if (cx < 0)
			{
				ret = 0;
				break;
			}
			if (cx >= M)
			{
				ret = 0;
				break;
			}
			if (cy >= 0)
			{
				int& a = gt.pool[cx][cy];
				if (a)
					if (c == 2)
						a = 0;
					else
					{
						ret = 0;
						break;
					}
				if (c == 1) a = sp[i];
			}
		}
	}
	return ret;
}
int Tetris::onkey(int key)
{
	if (_key != key)
		_key = key;
	return key;
}
int Tetris::trsScene(double s)
{
	if (!pause)
	{
		int x, y = 0;
		if (_temp.f1 == 1)
		{
			_temp.t = 0;
			gt.x = (M - 4) / 2, gt.y = -3;
			gt.s = _temp.s, gt.st = _temp.st;
			make();
			_temp.f1 = 0;
		}
		_temp.t -= s * 1000;
		gtime += s;
		//for (--; sleep1(_ms), --_temp.t)
		{
			int k = _key;
			_key = 0;
			while (k)
			{
				//if (gt.y < 0)break;
				if (k == 27) {
					return 0;
				}
				if (k == 'A' || k == 'a' || k == 37)
				{
					if (trsCopy(gt.tmap[gt.s][gt.st], gt.x - 1, gt.y, 0))
						--gt.x;
				}
				else if (k == 'D' || k == 'd' || k == 39)
				{
					if (trsCopy(gt.tmap[gt.s][gt.st], gt.x + 1, gt.y, 0))
						++gt.x;
				}
				else if (k == 'W' || k == 'w' || k == 38)
				{
					if (trsCopy(gt.tmap[gt.s][(gt.st + 1) % 4], gt.x, gt.y, 0))
						gt.st = (gt.st + 1) % 4;
				}
				break;
			}
			if (k == 'S' || k == 's' || k == 40 || _temp.t < 0)
			{
				if (trsCopy(gt.tmap[gt.s][gt.st], gt.x, gt.y + 1, 0))
				{
					++gt.y, _temp.t = t1;
				}
				else
				{
					int n = trsCopy(gt.tmap[gt.s][gt.st], gt.x, gt.y, 1);
					if ((gt.y < 0))
					{
						n = 0;
					}
					y = N;
					int inc = 0;
					for (--y; y > 0; --y) {
						for (x = 0; gt.pool[x][y] > 0; ++x);
						if (gt.pool[x][y] < 0)
						{
							++inc;
							for (k = y++; k > 0; --k)
								for (x = 0; gt.pool[x][0] >= 0; ++x)
								{
									gt.pool[x][k] = gt.pool[x][k - 1];
									gt.pool[x][k - 1] = 0;	// 如果没有置0会有bug
								}
						}
					}
					if (n == 0)
					{
						show();
					}
					if (inc)
					{
						_inc_score += score[inc];
						show_str();
					}
					cr_score = inc;
					_temp.f1 = 1;
					_auto_save = true;
					return n;//到底了
				}
			}
		}
	}
	trsCopy(gt.tmap[gt.s][gt.st], gt.x, gt.y, 1);//复制
	show();//显示画面
	trsCopy(gt.tmap[gt.s][gt.st], gt.x, gt.y, 2);//清除

	return 1;
}
void Tetris::show()
{
	if (_display)
	{
		for (int y = 0; gt.pool[0][y] >= 0 && y < NC; ++y)
			for (int x = 0; gt.pool[x][0] >= 0 && x < MC; ++x)
				_display(gt.pool[x][y], x, y);
	}
}

void Tetris::show_str()
{
	if (_score && set_ui_str)
	{
		auto str = std::format((char*)u8"分数\n%d\n最高分\n%d", _inc_score, _max_score);
		set_ui_str(_score, str.c_str());
	}
}

//微秒
uint64_t get_micro1()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
void Tetris::make()
{
	static std::default_random_engine random(get_micro1());

	std::uniform_int_distribution<int> dis1(1, rpool.size());
	std::uniform_int_distribution<int> dis2(0, 3);
	//int a = 1 + rand() % 7, st = 1 + rand() % 3;
	if (_temp.f1 == 1 || _temp.s < 1 || _temp.st < 0)
	{
		if (rpool.empty()) {
			load(sn);
		}
		if (rpool1.empty())
		{
			assert(rpool.size());
			rpool1 = rpool;
			std::shuffle(rpool1.begin(), rpool1.end(), random);
		}
		assert(rpool1.size());
		int a = 0, st = dis2(random);
		//	a = dis1(random), st =;
		a = *rpool1.rbegin();
		rpool1.pop_back();
		cvs2.push_back({ a, st });
		_temp.s = a;
		_temp.st = st;
	}

	if (_displayPre)
	{
		int i = 0;
		for (int m = 0; m < 4; ++m)
			for (int n = 0; n < 4; ++n, ++i)
				_displayPre(gt.tmap[_temp.s][_temp.st][i], n, m);
	}
}


int Tetris::run(double s)
{
	if (isrun)
	{
		int r = _rt;
		_rt = 0;
		bool issave = false;
		auto tid = get_tid();
		switch (r)
		{
		case 1:
		{
			if (_state != GAME_START)
			{
				gt = {};
				trsInit();
				bool isload = false;

				{
					isload = load(sn);
					sn = {};
				}
				if (!isload)
				{
					_state = GAME_START;
					_temp.f1 = 1;
				}
				else {
					trsScene(s);
				}
				make();
				_temp.t = t1;
				if (game_start_cb)
				{
					game_start_cb();
				}
				show_str();
				isstart = true; 
			}
		}
		break;
		case 2:
			_state = GAME_OVER;
			break;
		case 3:
			_pause = !_pause;
			if (_pause)
			{
				issave = true;
				_state = GAME_PAUSE;
			}
			else
			{
				_state = GAME_START;
			}
			break;
		default:
			break;
		} 
		if (!_pause)
		{
			if (_state == GAME_START)
			{
				if (trsScene(s) == 0)
				{
					_state = GAME_OVER;
				}
			}

			if (isstart && _state == GAME_OVER && game_over_cb)
			{
				_key = 0;
				if (_inc_score > _max_score)
				{
					_max_score = _inc_score;
				}
				issave = true;
				_is_load = false;
				game_over_cb(); cvs2.clear();
				isstart = false;
			}
		}
		if (issave || _auto_save)
		{
			save(saven, _state == GAME_START || _state == GAME_PAUSE);
			_has_save = true;
		}
	}
	return 0;
}

