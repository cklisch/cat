/*
 * ElevatorLogic.cpp
 *
 *  Created on: 20.06.2014
 *      Author: STSJR
 */

#include "ElevatorLogic.h"

#include <iostream>

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

void MyElevator::Init(const Elevator *elev) {
	int interf_cnt = this->GetInterfaceCount();
	
	for (int i=0; i<interf_cnt; i++) {
		floor_.push_back(static_cast<Floor*>(this->GetInterface(i)->GetLoadable(0)));	
	}
}

void MyElevator::AddStop(const Person* p) {
	stop_.push_back(p->GetCurrentFloor());
	stop_.push_back(p->GetFinalFloor());
}

void MyElevator::Move(Environment &env) {
	if (stop_.front()->IsBelow(GetCurrentFloor())) {
		env.SendEvent("Elevator::Down", 0, this);
		env.SendEvent("Elevator::Stop", 4, this);
	}
	else {
		env.SendEvent("Elevator::Up", 0, this);
		env.SendEvent("Elevator::Stop", 4, this);
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
}

MyElevator* ElevatorLogic::FindElevator(const Person* p) {
	return elev_[0];
}

Person* ElevatorLogic::GetCaller(const Event &e) {
	EventHandler *handler = e.GetEventHandler();
	Person * caller = NULL;
	if (handler->GetName().substr(0,6) == "Person") {
		caller = static_cast<Person*>(handler);		
	}
	return caller;
}

bool ElevatorLogic::IsCall(const Event &e) {
	
	Interface *interf = static_cast<Interface*>(e.GetSender());
	bool call = false;
	int ldbl_cnt = interf->GetLoadableCount();
	std::cout << elev_.size() << std::endl;
	for (int i=0; i<ldbl_cnt; ++i) {
		std::cout << i << std::endl;
		Loadable *loadable = interf->GetLoadable(i);
		if (loadable->GetType() == "Elevator") {
			std::cout << "Elevator" << std::endl;
			MyElevator* elev = static_cast<MyElevator*>(loadable);
			elev->Init(static_cast<Elevator*>(loadable));
			elev_[0] = elev;
			std::cout << "is call" << std::endl;

			call = true;
		}
	}
	std::cout << elev_.size() << std::endl;

	return call;

}

void ElevatorLogic::HandleNotify(Environment &env, const Event &e) {

	std::vector<MyElevator*> test;
	std::cout << elev_.size() << std::endl;

	
	if (IsCall(e)) {
		Person* caller = GetCaller(e);


		MyElevator* elev = FindElevator(caller);
		std::cout << "is call" << std::endl;

		elev->AddStop(caller);
		elev->Move(env);
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

void ElevatorLogic::HandleStopped(Environment &env, const Event &e) {

	Elevator *ele = static_cast<Elevator*>(e.GetSender());

	env.SendEvent("Elevator::Open", 0, this, ele);
}

void ElevatorLogic::HandleOpened(Environment &env, const Event &e) {

	Elevator *ele = static_cast<Elevator*>(e.GetSender());

	env.SendEvent("Elevator::Close", 4, this, ele);
}

void ElevatorLogic::HandleClosed(Environment &env, const Event &e) {

	
}
