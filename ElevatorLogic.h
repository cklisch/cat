/*
 * ElevatorLogic.h
 *
 *  Created on: 20.06.2014
 *      Author: STSJR
 */

#ifndef ELEVATORLOGIC_H_
#define ELEVATORLOGIC_H_

#include "EventHandler.h"

#include "Interface.h"
#include "Person.h"
#include "Floor.h"
#include "Elevator.h"
#include "Event.h"
#include "Environment.h"

#include <list>
#include <map>
#include <set>

class Elevator;
class Floor;
class Interface;

class MyElevator : public Elevator {
private:
	std::vector<Floor*> floor_;
	std::list<Person*> person_;
	std::list<Floor*> stop_;

	bool moved;

public:
	MyElevator();
	void Init(const Elevator*);

	void AddStop(const Person*);
	void Move(Environment &env);
	int GetTimeTo(const Floor*);
	//void GoTo(const Floor*);
	
};

class ElevatorLogic: public EventHandler {
private:
	std::vector<MyElevator*> elev_;

	

	void HandleNotify(Environment &env, const Event &e);
	void HandleStopped(Environment &env, const Event &e);
	void HandleOpened(Environment &env, const Event &e);
	void HandleClosed(Environment &env, const Event &e);

	Person* GetCaller(const Event &e);
	MyElevator* FindElevator(const Person* p);
	bool IsCall(const Event &e);

public:
	ElevatorLogic();
	virtual ~ElevatorLogic();

	void Initialize(Environment &env);


};

#endif /* ELEVATORLOGIC_H_ */
