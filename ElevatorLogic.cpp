/*
 * ElevatorLogic.cpp
 *
 *  Created on: 20.06.2014
 *      Author: STSJR
 */

#include "ElevatorLogic.h"

#include <iostream>
#include <fstream>

#include "Interface.h"
#include "Person.h"
#include "Floor.h"
#include "Elevator.h"
#include "Event.h"
#include "ElevatorEnvironment.h"


#include "stdio.h"

using namespace std;

ElevatorLogic::ElevatorLogic() : EventHandler("ElevatorLogic") {
	
}

ElevatorLogic::~ElevatorLogic() {
}

void ElevatorLogic::Initialize(Environment &env) {

	env.RegisterEventHandler("Interface::Notify", this, &ElevatorLogic::HandleNotify);
	env.RegisterEventHandler("Elevator::Moving", this, &ElevatorLogic::HandleMoving);
	env.RegisterEventHandler("Elevator::Stopped", this, &ElevatorLogic::HandleStopped);
	env.RegisterEventHandler("Elevator::Opened", this, &ElevatorLogic::HandleOpened);
	env.RegisterEventHandler("Elevator::Closed", this, &ElevatorLogic::HandleClosed);
	env.RegisterEventHandler("Person::Entered", this, &ElevatorLogic::HandleEntered);
	env.RegisterEventHandler("Person::Exited", this, &ElevatorLogic::HandleExited);
	env.RegisterEventHandler("Environment::All", this, &ElevatorLogic::HandleAll);
	env.RegisterEventHandler("Elevator::Malfunction", this, &ElevatorLogic::HandleMalfunction);
	env.RegisterEventHandler("Elevator::Fixed", this, &ElevatorLogic::HandleFixed);

}




void ElevatorLogic::HandleAll(Environment &env, const Event &e) {




}


void ElevatorLogic::HandleNotify(Environment &env, const Event &e) {

	Interface *interf = static_cast<Interface*>(e.GetSender());
	Person *person = static_cast<Person*>(e.GetEventHandler());
	Loadable *loadable = interf->GetLoadable(0);

	if (loadable->GetType() == "Elevator") {

		Floor* floor = person->GetCurrentFloor();

		stop[person] = floor;

		Elevator *elev = FindElevator(person, interf, e.GetData());

		AddToQueue(elev, person);
		info[person].elevator = elev;

		ExecuteTask(elev, env);
	
	}
	else if (loadable->GetType() == "Floor") {

		Elevator* elev = person->GetCurrentElevator(); 

		stop[person] = static_cast<Floor*>(loadable);

		passengers[elev].insert(person);

		cout << stop[person]->GetId() << endl;

		ExecuteTask(elev, env);
		
	}
}

void ElevatorLogic::AddToQueue(Elevator *elev, Person *person) {

	if (state[elev].up) {
		if (elev->GetCurrentFloor()->IsAbove(stop[person]) || stop[person] == elev->GetCurrentFloor()) {
			for (list<Person*>::iterator iter = queue[elev].begin(); iter != queue[elev].end(); iter++) {
				if (stop[person]->IsBelow(stop[*iter])) {
					queue[elev].insert(iter, person);
					return;
				}
			}
		}
		else {
			for (list<Person*>::reverse_iterator iter = queue[elev].rbegin(); iter != queue[elev].rend(); iter++) {
				if (stop[person]->IsAbove(stop[*iter])) {
					queue[elev].insert(iter.base(), person);
					return;
				}
			}
		}
		
	}
	else {
		if (elev->GetCurrentFloor()->IsBelow(stop[person]) || stop[person] == elev->GetCurrentFloor()) {
			for (list<Person*>::iterator iter = queue[elev].begin(); iter != queue[elev].end(); iter++) {
				if (stop[person]->IsAbove(stop[*iter])) {
					queue[elev].insert(iter, person);
					return;
				}
			}
		}
		else {
			for (list<Person*>::reverse_iterator iter = queue[elev].rbegin(); iter != queue[elev].rend(); iter++) {
				if (stop[person]->IsBelow(stop[*iter])) {
					queue[elev].insert(iter.base(), person);
					return;
				}
			}
		}
	}


	queue[elev].push_back(person);
}

