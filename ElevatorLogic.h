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
		int load;
		bool moving;
		bool closing;
		bool open;
		bool broken;
		bool overloaded;
		bool busy;
		bool up;
		int movingID;

	};

	struct PersonState {
		bool hasTraveled;
		Elevator *elevator;
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


	
	Elevator* FindElevator(Person *person, Interface *interf, std::string direction);
	int GetElevatorScore(Elevator *elev, Person *person, std::string direction);
	void SetState(Elevator *elev);
	void ExecuteTask(Elevator *elev, Environment &env);
	bool IsOverloaded(Elevator *elev, Environment &env);
	void CloseDoor(Elevator *elev, Environment &env);
	void AddToQueue(Elevator *elev, Person *person);
	void EraseFromQueue(Person *person);
	int GetDistance(Floor *f1, Floor *f2);


	std::map<Elevator*,std::list<Person*> > queue;
	std::map<Elevator*,std::set<Person*> > passengers;
	std::map<Person*,Floor*> stop;
	std::map<Elevator*,ElevatorState> state;
	std::map<Person*,PersonState> info;

};

#endif /* ELEVATORLOGIC_H_ */
