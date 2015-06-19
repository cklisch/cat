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
	env.RegisterEventHandler("Person::Exiting", this, &ElevatorLogic::HandleExiting);
	env.RegisterEventHandler("Person::Entering", this, &ElevatorLogic::HandleEntering);
	env.RegisterEventHandler("Elevator::Closing", this, &ElevatorLogic::HandleClosing);
	env.RegisterEventHandler("Elevator::Beeping", this, &ElevatorLogic::HanndleBeeping);
	env.RegisterEventHandler("Interface::Interact", this, &ElevatorLogic::HandleInteract);
}

void ElevatorLogic::HandleAll(Environment &env, const Event &e) {

}

void ElevatorLogic::HandleInteract(Environment &env, const Event &e) {

	Elevator *sender = static_cast<Elevator*>(e.GetSender());

	if (sender->GetType() == "Elevator") {

		Elevator *elev = static_cast<Elevator*>(sender);
		Floor *current = elev->GetCurrentFloor();
		float pos = elev->GetPosition();

		for (Person *person : state[elev].requestors) {

			Floor *stop = request[person].floor;

			if (current == stop && pos < 0.51 && pos > 0.49) {
				Stop(env, elev, 0);
				return;
			}
		}
		env.SendEvent("Interface::Interact", 1, elev);
	}

	moving
}

void ElevatorLogic::HandleNotify(Environment &env, const Event &e) {
	
	Interface *interf = static_cast<Interface*>(e.GetSender());
	Person *person = static_cast<Person*>(e.GetEventHandler());
	Loadable *loadable = interf->GetLoadable(0);
	Elevator *elev;
	
	if (loadable->GetType() == "Elevator") {

		Floor *floor = person->GetCurrentFloor();

		request[person].floor = floor;
		request[person].interf = interf;

		elev = FindElevator(person, interf);

		if (e.GetData() == "") {
			if (elev->IsLowestFloor(floor)) {
				request[person].direction = "Up";
			}
			else if (elev->IsHighestFloor(floor)) {
				request[person].direction = "Down";
			}
		}
		else {
			request[person].direction = e.GetData();
		}

		state[elev].requestors.insert(person);

		request[person].elev = elev;

	}
	else if (loadable->GetType() == "Floor") {

		elev = person->GetCurrentElevator();

		state[elev].requestors.insert(person);

		request[person].floor = static_cast<Floor*>(loadable);
		request[person].direction = "";
				
	}

	if (elev->GetState() != Elevator::Idle && !state[elev].broken) {
		
		env.SendEvent("Interface::Interact", 0, elev);

	}
	else {
		ExecuteTask(elev, env);
	}
}

Elevator* ElevatorLogic::FindElevator(Person *person, Interface *interf) {

	Elevator *favorite = static_cast<Elevator*>(interf->GetLoadable(0));
	int bestTime = GetElevatorTime(favorite, person);
	

	for (int i = 1; i < interf->GetLoadableCount(); i++) {
		
		Elevator *elev = static_cast<Elevator*>(interf->GetLoadable(i));

		int time = GetElevatorTime(elev, person);

		if (time < bestTime && !state[elev].broken) {
			favorite = elev;
			bestTime = time;
		}

	}

	if (favorite == NULL) {
		throw runtime_error("no elevator found");
	}

	return favorite;
}

int ElevatorLogic::GetElevatorTime(Elevator *elev, Person *person) {

	Floor *current = elev->GetCurrentFloor();
	Floor *stop = person->GetCurrentFloor();
	int distance = 0;

	if (!state[elev].busy) {
		distance += GetDistance(current, stop);
	}
	else if (state[elev].up) {
		if (current->IsAbove(stop)) {
			distance += GetDistance(current, stop);
		}
		else {
			distance += (current->GetHeight() / 2);
			while (!elev->IsHighestFloor(current)) {
				current = current->GetAbove();
				distance += current->GetHeight();
			}
			distance -= (current->GetHeight() / 2);
			distance += GetDistance(current, stop);
		}
	}
	else if (!state[elev].up) {
		if (current->IsBelow(stop)) {
			distance += GetDistance(current, stop);
		}
		else {
			distance += (current->GetHeight() / 2);
			while (!elev->IsLowestFloor(current)) {
				current = current->GetBelow();
				distance += current->GetHeight();
			}
			distance -= (current->GetHeight() / 2);
			distance += GetDistance(current, stop);
		}
	}

	return (distance / elev->GetSpeed());
}

bool ElevatorLogic::CanExecuteTask(Elevator *elev) {

	return (!state[elev].moving && !state[elev].open && !state[elev].broken);
}

void ElevatorLogic::Stop(Environment &env, Elevator *elev, int delay) {

	if (!state[elev].stopping) {
		state[elev].stopping = true;
		env.SendEvent("Elevator::Stop", delay, this, elev);
	}
}