void ElevatorLogic::EraseFromQueue(Person *person) {

	Elevator *elev = info[person].elevator;

	for(list<Person*>::iterator iter = queue[elev].begin(); iter != queue[elev].end(); iter++) {
		if (*iter == person) {
			queue[elev].erase(iter);
			return;
		}
	}
}

Elevator* ElevatorLogic::FindElevator(Person *person, Interface *interf, string direction) {



	int bestScore = 0;
	int score;
	Elevator *favorite = NULL;

	for (int i = 0; i < interf->GetLoadableCount(); i++) {
		
		Elevator *elev = static_cast<Elevator*>(interf->GetLoadable(i));

		score = GetElevatorScore(elev, person, direction);

		if (score > bestScore) {
			favorite = elev;
			bestScore = score;
		}

	}

	if (favorite == NULL) {
		throw runtime_error("no elevator found");
	}

	return favorite;
}

int ElevatorLogic::GetElevatorScore(Elevator *elev, Person *person, string direction) {

	Floor *current = elev->GetCurrentFloor();
	int score = 0;

	if (!info[person].hasTraveled) {
		score += person->GetGiveUpTime();
		score -= GetDistance(elev->GetCurrentFloor(), stop[person]) / elev->GetSpeed();
	}
	else {
		score = 2;
	}

	score += (elev->GetMaxLoad() - state[elev].load);

	if (current->IsAbove(stop[person]) && state[elev].up) {
		score += 10;
		score *= 2;
		if (direction == "Up") {
			score *= 5;
		}
	}
	else if (current == stop[person]) {
		score *= 3;
		if (state[elev].up && direction == "Up") {
			score *= 2;
		}
		else if (!state[elev].up && direction == "Down") {
			score *= 2;
		}
	}
	else if (!state[elev].up) {
		score += 10;
		score *= 2;
		if (direction == "Down") {
			score *= 5;
		}
	}

	return score;
}

//Note that map::insert won't insert if key is already present in map

void ElevatorLogic::SetState(Elevator *elev) {

	pair<Elevator*,ElevatorState> elevState = {elev, {0, false, false, false, false, false, 0}};

	state.insert(elevState);
}

void ElevatorLogic::ExecuteTask(Elevator *elev, Environment &env) {

	Floor *current = elev->GetCurrentFloor();
	float pos = elev->GetPosition();

	Floor *nextStop = NULL;
	if (!queue[elev].empty()) {
		nextStop = stop[queue[elev].front()];
	}
	else {
		for (Person *person : passengers[elev]) {
			if (state[elev].up) {
				if (current->IsAbove(stop[person])) {
					nextStop = stop[person];
					break;
				}
			}
			else {
				if (current->IsBelow(stop[person])) {
					nextStop = stop[person];
					break;
				}
			}
		}
	}

	if (nextStop == NULL) {
		if (passengers[elev].empty()) {
			return;
		}
		else {
			nextStop = stop[*passengers[elev].begin()];
		}
	}
	
	state[elev].busy = true;
	cout << nextStop->GetId() << endl;


	if (!state[elev].moving && !state[elev].open && !state[elev].broken && !state[elev].overloaded && nextStop != NULL) {

		state[elev].moving = true;

		if (current == nextStop && pos < 0.51 && pos > 0.49) {
			cout << "wtf" << endl;
			env.SendEvent("Elevator::Stop", 0, this, elev);
		}

		else if (current == nextStop) {
			if (pos <= 0.49) {
				state[elev].up = true;
				env.SendEvent("Elevator::Up", 0, this, elev);
			}
			else {
				state[elev].up = false;
				env.SendEvent("Elevator::Down", 0, this, elev);
			}
		}
		else if (current->IsAbove(nextStop)) {
			state[elev].up = true;
			env.SendEvent("Elevator::Up", 0, this, elev);
		}
		else {
			state[elev].up = false;
			env.SendEvent("Elevator::Down", 0, this, elev);
		}
	}
}

