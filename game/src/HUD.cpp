#include "HUD.h"
#include "Game.h"
#include "GameData.h"

#include "Text.h"
#include "Sprite.h"
#include "Player.h"
#include "Book.h"
#include "CameraFollower.h"

#include <math.h>
#include "config.h"

HUD::HUD(GameObject& associated) : Component(associated) {
	healthTrack = 0;
}

HUD::~HUD() {
	if(!player_info.expired())
		player_info.lock()->RequestDelete();

	for(int i = health.size()-1; i >= 0; i--)
		if(!health[i].expired())
			health[i].lock()->RequestDelete();
	health.clear();

	for(int i = attack.size()-1; i >= 0; i--)
		if(!attack[i].expired())
			attack[i].lock()->RequestDelete();
	attack.clear();

	if(!selector.expired())
		selector.lock()->RequestDelete();

	for(int i = timer.size()-1; i >= 0; i--)
		if(!timer[i].expired())
			timer[i].lock()->RequestDelete();
	timer.clear();

	if(!monsters.expired())
		monsters.lock()->RequestDelete();
	if(!monstersTXT.expired())
		monstersTXT.lock()->RequestDelete();

	if(!npcs.expired())
		npcs.lock()->RequestDelete();
	if(!npcsTXT.expired())
		npcsTXT.lock()->RequestDelete();
}

void HUD::Start() {
	GameObject* go;
	Sprite* sp;

	go = new GameObject();
	sp = new Sprite(*go, ASSETS_PATH("/img/HUD/player_info.png") );
	sp->SetScale(Vec2(3, 3));
	go->AddComponent(sp);
	go->AddComponent(new CameraFollower(*go, Vec2(5, 5)));
	player_info = Game::GetInstance().GetCurrentState().AddObject(go, "GUI");

	//Health Points
	for(int i = 0; i < 5; i++) {
		go = new GameObject();
		sp = new Sprite(*go, ASSETS_PATH("/img/HUD/health.png"), 5, 0, false);
		sp->SetScale(Vec2(3, 3));
		go->AddComponent(sp);
		go->AddComponent(new CameraFollower(*go, Vec2(96 + i*24, 15)));
		health.emplace_back(Game::GetInstance().GetCurrentState().AddObject(go, "GUI"));
	}

	//Attack Modes
	for(int i = 0; i < 3; i++) {
		go = new GameObject();
		sp = new Sprite(*go, ASSETS_PATH("/img/HUD/bubbles_on.png") );
		sp->SetScale(Vec2(2, 2));
		go->AddComponent(sp);
		go->AddComponent(new CameraFollower(*go, Vec2(110 + i*35, 50)));
		attack.emplace_back(Game::GetInstance().GetCurrentState().AddObject(go, "GUI"));
	}

	//Attack Selector
	go = new GameObject();
	sp = new Sprite(*go, ASSETS_PATH("/img/HUD/selector.png") );
	sp->SetScale(Vec2(2, 2));
	go->AddComponent(sp);
	go->AddComponent(new CameraFollower(*go, Vec2(110 + 35, 50)));
	selector = Game::GetInstance().GetCurrentState().AddObject(go, "GUI");

	//Timer Base
	go = new GameObject();
	sp = new Sprite(*go, ASSETS_PATH("/img/HUD/timer_base.png"), 1);
	sp->SetScale(Vec2(4, 4));
	go->AddComponent(sp);
	go->AddComponent(new CameraFollower(*go, Vec2(1024 - go->box.w - 10, 10)));
	timer.emplace_back(Game::GetInstance().GetCurrentState().AddObject(go, "GUI"));

	//Timer Fill
	go = new GameObject();
	sp = new Sprite(*go, ASSETS_PATH("/img/HUD/timer_fill.png"), 10, 0, false);
	sp->SetScale(Vec2(4, 4));
	go->AddComponent(sp);
	go->AddComponent(new CameraFollower(*go, Vec2(1024 - go->box.w - 10, 10)));
	timer.emplace_back(Game::GetInstance().GetCurrentState().AddObject(go, "GUI"));

	//Timer Pulse
	go = new GameObject();
	sp = new Sprite(*go, ASSETS_PATH("/img/HUD/timer_pulse.png"), 9, 0.11);
	sp->SetScale(Vec2(4, 4));
	go->AddComponent(sp);
	go->AddComponent(new CameraFollower(*go, Vec2(1024 - go->box.w - 10, 10)));
	timer.emplace_back(Game::GetInstance().GetCurrentState().AddObject(go, "GUI"));

	//NPCs
	go = new GameObject();
	sp = new Sprite(*go, ASSETS_PATH("/img/characters/old/idleSW.png"), 4, 0.2);
	go->AddComponent(sp);
	go->AddComponent(new CameraFollower(*go, Vec2(1024-go->box.w-10, 100)));
	npcs = Game::GetInstance().GetCurrentState().AddObject(go, "GUI");

	go = new GameObject();
	go->AddComponent(new Text(*go, ASSETS_PATH("/font/Sabo-Filled.ttf"), 36, "x", SDL_Color {}, Text::SOLID));
	go->AddComponent(new CameraFollower(*go, Vec2(1024-npcs.lock()->box.w-go->box.w-75, 100+npcs.lock()->box.h-go->box.h)));
	npcsTXT = Game::GetInstance().GetCurrentState().AddObject(go, "GUI");

	//Monsters
	go = new GameObject();
	sp = new Sprite(*go, ASSETS_PATH("/img/characters/monster/idleSW.png"), 6, 0.2);
	go->AddComponent(sp);
	go->AddComponent(new CameraFollower(*go, Vec2(1024-go->box.w-10, 140)));
	monsters = Game::GetInstance().GetCurrentState().AddObject(go, "GUI");

	go = new GameObject();
	go->AddComponent(new Text(*go, ASSETS_PATH("/font/Sabo-Filled.ttf"), 36, "x", SDL_Color {}, Text::SOLID));
	go->AddComponent(new CameraFollower(*go, Vec2(1024-monsters.lock()->box.w-go->box.w-70, 140+monsters.lock()->box.h-go->box.h)));
	monstersTXT = Game::GetInstance().GetCurrentState().AddObject(go, "GUI");
}

