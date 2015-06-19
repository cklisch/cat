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

	struct ElevatorState {
		std::set<Person*> requestors;
		int load;
		bool moving;
		bool closing;
		bool stopping;
		bool beeping;
		bool open;
		bool broken;
		bool busy;
		bool up;
		int movingID;
		int closingID;

	};

	struct Request {
		Floor *floor;
		std::string direction;
		Interface *interf;
		Elevator *elev;
	};

private:
	
	void HandleAll(Environment &env, const Event &e);
	void HandleNotify(Environment &env, const Event &e);
	void HandleMoving(Environment &env, const Event &e);
	void HandleStopped(Environment &env, const Event &e);
	void HandleClosed(Environment &env, const Event &e);
	void HandleEntered(Environment &env, const Event &e);
	void HandleExited(Environment &env, const Event &e);
	void HandleMalfunction(Environment &env, const Event &e);
	void HandleFixed(Environment &env, const Event &e);
	void HandleOpened(Environment &env, const Event &e);
	void HandleExiting(Environment &env, const Event &e);
	void HandleEntering(Environment &env, const Event &e);
	void HandleClosing(Environment &env, const Event &e);
	void HanndleBeeping(Environment &env, const Event &e);
	void HandleInteract(Environment &env, const Event &e);



	
	Elevator* FindElevator(Person *person, Interface *interf);
	int GetElevatorTime(Elevator *elev, Person *person);
	void SetState(Elevator *elev);
	void ExecuteTask(Elevator *elev, Environment &env);
	void CloseDoor( Environment &env, Elevator *elev, int delay);
	void AddToQueue(Elevator *elev, Person *person);
	void EraseFromQueue(Person *person);
	int GetDistance(Floor *f1, Floor *f2);
	bool CanExecuteTask(Elevator *elev);
	void Stop(Environment &env, Elevator *elev, int delay);



	std::map<Elevator*,ElevatorState> state;
	std::map<Person*,Request> request;

};

#endif /* ELEVATORLOGIC_H_ */
