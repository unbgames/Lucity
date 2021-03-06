#include "NPCTutorial.h"
#include "Game.h"
#include "GameData.h"

#include "Sprite.h"
#include "FOV.h"
#include "Reaction.h"
#include "Attack.h"
#include "Monster.h"
#include "MainObject.h"
#include "Animator.h"

#include <math.h>
#include "config.h"

NPCTutorial::NPCTutorial(GameObject& associated, Personality p) : Character(associated, p.GetName()) {
	underTutorialControl = true;
	SetHealth(1);
	person = p;
	offsetT = pow(-1,rand()%2)*(rand()%51)/100;
	damageCD = 0.5;
	destination = associated.box.GetPos();
}

NPCTutorial::~NPCTutorial() {

}

void NPCTutorial::Start() {
	GameObject* go = new GameObject();
	go->AddComponent(new FOV(*go, associated, 100));
	Game::GetInstance().GetCurrentState().AddObject(go, "MAIN");
}

void NPCTutorial::Damage(int damage) {
	if(GetHealth() > 0) {
		SetHealth(GetHealth()-damage);
		if(GetHealth() < 1) {
			associated.RequestDelete();
			if(!associated.GetComponent("Monster")) {
				GameObject* go = new GameObject();
				Sprite* sp = new Sprite(*go, ASSETS_PATH("/img/characters/" + person.GetName() + "/dead" + GetDirection() + ".png"), 4, 0.2, false);
				sp->SetScale(Vec2(2, 2));
				go->AddComponent(sp);
				go->box.SetCenter(associated.box.GetCenter());
				Game::GetInstance().GetCurrentState().AddObject(go, "MAIN");
			}
		}
	}
}

