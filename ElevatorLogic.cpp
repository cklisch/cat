/*
 * ElevatorLogic.cpp
 *
 *  Created on: 20.06.2014
 *      Author: STSJR
 */

#include "ElevatorLogic.h"

#include <iostream>
 #include <fstream>
#include <cstdlib>
 #include <stdio.h>

using namespace std;

// void MyElevator::GoTo(const Floor *f) {
// 	if (IsAbove(f)) {
// 		env.SendEvent("Elevator::Up", 0, this);
// 		env.SendEvent("Interface::Notify", 4, this);
// 	}
// 	else {
// 		env.SendEvent("Elevator::Down", 0, this);
// 		env.SendEvent("Interface::Notify", 4, this);
// 	}
// }


MyElevator::MyElevator(Elevator *e, ElevatorLogic* l) : elev(e), logic(l), moved(false) {
	

	initialFloor = e->GetCurrentFloor();
	int interf_cnt = e->GetInterfaceCount();
	floor_.push_back(static_cast<Floor*>(e->GetInterface(0)->GetLoadable(0)));
	list<Floor*>::iterator pos;
	Floor* f;
	bool b = true;
	cout << "size" << floor_.size() << interf_cnt << endl;

	for (int i=1; i<interf_cnt; i++, b = true) {
		
		f = static_cast<Floor*>(e->GetInterface(i)->GetLoadable(0));
		for (pos = floor_.begin(); pos != floor_.end(); pos++) {
			if (f->IsAbove(*pos)) {   //smalles element first (ascending order)
				floor_.insert(pos, f);
				b = false;
				break;
			}
		}
		if (b)
			floor_.push_back(f);	
	}

	// for (int i = 0; i < interf_cnt; i++) {
	// 	cout << "floor " << floor_.front()->GetId() << endl;
	// 	floor_.pop_front();
	// }

}



void MyElevator::AddStop(const Person* p) {

	if (stop_.size() != 0) {
		if (stop_.back() == p->GetCurrentFloor()) {

		}
		else {
			stop_.push_back(p->GetCurrentFloor());
		}
	}
	else {
		stop_.push_back(p->GetCurrentFloor());
	}
	
	stop_.push_back(p->GetFinalFloor());
}

int MyElevator::GetDistanceTo(const Floor* f) {

	if (initialFloor == f) {
		return 0;
	}

	//cout << "size" << floor_.size() << endl;
	list<Floor*>::iterator start = find(floor_.begin(), floor_.end(), initialFloor);
	list<Floor*>::iterator end = find(floor_.begin(), floor_.end(), f);
	int distance = f->GetHeight() + initialFloor->GetHeight();
	if (distance % 2 != 0) {
		return -1;
	}
	distance /= 2;
	bool up = !f->IsAbove(initialFloor);
	// cout << initialFloor->GetId() << " to " << f->GetId() << " bool "<< up << endl;
	if (up) {
		start++;
		while (start != end) {
			//cout << (*start)->GetId() << " and " << (*end)->GetId() << endl;

			//cout << distance << endl;
			distance += (*start)->GetHeight();
			start++;
		}
	}
	else {
		start--;
		
		while (start != end) {
			//cout << (*start)->GetId() << " and " << (*end)->GetId() << endl;
			//cout << distance << endl;

			distance += (*start)->GetHeight();
			start--;
		}

	}
	//cout << distance << endl;

	return distance;
	
	

}

bool MyElevator::HasStop(const Floor* f) {
	int speed = elev->GetSpeed();

	int distance = GetDistanceTo(f);

	cout << distance << " mod  " << speed <<  " = " << (distance % speed) << endl;

	if (distance % speed != 0 || distance == -1) {
		cout << "false" << endl;
		return false;
	}
		
	else 
		return true;
}



