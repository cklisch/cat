/*
 * ElevatorLogic.h
 *
 *  Created on: 20.06.2014
 *      Author: STSJR
 */

#ifndef ELEVATORLOGIC_H_
#define ELEVATORLOGIC_H_

#include "EventHandler.h"

#include <list>
#include <map>
#include <set>

class Elevator;
class Floor;
class Interface;
class Person;

class ElevatorLogic: public EventHandler {

public:
	ElevatorLogic();
	virtual ~ElevatorLogic();

	void Initialize(Environment &env);

	typedef struct {
		Floor* nextStop;
		bool needsChange;
		bool busy;
		std::list<Person*> queue;

	} ElevatorTask;

	struct ElevatorState {
		std::set<Person*> passengers;
		bool beeping;
		bool broken;
		bool overloaded;
		bool requested;
		int movingID;

	} ;


private:
	
	void HandleAll(Environment &env, const Event &e);

	void HandleNotify(Environment &env, const Event &e);
	void HandleMoving(Environment &env, const Event &e);
	void HandleStopped(Environment &env, const Event &e);
	void HandleOpened(Environment &env, const Event &e);
	void HandleClosed(Environment &env, const Event &e);
	void HandleEntered(Environment &env, const Event &e);
	void HandleExited(Environment &env, const Event &e);
	void HandleMalfunction(Environment &env, const Event &e);
	void HandleFixed(Environment &env, const Event &e);


	
	Elevator* FindElevator(Person *person, Interface *interf);
	void AddToQueue(Person *person, Elevator *elev);
	void EraseFromQueue(Person *person, Elevator *elev);
	void SetTask(Elevator *elev);
	void SetState(Elevator *elev);
	bool SetNextStop(Elevator *elev);
	void ExecuteTask(Elevator *elev, Environment &env);
	int GetWeight(std::set<Person*> passengers);
	bool IsOverloaded(Elevator *elev);
	int GetDistance(Floor *f1, Floor *f2);
	

	std::string log;

	std::map<Elevator*,ElevatorTask> task;
	std::map<Elevator*,ElevatorState> state;

};

#endif /* ELEVATORLOGIC_H_ */