void NPCTutorial::Update(float dt) {
	if(!GameData::paused) {
		actionT.Update(dt);
		damageT.Update(dt);

		if (underTutorialControl) {
			if (GetAction() == IDLE) {
				if (actionT.Get() > person.GetTime(IDLE) + offsetT) {
					actionT.Restart();
					offsetT = pow(-1, rand() % 2)*(rand() % 51) / 100;
				}
			}
			else if (GetAction() == WALK) {
				Vec2 pos = associated.box.GetCenter();
				Vec2 dest = NPCTutorial::destination;
				float cos = pos.GetCos(dest);
				if (cos != cos)
					cos = 0;
				float sin = pos.GetSin(dest);
				if (sin != sin)
					sin = 0;
				Vec2 speed = Vec2(person.GetNormalSpeed() * cos, person.GetNormalSpeed() * sin);
				if ((pos.x + speed.x*0.2 + offsetT > dest.x && dest.x > pos.x) || (pos.x + speed.x*0.2 + offsetT < dest.x && dest.x < pos.x))
					pos.x = dest.x;
				else
					pos.x += speed.x*0.2 + offsetT;
				if ((pos.y + speed.y*0.2 + offsetT > dest.y && dest.y > pos.y) || (pos.y + speed.y*0.2 + offsetT < dest.y && dest.y < pos.y))
					pos.y = dest.y;
				else
					pos.y += speed.y*0.2 + offsetT;
				associated.box.SetCenter(pos);
				if (pos ^= dest) {
					actionT.Restart();
					scared = true;
					SetAction(SHOCK);
					offsetT = pow(-1, rand() % 2)*(rand() % 51) / 100;
					GameObject* go = new GameObject();
					go->AddComponent(new Reaction(*go, associated, "interest"));
					Game::GetInstance().GetCurrentState().AddObject(go, "MISC");
				}
			}
			else if (GetAction() == SHOCK) {
				if (actionT.Get() > person.GetTime(SHOCK) + offsetT) {
					actionT.Restart();
					SetAction(PANIC);
					offsetT = pow(-1, rand() % 2)*(rand() % 51) / 100;
					SetAngleDirection(associated.box.GetCenter().GetAngle(Vec2(712, 420)));
					GameObject* go = new GameObject();
					go->AddComponent(new Reaction(*go, associated, "fear"));
					Game::GetInstance().GetCurrentState().AddObject(go, "MISC");
				}
			}
			else if (GetAction() == PANIC) {
				if (actionT.Get() > person.GetTime(PANIC) + offsetT) {
					actionT.Restart();
					offsetT = pow(-1, rand() % 2)*(rand() % 51) / 100;
					SetAction(IDLE);
				}
				else {
					associated.box.SetPos(associated.box.GetPos() + (Vec2(Vec2::Cos(GetAngleDirection()), Vec2::Sin(GetAngleDirection()))*GetSpeed()*dt));
				}
			}
		} else {
			if(GetAction() == IDLE) {
				if(actionT.Get() > person.GetTime(IDLE)+offsetT) {
					actionT.Restart();
					offsetT = pow(-1,rand()%2)*(rand()%51)/100;
					SetAction(WALK);
					SetSpeed(person.GetNormalSpeed());
					SetAngleDirection(rand()%360);
				}
			}
			else if(GetAction() == WALK) {
				if(actionT.Get() > person.GetTime(WALK)+offsetT) {
					actionT.Restart();
					offsetT = pow(-1,rand()%2)*(rand()%51)/100;
					SetAction(IDLE);
				}else{
					associated.box.SetPos(associated.box.GetPos()+(Vec2(Vec2::Cos(GetAngleDirection()), Vec2::Sin(GetAngleDirection()))*GetSpeed()*dt));
				}
			}
			else if(GetAction() == SHOCK) {
				if(actionT.Get() > person.GetTime(SHOCK)+offsetT) {
					actionT.Restart();
					offsetT = pow(-1,rand()%2)*(rand()%51)/100;
					if(scared) {
						SetAction(PANIC);
						SetSpeed(person.GetPanicSpeed());
						SetAngleDirection(GetAngleDirection()-180);

						GameObject* go = new GameObject();
						if(GetDirection() == "NE" || GetDirection() == "SE")
							go->AddComponent(new Sprite(*go, ASSETS_PATH("/img/effects/dustE.png"), 6, 0.05, false, 0.3));
						else if(GetDirection() == "NW" || GetDirection() == "SW")
							go->AddComponent(new Sprite(*go, ASSETS_PATH("/img/effects/dustW.png"), 6, 0.05, false, 0.3));
						go->box.SetCenter(associated.box.GetCenter()+(Vec2(Vec2::Cos(GetAngleDirection()+180), Vec2::Sin(GetAngleDirection()+180))*30));
						Game::GetInstance().GetCurrentState().AddObject(go, "EFFECT");
					}
					else {
						SetAction(WALK);
						SetSpeed(person.GetNormalSpeed());
						SetAngleDirection(rand()%360);
					}
				}
			}
			else if(GetAction() == PANIC) {
				if(actionT.Get() > person.GetTime(PANIC)+offsetT) {
					actionT.Restart();
					offsetT = pow(-1,rand()%2)*(rand()%51)/100;
					SetAction(IDLE);
				}else{
					associated.box.SetPos(associated.box.GetPos()+(Vec2(Vec2::Cos(GetAngleDirection()), Vec2::Sin(GetAngleDirection()))*GetSpeed()*dt));
				}
			}
		}
	}

	if(associated.box.x < 0)
		associated.box.x = 0;
	if(associated.box.x+associated.box.w > GameData::mapSize.x)
		associated.box.x = GameData::mapSize.x-associated.box.w;
	if(associated.box.y < GameData::upperLimit)
		associated.box.y = GameData::upperLimit;
	if(associated.box.y+associated.box.h > GameData::mapSize.y)
		associated.box.y = GameData::mapSize.y-associated.box.h;
}

void NPCTutorial::WalkToPoint(Vec2 destination, float dt) {
	initPos = associated.box.GetPos();
	SetAngleDirection(associated.box.GetCenter().GetAngle(destination));
	SetSpeed(person.GetNormalSpeed());
	NPCTutorial::destination = destination;
	SetAction(WALK);
}

