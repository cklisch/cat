// OpenBSD netcat (Debian patchlevel 1.89-4ubuntu1)

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

#include <algorithm>
#include <list>
#include <map>
#include <set>

class Elevator;
class Floor;
class Interface;
class ElevatorLogic;

class MyElevator {
private:
	Elevator *elev;
	ElevatorLogic* logic;
	Floor* initialFloor;

	
	std::list<Person*> person_;
	std::list<Floor*> stop_;
	bool moved;
	

public:
	
	std::list<Floor*> floor_;
	bool HasMoved() {return moved;}
	MyElevator(Elevator *e, ElevatorLogic* l);
	// void Init();
	Elevator* GetElevator() {return elev;};
	bool HasElevator(const Elevator* e) {return e == elev;};
	bool HasStop(const Floor* f);
	void AddStop(const Person*);
	void Move(Environment &env);
	int GetDistanceTo(const Floor* f);
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
	void HandleEntered(Environment &env, const Event &e);
	void HandleExited(Environment &env, const Event &e);

	
	Person* GetPerson(const Event &e);
	void AddMyElevator(const Interface* i);
	MyElevator* FindElevator(const Person* p);
	MyElevator* GetMyElevator(const Elevator* elev);

	bool FromElevatorInterface(const Event &e);
	bool FromFloorInterface(const Event &e);
public:
	ElevatorLogic();
	virtual ~ElevatorLogic();

	void Initialize(Environment &env);


};

#endif /* ELEVATORLOGIC_H_ */
