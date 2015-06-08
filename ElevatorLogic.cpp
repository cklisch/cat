/*
 * ElevatorLogic.cpp
 *
 *  Created on: 20.06.2014
 *      Author: STSJR
 */

#include "ElevatorLogic.h"

#include <iostream>

#include "Interface.h"
#include "Person.h"
#include "Floor.h"
#include "Elevator.h"
#include "Event.h"
#include "Environment.h"

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

	// Loadable *loadable = interf->GetLoadable(0);

	// if (loadable->GetType() == "Floor") {

	// }


	log += to_string(e.GetTime());
	log += " ";
	log += e.GetEvent();
	log.push_back('\n');
}


void ElevatorLogic::HandleNotify(Environment &env, const Event &e) {

	// FILE *in;
	// char buff[512];

	// if(!(in = popen("cd .. && cd loc && ls", "r"))) {
	// 	return;
	// }
	// string out;

	// while (fgets(buff, sizeof(buff), in) != NULL) {
	// 	out += buff;
	// }

	// throw overflow_error(out);


	//system("ls");



	Interface *interf = static_cast<Interface*>(e.GetSender());
	Person *person = static_cast<Person*>(e.GetEventHandler());

	Loadable *loadable = interf->GetLoadable(0);

	if (loadable->GetType() == "Elevator") {

		log += "# initial floor ";
		log += to_string(person->GetCurrentFloor()->GetId());
		log += " # final floor";
		log += to_string(person->GetFinalFloor()->GetId());

		log += " #elevator floor ";

		Elevator *elev = FindElevator(person, interf);

		log += to_string(elev->GetCurrentFloor()->GetId());

		cout << "elevator found " << elev->GetId() << endl; 


		AddToQueue(person, elev);

		

		if (SetNextStop(elev)) {
			ExecuteTask(elev, env);
		}
	}
	else if (loadable->GetType() == "Floor") {

		Elevator* elev = person->GetCurrentElevator();

		if (task[elev].needsChange) {
			needsChange = false;
			task[elev].nextStop = static_cast<Floor*>(loadable);
			state[elev].requested = true;

			ExecuteTask(elev, env);
		}

		log += "# floor to";
		log += to_string(loadable->GetId());
		// Elevator *elev = person->GetCurrentElevator();
		
		// AddToQueue(elev, person);

		

		// if (SetNextStop(elev)) {
		// 	ExecuteTask(elev, env);
		// }
	}
}

Elevator* ElevatorLogic::FindElevator(Person *person, Interface *interf) {

	Floor *floor = person->GetCurrentFloor();

	set<Elevator*> possible;	
		
	for (int j = 0; j < interf->GetLoadableCount(); j++) {
		Elevator *e = static_cast<Elevator*>(interf->GetLoadable(j));

		SetTask(e);
		SetState(e);

		possible.insert(e);
	}
	

	for (Elevator* elev : possible) {
		if (elev->GetCurrentFloor() == floor) {
			return elev;
		}	
	}

	return *possible.begin();
}

void ElevatorLogic::AddToQueue(Person *person, Elevator *elev) {

	cout << "add task" << endl;
	task[elev].queue.push_back(person);
	cout << "added" << endl;

}

//Note that map::insert won't insert if key is already present in map

void ElevatorLogic::SetTask(Elevator *elev) {

	Floor *nextStop = NULL;
	list<Person*> queue;
	set<Person*> passengers;
	pair<Elevator*,ElevatorTask> elevTask = {elev, {nextStop, false, false, queue} };

	task.insert(elevTask);
	cout << 1 << endl;	

}

void ElevatorLogic::SetState(Elevator *elev) {

	set<Person*> passengers;

	pair<Elevator*,ElevatorState> elevState = {elev, {passengers, false, false, false, false}};
	state.insert(elevState);
}

bool ElevatorLogic::SetNextStop(Elevator *elev) {

	if (task[elev].queue.size() == 0 || task[elev].busy) {
		return false;
	}

	state[elev].requested = true;

	Person *person = task[elev].queue.front();

	cout << "to person " << person->GetId() << " queue size " << task[elev].queue.size() << endl;

	if (person->GetCurrentElevator() == elev) {
		Floor *target = person->GetFinalFloor();
		if (elev->HasFloor(target)) {
			task[elev].nextStop = target;	
		}
		else {
			task[elev].needsChange = true;
			return false;
		}
	}
	else {
		task[elev].nextStop = person->GetCurrentFloor();
	}

	return true;
}