void NPCTutorial::NotifyCollision(GameObject& other) {
	if(other.GetComponent("Attack")) {
		Attack* attack = (Attack*) other.GetComponent("Attack");
		actionT.Restart();
		offsetT = pow(-1,rand()%2)*(rand()%51)/100;
		scared = true;
		SetAction(PANIC);
		SetAngleDirection(associated.box.GetCenter().GetAngle(other.box.GetCenter()));
		Damage(attack->GetDamage());
	}
	else if(other.GetComponent("NPCTutorial")) {
		if(other.GetComponent("Monster")) {
			Monster* mon = (Monster*) other.GetComponent("Monster");
			if(mon->IsTransformed()) {
				if(GetAction() != PANIC && GetAction() != SHOCK) {
					actionT.Restart();
					offsetT = pow(-1,rand()%2)*(rand()%51)/100;
					scared = true;
					SetAction(SHOCK);
					SetAngleDirection(associated.box.GetCenter().GetAngle(other.box.GetCenter()));
					GameObject* go = new GameObject();
					go->AddComponent(new Reaction(*go, associated, "fear"));
					Game::GetInstance().GetCurrentState().AddObject(go, "MISC");
				}
			}
		}
		else {
			NPCTutorial* npc = (NPCTutorial*) other.GetComponent("NPCTutorial");
			if(person.IsAfraid(npc->GetName())) {
				if(GetAction() != PANIC && GetAction() != SHOCK) {
					actionT.Restart();
					offsetT = pow(-1,rand()%2)*(rand()%51)/100;
					scared = true;
					SetAction(SHOCK);
					SetAngleDirection(associated.box.GetCenter().GetAngle(other.box.GetCenter()));
					GameObject* go = new GameObject();
					go->AddComponent(new Reaction(*go, associated, "fear"));
					Game::GetInstance().GetCurrentState().AddObject(go, "MISC");
				}
			}
			else if(person.IsInterested(npc->GetName())) {
				if(GetAction() == IDLE) {
					actionT.Restart();
					offsetT = pow(-1,rand()%2)*(rand()%51)/100;
					scared = false;
					SetAction(SHOCK);
					SetAngleDirection(associated.box.GetCenter().GetAngle(other.box.GetCenter()));
					GameObject* go = new GameObject();
					go->AddComponent(new Reaction(*go, associated, "interest"));
					Game::GetInstance().GetCurrentState().AddObject(go, "MISC");
				}
			}
		}
	}
	else if(other.GetComponent("MainObject")) {
		MainObject* mo = (MainObject*) other.GetComponent("MainObject");
		if(person.IsAfraid(mo->GetName())) {
			if(GetAction() != PANIC && GetAction() != SHOCK) {
				actionT.Restart();
				offsetT = pow(-1,rand()%2)*(rand()%51)/100;
				scared = true;
				SetAction(SHOCK);
				SetAngleDirection(associated.box.GetCenter().GetAngle(other.box.GetCenter()));
				GameObject* go = new GameObject();
				go->AddComponent(new Reaction(*go, associated, "fear"));
				Game::GetInstance().GetCurrentState().AddObject(go, "MISC");
			}
		}
		else if(person.IsInterested(mo->GetName())) {
			if(GetAction() == IDLE) {
				actionT.Restart();
				offsetT = pow(-1,rand()%2)*(rand()%51)/100;
				scared = false;
				SetAction(SHOCK);
				SetAngleDirection(associated.box.GetCenter().GetAngle(other.box.GetCenter()));
				GameObject* go = new GameObject();
				go->AddComponent(new Reaction(*go, associated, "interest"));
				Game::GetInstance().GetCurrentState().AddObject(go, "MISC");
			}
		}
	}
}

bool NPCTutorial::Is(std::string type) {
	return (type == "NPCTutorial" || Character::Is(type));
}

void NPCTutorial::SetAngleDirection(int angle) {
	Character::SetAngleDirection(angle);
	if(GetAngleDirection() > 360)
		Character::SetAngleDirection(GetAngleDirection()-360);
	else if(GetAngleDirection() < 0)
		Character::SetAngleDirection(GetAngleDirection()+360);

	if(GetAngleDirection() >= 0 && GetAngleDirection() < 90)
		SetDirection("SE");
	else if(GetAngleDirection() >= 90 && GetAngleDirection() < 180)
		SetDirection("SW");
	else if(GetAngleDirection() >= 180 && GetAngleDirection() < 270)
		SetDirection("NW");
	else if(GetAngleDirection() >= 270 && GetAngleDirection() < 360)
		SetDirection("NE");
}

void NPCTutorial::SetPerson(Personality p) {
	person = p;
	Animator* anim = (Animator*) associated.GetComponent("Animator");
	anim->RebuildSprites(p.GetName());
}

std::string NPCTutorial::GetName() {
	return person.GetName();
}

void NPCTutorial::ToggleTutorialControl() {

	underTutorialControl = !underTutorialControl;

}

bool NPCTutorial::GetTutorialControl() {

	return underTutorialControl;

}