void MyElevator::Move(Environment &env) {
	cout << elev->GetPosition() << " " << elev->GetId() << " floor id "<< elev->GetCurrentFloor()->GetId() << endl;
	if (stop_.size() > 0) { 




		if (stop_.front()->IsAbove(elev->GetCurrentFloor()) 
			|| (elev->GetPosition() > 0.5 && elev->GetCurrentFloor() == stop_.front())) {
			if (elev->GetState() != Elevator::Down)
				env.SendEvent("Elevator::Down", 0, logic, elev);
			env.SendEvent("Interface::Notify", 1, elev->GetInterface(0), elev);
		}
		else if (stop_.front()->IsBelow(elev->GetCurrentFloor()) 
			|| (elev->GetPosition() < 0.5 && elev->GetCurrentFloor() == stop_.front())) {
			if (elev->GetState() != Elevator::Up)
				env.SendEvent("Elevator::Up", 0, logic, elev);
			env.SendEvent("Interface::Notify", 1, elev->GetInterface(0), elev);
		}
		else {
			env.SendEvent("Elevator::Stop", 0, logic, elev);
			stop_.pop_front();
		}
		moved = true;
		
	}
	else {
		env.SendEvent("Elevator::Stop", 0, logic, elev);
	}
}



//-------------------------------------------------------------------------------------




ElevatorLogic::ElevatorLogic() : EventHandler("ElevatorLogic") {
	
}

ElevatorLogic::~ElevatorLogic() {
}

void ElevatorLogic::Initialize(Environment &env) {
	env.RegisterEventHandler("Interface::Notify", this, &ElevatorLogic::HandleNotify);
	env.RegisterEventHandler("Elevator::Stopped", this, &ElevatorLogic::HandleStopped);
	env.RegisterEventHandler("Elevator::Opened", this, &ElevatorLogic::HandleOpened);
	env.RegisterEventHandler("Elevator::Closed", this, &ElevatorLogic::HandleClosed);
	env.RegisterEventHandler("Person::Entered", this, &ElevatorLogic::HandleEntered);
	env.RegisterEventHandler("Person::Exited", this, &ElevatorLogic::HandleExited);
}

MyElevator* ElevatorLogic::GetMyElevator(const Elevator* e) {
	for (unsigned int i=0; i<elev_.size(); i++) {
		if (elev_[i]->HasElevator(e))
			return elev_[i];
	}
	return NULL;
}

MyElevator* ElevatorLogic::FindElevator(const Person* p) {

	Floor* f = p->GetCurrentFloor();
	vector<MyElevator*> possible;

	for (int i=0; i<f->GetInterfaceCount(); i++) {
		Interface* interf = f->GetInterface(i);
		for (int j=0; j<interf->GetLoadableCount(); j++) {
			Elevator* elev = static_cast<Elevator*>(interf->GetLoadable(j));
			if (GetMyElevator(elev) != NULL) {
				MyElevator *myElev = GetMyElevator(elev);
				if (myElev->HasStop(p->GetCurrentFloor()) && myElev->HasStop(p->GetFinalFloor()))
					possible.push_back(GetMyElevator(elev));
			}
		}
	}
	cout << p->GetId() << endl;
	for (unsigned int i=0; i<possible.size(); i++) {
		if (!possible[i]->HasMoved()) {
			return possible[i];
		}
		
			
	}



	return possible[0];
}


Person* ElevatorLogic::GetPerson(const Event &e) {
	EventHandler *handler = e.GetEventHandler();
	Person * caller = NULL;
	if (handler->GetName().substr(0,6) == "Person") {
		caller = static_cast<Person*>(handler);		
	}
	return caller;
}



bool ElevatorLogic::FromFloorInterface(const Event &e) {
	Interface *interf = static_cast<Interface*>(e.GetSender());

	if (interf->GetLoadable(0)->GetType() == "Elevator") {
		return true;
	}
	else
		return false;
}

void ElevatorLogic::AddMyElevator(const Interface* interf) {
	int ldbl_cnt = interf->GetLoadableCount();
	for (int i=0; i<ldbl_cnt; ++i) {
		Loadable *loadable = interf->GetLoadable(i);
		if (loadable->GetType() == "Elevator") {
			Elevator*  e = static_cast<Elevator*>(loadable);
			if (GetMyElevator(e) == NULL) {
				MyElevator* elev = new MyElevator(e, this);
				elev_.push_back(elev);
			}
		}
	}
}


bool ElevatorLogic::FromElevatorInterface(const Event &e) {
	Interface *interf = static_cast<Interface*>(e.GetSender());
	if (interf->GetLoadable(0)->GetType() == "Floor" ) {
		return true;
	}
	else 
		return false;
}