void ElevatorLogic::HandleMoving(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	Floor *nextStop = stop[queue[elev].front()];

	state[elev].busy = true;

	state[elev].moving = true;
	Floor *current = elev->GetCurrentFloor();
	float pos = elev->GetPosition();

	if (current == nextStop && pos < 0.51 && pos > 0.49) {
		env.SendEvent("Elevator::Stop", 0, this, elev);
		return;
	}

	for (Person *person : passengers[elev]) {
		if ( current == stop[person] && pos < 0.51 && pos > 0.49) {
			env.SendEvent("Elevator::Stop", 0, this, elev);
			return;
		}
	}

	state[elev].movingID = env.SendEvent("Elevator::Moving", 1, elev);	
	
}

void ElevatorLogic::HandleMalfunction(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	env.SendEvent("Elevator::Stop", 0, this, elev);

	env.CancelEvent(state[elev].movingID);

	state[elev].broken = true;
}

void ElevatorLogic::HandleFixed(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	state[elev].broken = false;

	ExecuteTask(elev, env);
	
}

bool ElevatorLogic::IsOverloaded(Elevator *elev, Environment &env) {

	if (state[elev].load > elev->GetMaxLoad()) {
		env.SendEvent("Elevator::Beep", 0, this, elev);
		state[elev].overloaded = true;
		return true;
	}
	else {
		if (state[elev].overloaded) {
			state[elev].overloaded = false;
			env.SendEvent("Elevator::StopBeep", 0, this, elev);
		}
		return false;
	}
}

void ElevatorLogic::HandleEntered(Environment &env, const Event &e) {

	Person *person = static_cast<Person*>(e.GetSender());
	Elevator *elev = static_cast<Elevator*>(e.GetEventHandler());

	state[elev].load += person->GetWeight();
	EraseFromQueue(person);
	info[person].hasTraveled = true;

	if (!IsOverloaded(elev, env)) {
		
		CloseDoor(elev, env);	

	}
}

void ElevatorLogic::HandleExited(Environment &env, const Event &e) {

	Person *person = static_cast<Person*>(e.GetSender());
	Elevator *elev = static_cast<Elevator*>(e.GetEventHandler());

	state[elev].load -= person->GetWeight();
	
	passengers[elev].erase(person);

	if (!IsOverloaded(elev, env)) {
		
		CloseDoor(elev, env);	
	}
}

void ElevatorLogic::CloseDoor(Elevator *elev, Environment &env) {

	if (!state[elev].closing && !state[elev].overloaded) {
		state[elev].closing = true;

		env.SendEvent("Elevator::Close", 0, this, elev);
	}
}

void ElevatorLogic::HandleOpened(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	state[elev].open = true;

}

void ElevatorLogic::HandleStopped(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	state[elev].moving = false;

	if (!state[elev].broken) {
		state[elev].open = true;
		env.SendEvent("Elevator::Open", 0, this, elev);
	}
	
}

void ElevatorLogic::HandleClosed(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());
	
	state[elev].closing = false;
	state[elev].open = false;

	ExecuteTask(elev, env);

}

// Note: GetDistance return -1 if floors can't be connected by any elevator.

int ElevatorLogic::GetDistance(Floor *f1, Floor *f2) {

		cout << "distance "  << endl;

	if (f1 == f2) {
		return 0;
	}
		cout << "distance "  << endl;

	int distance = f1->GetHeight() + f2->GetHeight();

	if (distance % 2 != 0) {
		return -1;
	}

	distance /= 2;

	while (f1 != f2) {
		if (f1->IsAbove(f2)) {
			f1 = f1->GetAbove();
		}
		else {
			f1 = f1->GetBelow();
		}
		if (f1 != f2) {
			distance += f1->GetHeight();
		}
	}

	cout << "distance " << distance << endl;
	return distance;

}