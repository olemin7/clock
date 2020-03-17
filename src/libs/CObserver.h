/*
 * CObserver.h
 *
 *  Created on: Aug 15, 2017
 *      Author: ominenko
 */

#ifndef COBSERVER_H_
#define COBSERVER_H_

#include <stdint.h>
#include <set>
using namespace std ;
/***
 *
 */
template <class T>
class Subject;

template <class T>
class Observer{
   public:
      Observer() {}
      virtual ~Observer() {}
      virtual void update(const Subject<T> &)= 0;
   };
/***
 *
 */
template <class T>
class Subject{
    private:
        T Value=0;
        std::set<Observer<T> *> m_observers; //use set to allows one connection only to the same subject
    public:
        void addListener (Observer<T> &observer){
           m_observers.insert(&observer);
       }
       virtual void notify(){
          for (auto iterator:m_observers)
              iterator->update( *(static_cast<Subject<T> *>(this)));
       }
       T getValue() const {
              return Value;
       }

       virtual void setValue(T value) {
           Value = value;
           this->notify();
       }
        virtual ~Subject(){};
};

/***
 *  Listen subject write value to writeVaue
 */
template<class T>
class ObserverWrite: public Observer<T> {
private:
    virtual void update(const Subject<T> &subject) {
        writeValue(subject.getValue());
    }
public:
    virtual void writeValue(T value)=0;
    virtual ~ObserverWrite() {
    }
    ;
};
/***
 *  Listen subject keep last value and calculate count of call
 */
template <class T>
class ObserverEcho: public ObserverWrite<T> {
public:
    T Value=0;
    uint32_t count=0;
    virtual void writeValue(T value) {
        Value = value;
        count++;
    }
    virtual ~ObserverEcho(){};
};
#endif /* COBSERVER_H_ */