void ElevatorLogic::HandleNotify(Environment &env, const Event &e) {

	


	MyElevator *myElev;
	if (FromFloorInterface(e)) {
		AddMyElevator(static_cast<Interface*>(e.GetSender()));
		cout << "person" << endl;
		Person* caller = GetPerson(e);

		myElev = FindElevator(caller);

		
		// string a = to_string(elev_[0]->GetElevator()->GetSpeed());




		// //string b = to_string(elev_[1]->GetElevator()->GetSpeed());
		
		// cout << "wtf " << a  << " wtf2 "  << endl;

		// throw std::overflow_error(a );

		myElev->AddStop(caller);

		if (!myElev->HasMoved())
			myElev->Move(env);
	}
	else if (FromElevatorInterface(e)) {
		if (GetPerson(e) == NULL) {
			myElev = GetMyElevator(static_cast<Elevator*>(e.GetEventHandler()));
			myElev->Move(env);
		}
		
	}
	
	// 

	// 	for (i=0; i<elev_.size(); i++) {
	// 					cur_floor = elev_[i]->GetCurrentFloor();
	// 		if (elev_[i]->IsHighestFloor(cur_floor)) {
	// 			env.SendEvent("Elevator::Stop", 0, this, elev_[i]);

	// 		}
	// 		else {
	// 			env.SendEvent("Elevator::Up", 0, this, elev_[i]);
	// 			env.SendEvent("Interface::Notify", 1, interf, this);
	// 		}
	// 	}

	// 	env.SendEvent("Interface::Notify", 0, interf, handler);
	// }
	


	




	// for (i=0; i<elev_.size(); i++) {
	// 	std::cout << elev_[i]->GetPosition() << elev_[i]->GetSpeed() << "floor " << elev_[i]->GetCurrentFloor()->GetHeight() << std::endl;
	// 	Floor *cur_floor = elev_[i]->GetCurrentFloor();
	// 	if (elev_[i]->GetState() == Elevator::Up) {
	// 		std::cout << "up" << std::endl;
	// 	}
	// 	for (j=0; i<cur_floor->GetInterfaceCount(); i++) {
	// 		if (interf == cur_floor->GetInterface(i)) {
	// 			env.SendEvent("Elevator::Stop", 0, this, elev_[i]);
	// 			found = true;
	// 		}

	// 	}
		
	// 	if (!found) {
	// 		env.SendEvent("Elevator::Up", 0, this, elev_[i]);
	// 		env.SendEvent("Interface::Notify", 1, interf);
	// 		moved_ = true;
	// 	}
		
	// }




	// if (loadable->GetType() == "Elevator") {

	// 	Elevator *ele = static_cast<Elevator*>(loadable);
		
		
	// 	//env.SendEvent("Elevator::Stop", 2, this, ele);
		
		
		
	// 	if (!ele->IsHighestFloor(ele->GetCurrentFloor())) {
	// 		env.SendEvent("Elevator::Up", 0, this, ele);
			
	// 		env.SendEvent("Interface::Notify", 1, this, ele);

			
	// 	}
	// 	else {
	// 		env.SendEvent("Elevator::Stop", 1, this, ele);


	// 		env.SendEvent("Elevator::Open", 1, this, ele);
	// 	}
		
	// }
}

void ElevatorLogic::HandleEntered(Environment &env, const Event &e) {
	Elevator *ele =static_cast<Elevator*>(e.GetEventHandler());

	env.SendEvent("Elevator::Close", 0, this, ele);
}

void ElevatorLogic::HandleExited(Environment &env, const Event &e) {
	Elevator *ele = static_cast<Elevator*>(e.GetEventHandler());

	env.SendEvent("Elevator::Close", 0, this, ele);
}

void ElevatorLogic::HandleStopped(Environment &env, const Event &e) {

	Elevator *ele = static_cast<Elevator*>(e.GetSender());

	env.SendEvent("Elevator::Open", 0, this, ele);
}

void ElevatorLogic::HandleOpened(Environment &env, const Event &e) {

	// Elevator *ele = static_cast<Elevator*>(e.GetSender());

	// env.SendEvent("Elevator::Close", 4, this, ele);
}

void ElevatorLogic::HandleClosed(Environment &env, const Event &e) {

	MyElevator* elev = GetMyElevator(static_cast<Elevator*>(e.GetSender()));

	elev->Move(env);
	
}
