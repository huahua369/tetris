

#ifdef __ANDROID__
#define NC_STATIC
#endif // __ANDROID__

#include <algorithm>
#include <SDL2/SDL_keycode.h>

// 内存泄漏检测
#ifdef _DEBUG
#define new new( _CLIENT_BLOCK, __FILE__, __LINE__)
#endif

#include "dsmain.h"

#include "tetris.h"

#ifdef _WIN32

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#ifndef LOGI
#define LOGI
#endif
#else
#ifndef HZ_VERSION
#define HZ_VERSION "luna5.0"
#endif // !HZ_VERSION
#ifndef LOGI
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, HZ_VERSION, __VA_ARGS__))
#endif
#endif



void init_game(sdl_form* form, glm::vec2 size, int keyms);




void init_game(sdl_form* form, glm::vec2 size, int keyms)
{
	auto bw = ctx->get_bw();
	assert(bw);
	auto setfn = bw->externalDataPath + "setting.json";
	auto setn = read_json(setfn);

	auto savepath = bw->externalDataPath + "save1.json";
	auto sjn = read_json(savepath);

	auto fsize = size;
	if (size.x < M && size.y < N)
		fsize = form->get_size();
	int fontheight = 16;
	{
		float fk = std::min(fsize.x, fsize.y);
		fk *= 0.05;
		fontheight = fk + 0.5;
		if (fontheight < 12)
		{
			fontheight = 12;
		}
	}
	int nw = fsize.x * 0.8 / 16;
	if (nw & 1)nw++;
	int nh = nw * 32;
	int nc = M * N;

	glm::vec2 ncpos = { nw, nw };

	// ui

	{
		njson dv;
		dv["t"] = "div";
		dv["gid"] = "gdiv";
		dv["pid"] = 0;	// pid==0是根节点
		dv["dragable"] = 0;	// 可拖动
		dv["front"] = 0 * 1;	// 点击前置显示
		dv["border"] = 0;	dv["fill"] = 0xf0222222; // 边框背景颜色
		//dv["color"] = info["color"];
		dv["rounding"] = 0;	// 圆角
		dv["thickness"] = 2;
		glm::ivec2 s = fsize;
		dv["size"] = { s.x,s.y };
#ifdef _WIN32
		dv["pos"] = { 10,10 };
		dv["dragable"] = 1;	// 可拖动
#endif 
		dv["justify_content"] = (int)flex::e_align::align_center;
		dv["align_content"] = (int)flex::e_align::align_space_evenly;


		njson r;

		njson rv;
		rv["t"] = "view";
		rv["fill"] = 0x02005020;
		rv["border_color"] = 0x50ff8030;
		rv["tc"] = -1;
		// auto_size{xy一般为负数(直接和父级相加)，zw要大于0}

		fsize = glm::vec2(nw * (M + 7), nw * (N + 2));
		rv["size"] = { fsize.x,fsize.y };
		rv["vss"] = { fsize.x,fsize.y };
		//rv["step"] = { scroll.x, scroll.y };
		rv["sc_visible"] = false;
		rv["gid"] = "gamect";
		rv[".c"] = r;
		dv[".c"].push_back(rv);
		new_ui(&dv, 0);

	}
	auto sv = get_svptr(ctx, "gamect", 0);

	ui::bcs_t* ptr = 0;
	if (sv)
	{
		auto ac = sv->ptm;
		g2d_c* p = new g2d_c(ac);
		if (p)
		{
			njson d, v;
			njson st = mk_style(0xffff8888, 0, 0, {}, 1, 1);
			v["style"].push_back(st);
			st = mk_style(0xffff8888, 0x25ff8030, 0, {}, 1, 1);
			v["style"].push_back(st);
			std::vector<glm::vec4> v4, v0, vpre;
			std::vector<int> stp;
			glm::vec2 cw = { M, N }, size = { nw,nw };
			float scale = 0.98, scale1 = 1;
			auto pos = ncpos;
			glm::ivec2 nss = size * scale;
			glm::vec2 r0pos = { (nss.x - nw) * 2, 2 * (nss.y - nw) }, r0size = { M * (nw), N * (nw) };
			r0size -= r0pos + glm::vec2((nss.x - nw), (nss.y - nw));
			auto ps1 = r0pos + ncpos;

			for (int y = 0; y < cw.y; y++)
			{
				for (int x = 0; x < cw.x; x++)
				{
					int idx = y * cw.x + x;
					glm::vec4 t = { pos.x + x * size.x,pos.y + y * size.y, nss.x,nss.y };
					v4.push_back(t);
				}
			}
			ncpos.x += nw * (M + 1);

			//预览
			for (int y = 0; y < 4; y++)
			{
				for (int x = 0; x < 4; x++)
				{
					int idx = y * 4 + x;
					glm::vec4 t = { x * size.x + ncpos.x,ncpos.y + y * size.y, nss.x,nss.y };
					vpre.push_back(t);
				}
			}
			{
				glm::vec4 t = { ps1.x  ,ps1.y  , scale1 * r0size.x, scale1 * r0size.y };
				v0.push_back(t);
			}

			njson rs = new_rect(1, v4.data(), v4.size());
			njson rs1 = new_rect(0, v0.data(), v0.size());
			njson rs2 = new_rect(1, vpre.data(), vpre.size());
			d.push_back(rs1);
			d.push_back(rs2);
			d.push_back(rs);
			int order = 0;
			p->ftx = ctx->get_ftctx();
			auto g1 = p->parseg(d, v, {});
			if (g1)
			{
				ptr = g1;
				sv->push(g1, order);
			}
		}

	}
	uint32_t rbc = 0x800050ff;
	ui::button_u* ks = 0;

	auto fps = new ui::tag_u();
	fps->fontheight = fontheight;
	fps->color = 0xaf0060f0;
	fps->light = 0;
#ifdef _WIN32
	fps->set_pos({ 0.1 * nw, 0.1 * nw });
#else
	fps->set_pos({ 0.5 * nw, 0.5 * nw });
#endif 
	ui::div_u* r = get_ptr<ui::div_u>(ctx, "gdiv", 0, 0);
	// 显示分数
	auto score = new ui::tag_u();
	ncpos.y += nw * 5;
	score->height = 0;
	score->sps.x = 0;
	score->sps.y = 0;
	score->light = 0;
	score->set_pos(ncpos);
	score->fontheight = fontheight;
	score->color = 0xff0020ff;
	score->set_visible(false);
	sv->push(score);

	glm::vec2 ss = { fsize.x, 10 * nw };

	Tetris* _game = new Tetris();
	_game->savepath = savepath;
	if (keyms > 0)
		_game->_keyts = keyms * 0.001;
	_game->_score = score;
	_game->sn = sjn;
	static bool has_sound_effect = toBool(setn["has_sound_effect"], false);
	static bool has_sound = toBool(setn["has_sound"], false);
	static bool keylayout = toBool(setn["keylayout"], false);
	static bool touch = toBool(setn["touch"], true);
	static bool g_mode = toBool(setn["g_mode"], false);
	bw->sound_pause(has_sound);
	bw->set_has_sound_effect(has_sound_effect);
	auto gkey = new ui::div_u();
	gkey->set_size(ss);
	gkey->set_bf_color(0, 0x20888888, fontheight, fontheight * 0.2);
	gkey->set_align((int)flex::e_align::align_space_evenly, (int)flex::e_align::align_space_evenly);

	auto gkey0 = new ui::div_u();
	auto nss0 = ss;
	nss0.x *= 0.6;
	gkey0->set_size(nss0);
	gkey0->set_bf_color(0, 0, fontheight, fontheight * 0.2);
	gkey0->set_align((int)flex::e_align::align_space_evenly, (int)flex::e_align::align_center);
	gkey0->set_align_items((int)flex::e_align::align_center);
	gkey0->_auto_left = true;
	auto npos = fsize;
	npos.x = 10;
	npos.y -= ss.y + 10;
	r->push(gkey);
	gkey->push(gkey0);
	static std::vector<std::string> gstr = { "scorering.xxogg", "oneup.ogg", "mushroomappear.ogg", "mushroomeat.ogg" };
	sv->update_cb = [=](cpg_time* t) {
		_game->run(t->deltaTime);
		if (_game->_auto_save)
		{
			if (_game->cr_score >= 0)
			{
				if (_game->cr_score > 2)
					_game->cr_score--;
				auto str = gstr[_game->cr_score];
				bw->play_sound(_game->msound[str]);
				_game->cr_score = 0;
			}
			_game->_auto_save = false;
		}
		if (_game->_has_save)
		{
			_game->saven["color"] = _game->dcolor;
			save_json(_game->saven, _game->savepath);
			_game->_has_save = false;
		}

		static double ts = 1;
		ts += t->deltaTime;
		if (ts < 1)return;
		auto f = ctx->get_fps();
		if (f < 0)return;
		ts = 0;
		auto str = std::to_string(f);
		str = "fps: " + str;
		fps->set_label(str);
	};
	auto syss = r->get_size() - (gkey->get_size() + sv->get_size());
	if (syss.y > 0)
	{
		syss.x = fsize.x;
		syss.y *= 0.36;
		r->push(new ui::base_u(syss));
	}
	{
		static std::string k = "WADSW";
		static std::vector<std::string> str = { (char*)u8"↑",(char*)u8"←",(char*)u8"→",(char*)u8"↓",(char*)u8"↑" };

		glm::vec2 bs = {};
		bs.x = bs.y = (nss0.x * 0.9) * 0.3;
		ui::button_u* upbtn = 0;
		auto set0 = new ui::div_u();
		for (size_t i = 0; i < 5; i++)
		{
			auto bt = new ui::button_u();
			if (i == 4)
			{
				upbtn = bt;
			}
			bt->fontheight = fontheight;
			bt->set_color_idx(0);
			auto bs1 = bs;
			if (i == str.size() - 1)
			{
				bs1.x = bs1.y = ss.x * 0.3;
				// ss.y * 0.3;
			}
			bt->_is_circle = true;
			bt->set_gradient(0x20ff2000, -1, (bs1.x * 0.5) - 1, 0);

			bt->set_size(bs1);
			bt->set_label(str[i]);
			static int kki = -1;
			bt->down_cb = [=](ui::button_u* p, cpg_time* ct) {
				auto kts = _game->_keyts;
				if (str[i] == (char*)u8"↑")
				{
					kts *= 2;
				}
				if (str[i] == (char*)u8"↓")
					kts *= 0.5;
				if (_game->jt > kts) {
					if (kki != i) {
						kki = i;
						if (str[i] == (char*)u8"↑")
						{
							bw->play_sound(_game->msound["stomp.ogg"]);
						}
						else {
							//bw->play_sound(_game->msound["button.ogg"]);
						}
					}

					_game->onkey(k[i]);
					_game->jt = 0;
#ifdef _WIN32
					printf("down: %d\n", i);
#else
					auto st = "\ngame down: " + std::to_string(i) + "\n";
					dslog(st.c_str(), st.size());
#endif
				}
				_game->jt += ct->deltaTime;
			};
			bt->click_cb = [=](ui::button_u* p) {
				kki = -1;
				set0->set_visible(false);
				_game->onkey(0); _game->jt = _game->_keyts + 1;
			};
			if (i < str.size() - 1)
			{
				bs1.x *= (i == 2) ? 0.8 : 1.2;
				int qn = i == str.size() - 2 ? 2 : 1;
				ui::base_u* eb[2] = {};
				for (int q = 0; q < qn; q++)
				{
					auto nk = new ui::base_u(bs1);
					rect_cs r = {};
					style_cs sc = {};
					r.size = bs1;
					eb[q] = (nk);
				}
				if (eb[0])
				{
					gkey0->push(eb[0]);
				}
				gkey0->push(bt);
				if (eb[1])
				{
					gkey0->push(eb[1]);
				}
			}
			else
				gkey->push(bt);

		}

		upbtn->set_order(keylayout ? -1 : 1);
		static bool swset0 = false;
		{
			// 设置界面

			auto nss0 = fsize;
			nss0.x *= 0.5;
			nss0.y *= 0.5;
			auto tbs = nss0;
			tbs.y = fontheight;
			set0->set_size(nss0);
			set0->set_bf_color(0, 0xc0999999, fontheight, fontheight * 0.2);
			set0->set_align((int)flex::e_align::align_space_evenly, (int)flex::e_align::align_center);
			set0->set_align_items((int)flex::e_align::align_center);
			set0->_auto_left = true;
			nss0.x *= 0.5;
			nss0.y *= 0.5;
			set0->set_pos(nss0);
			set0->set_position(true);
			set0->set_visible(swset0);
			r->push(set0);
			std::vector<std::string> vs = { (char*)u8"键位", (char*)u8"音乐" , (char*)u8"音效", (char*)u8"触控" };
			bool bv[] = { keylayout, has_sound, has_sound_effect,touch };
			for (int i = 0; i < vs.size(); i++)
			{
				auto tg = new ui::tag_u(vs[i].c_str());
				tg->fontheight = fontheight;
				tg->color = -1;
				tg->light = 0;
				auto sw1 = new ui::switch_u();
				sw1->color = { 0xff0000ff,0xffff8000,0xffffffff };
				sw1->set_size({ 1.5 * fontheight * 2, 1.5 * fontheight }, 0.6 * fontheight);

				sw1->on_change_cb = [=](ui::switch_u* p, bool v) {
					if (i == 2)
					{
						has_sound_effect = v;
						bw->set_has_sound_effect(v);
					}
					if (i == 1)
					{
						has_sound = v;
						bw->sound_pause(v);
					}
					if (i == 0) {
						upbtn->set_order(v ? -1 : 1);
						gkey->sort_inc++;
						keylayout = v;
					}
					if (i == 3) {
						// 关闭触控
						touch = v;
						if (gkey)
						{
							gkey->set_visible(!touch);
						}
					}
					njson setn;
					setn["has_sound_effect"] = has_sound_effect;
					setn["has_sound"] = has_sound;
					setn["keylayout"] = keylayout;
					setn["touch"] = touch;
					save_json(setn, setfn);
				};
				sw1->set_value(bv[i]);
				set0->push(tg);
				set0->push(sw1);
				set0->push(new ui::base_u(tbs));
			}
		}
		{
			std::string str[4] = { (char*)u8"开始",(char*)u8"暂停",(char*)u8"设置",(char*)u8"继续" };
			bs = { nw * 5, nw * 2.5 };
			ncpos.y += nw * 4;
			ncpos.x -= 0.5 * nw;
			std::vector<ui::button_u*> vts;
			for (int i = 0; i < 3; i++)
			{
				auto bt = new ui::button_u();
				bt->fontheight = fontheight;
				bt->set_color_idx(0);
				bt->set_gradient(0, 0xa00050ff, bs.x * 0.1, 0);
				bt->set_size(bs);
				ncpos.y += bs.y * 1.1;
				bt->set_pos(ncpos);
				bt->set_label(str[i]);
				sv->push(bt);
				vts.push_back(bt);
			}
			ks = vts[0];
			ui::button_u* ks1 = 0;
			ks1 = vts[1];

			if (gkey)
			{
				gkey->set_visible(!touch);
			}
			kcb = [=](int key, bool pressed) {
				//wsad键盘操作
				char ch = 0;
				set0->set_visible(false);
				if (!ks1)return;
				if (ks1->_disabled)
				{
					key = 0;
				}
				switch (key)
				{
				case SDLK_SPACE:
				{
					if (!pressed)
					{
						_game->start(1);
						ks1->set_label(_game->pause ? str[1] : str[3]);
						_game->pause = !_game->pause;
						bw->play_sound(_game->msound["pause.ogg"]);
						return;
					}
				}
				break;
				case SDLK_UP:
				case SDLK_w:
					ch = 'w';
					bw->play_sound(_game->msound["stomp.ogg"]);
					break;
				case SDLK_DOWN:
				case SDLK_s:
					ch = 's';
					break;
				case SDLK_LEFT:
				case SDLK_a:
					ch = 'a';
					break;
				case SDLK_RIGHT:
				case SDLK_d:
					ch = 'd';
					break;
				default:
					return;
				}
				_game->jt = _game->_keyts + 1;
				_game->onkey(pressed ? ch : 0);
				//if (!pressed)
				_game->jt = 0;
			};
			// 触屏操作
			form->on_touch_cb = [=](int id, glm::vec2 pos, bool iup, int t) {
				do {
					if (!set0->contains(pos, 0) && set0->get_visible())
					{
						set0->set_visible(false);
						break;
					}
					// 按键区不响应
					if (id != 0 || /*gkey->contains(pos, 0) || gkey0->contains(pos, 0) ||*/ !touch || pos.x > ncpos.x || !ks1 || ks1->_disabled)
					{
						break;
					}

					glm::ivec2 ps = pos;
					static glm::ivec2 lastps = {};
					static int step = nw * 1;
					if (t == 0)
						lastps = pos;
					int x = ps.x - lastps.x;
					int y = ps.y - lastps.y;
					glm::ivec2 a = { abs(x),abs(y) };
					if (a.x > a.y)
					{
						y = 0;
					}
					if (a.x < a.y)
					{
						x = 0;
					}
					int ix = -1;
					//"WADSW";
					if (t == 1)
					{
						if (x != 0)
						{
							if (x - step > 0)
							{
								// 右
								ix = 2; 
							}
							if (x + step < 0)
							{
								// 左
								ix = 1; 
							}
						}
						else if (y - step > 0)
						{
							// 下
							ix = 3;
						}
					}
					if (y != 0 && t == 2)
					{
						if (y < 0)
						{
							// 上
							ix = 0;
						}
					}
					if (ix >= 0)
					{
						lastps = pos;
						_game->onkey(k[ix]);
					}
					_game->jt = 0;

				} while (0);
			};
			for (size_t i = 0; i < vts.size(); i++)
			{
				auto bt = vts[i];
				bt->click_cb = [=](ui::button_u* p) {

					if (i == 0)
					{
						p->_disabled = true;
						ks1->_disabled = false;
					}
					if (i != 2)
						_game->start(i);
					if (i == 1)
					{
						p->set_label(_game->pause ? str[1] : str[3]);
						_game->pause = !_game->pause;
						bw->play_sound(_game->msound["pause.ogg"]);// , 0, -1, 1);
					}
					else {
						//bw->play_sound(_game->msound["button.ogg"]);
					}

					if (i == 2)
					{
						auto bvi = set0->get_visible();
						swset0 = !bvi;
						set0->set_visible(swset0);
					}
					else {
						set0->set_visible(false);
					}
#ifdef _WIN32
					printf("click: %d\n", i);
#else
					auto st = "\ngame click: " + std::to_string(i) + "\n";
					dslog(st.c_str(), st.size());
#endif
				};
			}
			auto rs = new ui::tag_u("0123456789.+-*/");
			rs->fontheight = fontheight;
			rs->color = 0xaf0060f0;
			rs->light = 0;
			rs->set_position(true);
			rs->set_pos({ -fontheight,-fontheight * 3 });
			r->push(rs);

			ks1->_disabled = true;
			_game->game_over_cb = [=]() {
				ks->_disabled = false;
				ks1->_disabled = true;

				bw->play_sound(_game->msound["gameover.ogg"]);
			};

			static std::default_random_engine random(get_micro1());
			_game->game_start_cb = [=]() {
				rs->set_visible(false);
				if (!_game->_is_load)
				{
					auto& pdc = _game->dcolor;
					auto dt = pdc.begin();
					dt++;
					std::shuffle(dt, pdc.end(), random);
				}
				ks->_disabled = true;
				ks1->_disabled = false;
				_game->pause = _game->_pause;
				ks1->set_label(_game->_pause ? str[3] : str[1]);
			};
		}
	}
#if 1
	// 更新显示到ui
	_game->_displayPre = [=](char a, int x, int y) {
		int idx = y * 4 + x;
		auto& rr = ptr[1];
		auto& st = rr.ct[idx];
		st.color = set_alpha(a ? _game->dcolor[a] : 0xffff8888, a ? 0xc0 : 0x10);
		st.fill = set_alpha(_game->dcolor[a], a ? 0xcc : 0x02);// 0x25ff8030;
	};
	_game->_display = [=](char a, int x, int y) {
		int idx = y * M + x;
		auto& rr = ptr[0];
		auto rsp1 = &rr.ct[idx];
		rsp1->color = set_alpha(a ? _game->dcolor[a] : 0xffff8888, a ? 0xc0 : 0x10);
		rsp1->fill = set_alpha(_game->dcolor[a], a ? 0xcc : 0x02);// 0x25ff8030;
	};
#endif
	_game->set_ui_str = [](void* ui, const char* str) {
		auto pt = (ui::tag_u*)ui;
		if (pt && str)
		{
			pt->set_label(str);
			pt->set_visible(true);
		}
	};
	_game->show_str();

	// 开始加载存档
	bool r0 = toBool(sjn["gstate"]);
	_game->_max_score = toInt(sjn["max_score"], 0);
	_game->show_str();
	if (r0)
		ks->click_cb(ks);
	{

		std::vector<std::string> vfn;
		std::vector<std::string> vfn1;
		std::vector<std::string> mss = { "overworld.ogg","overworld-fast.ogg","underground.ogg","underground-fast.ogg","underwater.ogg","underwater-fast.ogg" };
		std::vector<std::string> mss1 = { "pause.ogg", "button.ogg" ,"stomp.ogg", "gameover.ogg" };
		mss1.insert(mss1.begin(), gstr.begin(), gstr.end());
#ifdef _WIN32
		//auto svs = File::IterFiles("assets/sound/", &vfn1);
		for (auto& mt : mss)
		{
			vfn.push_back("assets/sound/" + mt);
		}
		for (auto& mt : mss1)
		{
			auto nit = "assets/sound/" + mt;
			vfn1.push_back(nit);
			auto bk = bw->add_sound_effect(nit);
			if (bk)
			{
				_game->msound[mt] = bk;
			}
		}
#else
		for (auto it : mss)
		{
			std::vector<char> d;
			it = "sound/" + it;
			bw_sdl::sdl_readfile(it.c_str(), &d);
			if (d.size())
			{
				save_cache(it.c_str(), d.data(), d.size(), ctx);
				vfn.push_back(bw->externalCachePath + it);
			}

		}
		for (auto it : mss1)
		{
			std::vector<char> d;
			auto fn = "sound/" + it;
			bw_sdl::sdl_readfile(fn.c_str(), &d);
			if (d.size())
			{
				save_cache(fn.c_str(), d.data(), d.size(), ctx);
				auto nit = bw->externalCachePath + fn;
				vfn1.push_back(nit);
				auto bk = bw->add_sound_effect(nit);
				if (bk)
				{
					_game->msound[it] = bk;
				}
			}
		}

#endif // _WIN32
		bw->add_sonud(vfn);
		 
		_game->pay_sound_cb = [=](int n) {


		}; 
	}
}
 