void HUD::Update(float dt) {
	if(!GameData::player.expired()) {
		//Health
		Player* p = (Player*) GameData::player.lock()->GetComponent("Player");
		if(p) {
			if(p->GetHealth() < healthTrack && p->GetHealth() > 0) {
				for(unsigned int i = healthTrack-1; i < health.size(); i++) {
					Sprite* s = (Sprite*) health[i].lock()->GetComponent("Sprite");
					s->SetFrameTime(0.2);
				}
			}
			healthTrack = p->GetHealth();
		}

		//Attack
		Book* b = (Book*) GameData::book.lock()->GetComponent("Book");
		if(b) {
			CameraFollower* c1 = (CameraFollower*) selector.lock()->GetComponent("CameraFollower");
			if(b->GetAttackMode() == "bubbles") {
				CameraFollower* c2 = (CameraFollower*) attack[0].lock()->GetComponent("CameraFollower");
				c1->SetTruePos(c2->GetTruePos() + attack[0].lock()->box.GetSize()/2 - Vec2(selector.lock()->box.w/2, selector.lock()->box.h/2));
			}
			else if(b->GetAttackMode() == "fireball") {
				CameraFollower* c2 = (CameraFollower*) attack[1].lock()->GetComponent("CameraFollower");
				c1->SetTruePos(c2->GetTruePos() + attack[1].lock()->box.GetSize()/2 - Vec2(selector.lock()->box.w/2, selector.lock()->box.h/2));
			}
			else if(b->GetAttackMode() == "bind") {
				CameraFollower* c2 = (CameraFollower*) attack[2].lock()->GetComponent("CameraFollower");
				c1->SetTruePos(c2->GetTruePos() + attack[2].lock()->box.GetSize()/2 - Vec2(selector.lock()->box.w/2, selector.lock()->box.h/2));
			}

			for(unsigned int i = 0; i < attack.size(); i++) {
				Sprite* s = (Sprite*) attack[i].lock()->GetComponent("Sprite");

				std::string path = ASSETS_PATH("/img/HUD/");
				std::string mode;
				if(i == 0) {
					if(b->GetAttackMode() == "bubbles")
						mode = "bubbles_on";
					else
						mode = "bubbles_off";
				}
				else if(i == 1) {
					if(b->GetAttackMode() == "fireball")
						mode = "fireball_on";
					else
						mode = "fireball_off";
				}
				else if(i == 2) {
					if(b->GetAttackMode() == "bind")
						mode = "bind_on";
					else
						mode = "bind_off";
				}
				s->Open(path+mode+".png");
			}
		}

		//Timer
		Sprite* s = (Sprite*) timer[1].lock()->GetComponent("Sprite");
		if(s)
			s->SetFrame((int)(GameData::eventT.Get()/(GameData::eventCD/10)));

		Text* txt;

		//Number of Civilians
		txt = (Text*) npcsTXT.lock()->GetComponent("Text");
		if(txt) {
			char ch[3];
			sprintf(ch, "%d", GameData::nCivilians);
			std::string n = ch;
			std::string nc;
			std::string x = "x";
			if(GameData::nCivilians > 9)
				nc = std::string(n);
			else
				nc = "0" + std::string(n);
			std::string text = nc + x;
			txt->SetText(text);	
		}

		//Number of Monsters
		txt = (Text*) monstersTXT.lock()->GetComponent("Text");
		if(txt) {
			char ch[3];
			sprintf(ch, "%d", GameData::nMonsters);
			std::string n = ch;
			std::string nm;
			std::string x = "x";
			if(GameData::nMonsters > 9)
				nm = std::string(n);
			else
				nm = "0" + std::string(n);
			std::string text = nm + x;
			txt->SetText(text);	
		}
	}
	else {
		if(healthTrack >= 0) {
			for(int i = 0; i < 5; i++) {
				Sprite* s = (Sprite*) health[i].lock()->GetComponent("Sprite");
				s->SetFrameTime(0.2);
			}
			healthTrack = -1;
		}
	}
}

bool HUD::Is(std::string type) {
	return (type == "HUD");
}