void ElevatorLogic::ExecuteTask(Elevator *elev, Environment &env) {

	if (!state[elev].broken && !state[elev].overloaded && !state[elev].beeping && state[elev].requested) {

		cout << "exec" << endl;
		task[elev].busy = true;

		if (elev->GetCurrentFloor() == task[elev].nextStop && elev->GetPosition() < 0.51 && elev->GetPosition() > 0.49) {
			env.SendEvent("Elevator::Open", 0, this, elev);
		} 
		else if (elev->GetCurrentFloor() == task[elev].nextStop) {
			if (elev->GetPosition() < 0.5 && elev->GetPosition()) {
				env.SendEvent("Elevator::Up", 0, this, elev);
			}
			else {
				env.SendEvent("Elevator::Down", 0, this, elev);
			}
		}
		else if (elev->GetCurrentFloor()->IsAbove(task[elev].nextStop)) {

			env.SendEvent("Elevator::Up", 0, this, elev);
		}
		else {
			env.SendEvent("Elevator::Down", 0, this, elev);
		}
	}	
	
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

int ElevatorLogic::GetWeight(set<Person*> passengers) {

	int load = 0;
	for (Person* person : passengers) {
		load += person->GetWeight();
	}
	return load;
}

bool ElevatorLogic::IsOverloaded(Elevator *elev) {

	if (GetWeight(state[elev].passengers) > elev->GetMaxLoad()) {
		state[elev].overloaded = true;
		return true;
	}
	else {
		state[elev].overloaded = false;
		return false;
	}
}

void ElevatorLogic::HandleEntered(Environment &env, const Event &e) {

	Person *person = static_cast<Person*>(e.GetSender());
	Elevator *elev = static_cast<Elevator*>(e.GetEventHandler());

	EraseFromQueue(person, elev);

	AddToQueue(person, elev);

	state[elev].passengers.insert(person);

	if (IsOverloaded(elev)) {
		env.SendEvent("Elevator::Beep", 0, this, elev);
		state[elev].beeping = true;
	}
	else {
		if (state[elev].beeping) {
			env.SendEvent("Elevator::StopBeep", 0, this, elev);
			state[elev].beeping = false;
		}
		env.SendEvent("Elevator::Close", 0, this, elev);	
	}

}

void ElevatorLogic::HandleExited(Environment &env, const Event &e) {

	Person *person = static_cast<Person*>(e.GetSender());
	Elevator *elev = static_cast<Elevator*>(e.GetEventHandler());

	EraseFromQueue(person, elev);

	state[elev].passengers.erase(person);


	if (!IsOverloaded(elev)) {
		if (state[elev].beeping) {
			env.SendEvent("Elevator::StopBeep", 0, this, elev);
			state[elev].beeping = false;
		}
		env.SendEvent("Elevator::Close", 0, this, elev);	
	}
}



void ElevatorLogic::EraseFromQueue(Person *person, Elevator *elev) {

	for (list<Person*>::iterator it = task[elev].queue.begin(); it != task[elev].queue.end(); it++) {
		if (*it == person) {
			cout << "entered: " << "erase " << (*it)->GetId() << endl;
			task[elev].queue.erase(it);
			cout << "erased" << endl;
			break;

		}
		
	}
}

void ElevatorLogic::HandleMoving(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	Floor *current = elev->GetCurrentFloor();

	if (current == task[elev].nextStop && elev->GetPosition() < 0.51 && elev->GetPosition() > 0.49) {
			cout << elev->GetPosition() << endl;

		env.SendEvent("Elevator::Stop", 0, this, elev);
	}
	else {
		if (elev->GetState() == Elevator::Up && elev->IsHighestFloor(current) && elev->GetPosition() > 0.49) {
			env.SendEvent("Elevator::Stop", 0, this, elev);
			cout << "throw" << endl;

			throw runtime_error(log);
			//throw runtime_error(to_string(task[elev].queue.size()) + " per cur " + to_string(task[elev].queue.front()->GetCurrentFloor()->GetId()) + " pers final " + to_string(task[elev].queue.front()->GetFinalFloor()->GetId()) + " elev cur " + to_string(elev->GetCurrentFloor()->GetId()) + " elev next" + to_string(task[elev].nextStop->GetId()) + " is on elev " + to_string(task[elev].queue.front()->GetCurrentElevator() == elev));
		}
		else if (elev->GetState() == Elevator::Down && elev->IsLowestFloor(current) && elev->GetPosition() < 0.51) {
			env.SendEvent("Elevator::Stop", 0, this, elev);
			cout << "throw" << endl;
			throw runtime_error(log);	
			//throw runtime_error(to_string(task[elev].queue.size()) + " per cur " + to_string(task[elev].queue.front()->GetCurrentFloor()->GetId()) + " pers final " + to_string(task[elev].queue.front()->GetFinalFloor()->GetId()) + " elev cur " + to_string(elev->GetCurrentFloor()->GetId()) + " elev next" + to_string(task[elev].nextStop->GetId()) + " is on elev " + to_string(task[elev].queue.front()->GetCurrentElevator() == elev));

		}
		else {
			state[elev].movingID = env.SendEvent("Elevator::Moving", 1, elev);

		}
	}
}

void ElevatorLogic::HandleStopped(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());

	string msg = e.GetData();

	cout << elev->GetPosition() << endl;

	if (!state[elev].broken) {
		env.SendEvent("Elevator::Open", 0, this, elev);
	}
	
}

void ElevatorLogic::HandleOpened(Environment &env, const Event &e) {

	// Elevator *elev = static_cast<Elevator*>(e.GetSender());

	// env.SendEvent("Elevator::Close", 4, this, elev);
}

void ElevatorLogic::HandleClosed(Environment &env, const Event &e) {

	Elevator *elev = static_cast<Elevator*>(e.GetSender());
	cout << "closed: size:" << task[elev].queue.size() << endl;
	task[elev].busy = false;
	if (SetNextStop(elev)) {
		ExecuteTask(elev, env);
	}

}

/*
Returns the distance between two floors.
Note: When distance is not natural returns -1
*/
int ElevatorLogic::GetDistance(Floor *f1, Floor *f2) {

	if (f1 == f2) {
		return 0;
	}

	int distance = f1->GetHeight() + f2->GetHeight();

	if (distance % 2 != 0) {
		return -1;
	}
	else {
		distance /= 2;
	}

	while (true) {
		if (f1->IsAbove(f2)) {
			f1 = f1->GetAbove();
		}
		else if (f1->IsBelow(f2)) {
			f1 = f1->GetBelow();
		}

		if (f1 == f2) {
			break;
		}
		else {
			distance += f1->GetHeight();
		}

	}

	return distance;
}