void ElevatorLogic::ExecuteTask(Elevator *elev, Environment &env) {

	Floor *current = elev->GetCurrentFloor();
	float pos = elev->GetPosition();
	int up_count = 0, down_count = 0;

	if (CanExecuteTask(elev)) {
		for (Person *person : state[elev].requestors) {

			Floor *stop = NULL;

			if (request[person].floor != NULL) {
				stop = request[person].floor;
			}
			cout << person->GetId() << endl;
			if (stop == NULL) {
				cout << "no floor" << endl;

				if (request[person].direction == "Up") {
					up_count++;
				} 
				else if (request[person].direction == "Down") {
					down_count++;
				}
			}
			else if (stop == current){
				if (pos < 0.51 && pos > 0.49) {
					Stop(env, elev, 0);
					return;
				}
				else if (pos >= 0.51) {
					down_count++;
				}
				else {
					up_count++;
				}
			}
			else if (current->IsAbove(stop)) {
				up_count++;
			}
			else if (current->IsBelow(stop)){
				down_count++;
			}
		}
		cout << "up " << up_count << "down " << down_count << endl;
		if (state[elev].up) {
			if (up_count > 0) {
				env.SendEvent("Elevator::Up", 0, this, elev);
				state[elev].up = true;
				state[elev].moving = true;
			}
			else if (down_count > 0) {
				state[elev].moving = true;	
				state[elev].up = false;
		
				env.SendEvent("Elevator::Down", 0, this, elev);
			}
		}
		else if (!state[elev].up) {
			if (down_count > 0) {
				state[elev].moving = true;
				env.SendEvent("Elevator::Down", 0, this, elev);
				state[elev].up = false;
			}
			else if (up_count > 0) {
				state[elev].moving = true;
				env.SendEvent("Elevator::Up", 0, this, elev);
				state[elev].up = true;
			}
			
		}		
	}
}

void ElevatorLogic::HandleMoving(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	state[elev].busy = true;

	state[elev].moving = true;

	env.SendEvent("Interface::Interact", 0, elev);



	
}

void ElevatorLogic::HandleMalfunction(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	env.SendEvent("Elevator::Stop", 0, this, elev);

	env.CancelEvent(state[elev].movingID);

	state[elev].broken = true;

	for (auto person : state[elev].requestors) {
		if (request[person].interf != NULL) {
			env.SendEvent("Interface::Notify", 0, request[person].interf, person, request[person].direction);
		}
	}
}

void ElevatorLogic::HandleFixed(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	state[elev].broken = false;

	ExecuteTask(elev, env);
	
}


void ElevatorLogic::HandleEntered(Environment &env, const Event &e) {

	Person *person = static_cast<Person*>(e.GetSender());
	Elevator *elev = static_cast<Elevator*>(e.GetEventHandler());

	state[elev].load += person->GetWeight();

	
	if (state[elev].load > elev->GetMaxLoad() && !state[elev].beeping) {
		state[elev].beeping = true;
		env.CancelEvent(state[elev].closingID);
		env.SendEvent("Elevator::Beep", 0, this, elev);
	}
	else {
		CloseDoor(env, elev, 0);
	}


}

void ElevatorLogic::HandleExited(Environment &env, const Event &e) {

	Person *person = static_cast<Person*>(e.GetSender());
	Elevator *elev = static_cast<Elevator*>(e.GetEventHandler());
	
	state[elev].load -= person->GetWeight();


	if (state[elev].beeping && (state[elev].load <= elev->GetMaxLoad())) {
		state[elev].beeping = false;
		env.SendEvent("Elevator::StopBeep", 0, this, elev);
	}

	CloseDoor(env, elev, 0);

}

void ElevatorLogic::HandleEntering(Environment &env, const Event &e) {

	Person *person = static_cast<Person*>(e.GetSender());
	Elevator *elev = static_cast<Elevator*>(e.GetEventHandler());

	// this requested floor was sereved
	request[person].floor = NULL;

	// check if perosn is entering correct elevator
	if (request[person].elev != elev) {

		state[request[person].elev].requestors.erase(person);
		state[elev].requestors.insert(person);

	}

	// no elevator reqested anymore
	request[person].elev = NULL;
	request[person].interf = NULL;


	
		
	env.CancelEvent(state[elev].closingID);

	state[elev].closingID = 0;

	

}

void ElevatorLogic::HandleExiting(Environment &env, const Event &e) {

	Person *person = static_cast<Person*>(e.GetSender());
	Elevator *elev = static_cast<Elevator*>(e.GetEventHandler());

	state[elev].requestors.erase(person);

	// env.CancelEvent(state[elev].closingID);

	// state[elev].closingID = 0;

}

void ElevatorLogic::HanndleBeeping(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	CloseDoor(env, elev, 1);

}

void ElevatorLogic::HandleClosing(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	state[elev].closing = true;

}

void ElevatorLogic::CloseDoor(Environment &env, Elevator *elev, int delay) {

	if (!state[elev].closing && !state[elev].beeping && state[elev].open) {

		state[elev].closingID = env.SendEvent("Elevator::Close", delay, this, elev);
	}

}

void ElevatorLogic::HandleOpened(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	state[elev].open = true;

	CloseDoor(env, elev, 0);

}

void ElevatorLogic::HandleStopped(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	state[elev].stopping = false;

	cout << "stop at floor " << elev->GetCurrentFloor()->GetId() << endl;
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

	if (f1 == f2) {
		return 0;
	}

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
	return distance;
} 