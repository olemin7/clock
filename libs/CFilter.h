#ifndef CFILTER_H_
#define CFILTER_H_
#include "CObserver.h"

namespace std {
class CFilterLoop{
private:
    static std::set<CFilterLoop *> filters;
protected:
    virtual uint32_t getTimeInMs()=0;
public:
    CFilterLoop(){
        filters.insert(this);
    }
    virtual void loop()=0;
    static void loops();
    virtual ~CFilterLoop(){
        filters.erase(this);
    };
};

/***
 * test stub
 */
class CTimeInMs_stub{
protected:
    static uint32_t _time;
public:
    static uint32_t getTimeInMs(){ return _time; };
    static void setTimeInMs(uint32_t time){
        _time=time;
        CFilterLoop::loops();
    };
    static void incTimeInMs(uint32_t time){setTimeInMs(_time+=time);};
};
template <class T>
class CFilter: public Subject<T> ,public CFilterLoop{
public:
    virtual ~CFilter(){};
 };

/***
 * read value from readValue and push subject
 * if readValue ==true set delay periodOk , if false set periodFail
 */

template<class T>
class CSubjectPeriodic: public Subject<T>, public CFilterLoop {
private:
    const uint32_t periodOk;
    const uint32_t periodFail;
    uint32_t nextUpdate;
public:
    virtual bool readValue(T &)=0; //return true if result ok
    CSubjectPeriodic() : periodOk(0), periodFail(0),nextUpdate(0) { } ;
    CSubjectPeriodic(uint32_t aperiod) : periodOk(aperiod), periodFail(0),nextUpdate(0) { } ;
    CSubjectPeriodic(uint32_t aperiodOk,uint32_t aperiodFail): periodOk(aperiodOk), periodFail(aperiodFail),nextUpdate(0) { } ;
    void setValue(T value){//to force update
        Subject<T>::setValue(value);
        if(periodOk){
        	nextUpdate = getTimeInMs() + periodOk;
        }
    }
    void loop() {
    	if(0==periodOk && 0==periodFail){// speed up for zero timeout
    		T value;
    		if(readValue(value)){
				setValue(value);
			}
		   return;
    	}

        if (getTimeInMs() >= nextUpdate) {
            T value;
            if(readValue(value)){
            	setValue(value);
            }else{
                nextUpdate = getTimeInMs() + periodFail;
            }
        }
    }
    virtual ~CSubjectPeriodic() {} ;
};


/***
 * send notice if subject changed
 *
 */
template <class T>
class CFilter_OnChange:public ObserverWrite<T>, public Subject<T>{
protected:
    virtual bool isChanged(const T &value){
        return (Subject<T>::getValue() != value);
    }
public:
    virtual void writeValue(T value) {
        if(isChanged(value)){
            Subject<T>::setValue(value);
        }
    }
    virtual ~CFilter_OnChange(){};
};

/***
 * send notice if subject changed with threshold
 *
 */
//TODO tests
template <class T>
class CFilter_OnChangeWithThreshold:public CFilter_OnChange<T>{
protected:
    const T deltaUp;
    const T deltaDown;
    virtual bool isChanged(const T &value){
        if(Subject<T>::getValue() > (value+ deltaUp))return true;
        if(Subject<T>::getValue() < (value- deltaDown))return true;
        return false;
    }
public:
    CFilter_OnChangeWithThreshold(T _deltaUp,T _deltaDown):deltaUp(_deltaUp),deltaDown(_deltaDown){};
    virtual ~CFilter_OnChangeWithThreshold(){};
};


/***
 * send notice if subject changed or resend by timeout
 * can be used to rise event on change and continue notificate that source is live
 */
template <class T>
class CFilter_ChTimed:public CFilter_OnChange<T>, public CFilterLoop{
protected:
    const uint32_t periodResend;
    uint32_t nextResend;
    virtual void setValue(T value) {
        CFilter_OnChange<T>::setValue(value);
        nextResend=getTimeInMs()+periodResend; //set resent timeout
    }
public:
    CFilter_ChTimed(uint32_t period):periodResend(period),nextResend(period){};
      void loop() {
        if (getTimeInMs() >= nextResend) {
            setValue(Subject<T>::getValue());
       }
    }
    virtual ~CFilter_ChTimed(){};
};

/***
 * calculate position in range
 * [0 ..100 > 200]
 * return -1 if below first value
 */
template <class T>
class CFilter_ValueToPos:public Observer<T>, public Subject<int16_t>{
protected:
    const T *pRagnes;
    const int16_t count;
public:
    CFilter_ValueToPos(const T *aRagnes,int16_t acount):pRagnes(aRagnes),count(acount){};
    virtual void setValue(T value) {
        int16_t result=-1;
        int16_t n=count;
        const T *pTmp=pRagnes;

        while(n){
            if(value<*pTmp)break;
            pTmp++;
            result++;
            n--;
        }

        Subject<int16_t>::setValue(result);
    }
    virtual void update(const  Subject<T> &subject){
        setValue(subject.getValue());
    }
    virtual ~CFilter_ValueToPos(){};
};

/***
 * send notice if subject changed after setting value;
 * possible set separated debounce value for changing up and down
 *
 */
template <class T>
class CFilter_Debounce:public Observer<T>, public CFilter<T>{
protected:
    const uint32_t debounceTimeUp;
    const uint32_t debounceTimeDown;
    uint32_t debounceTimeout=0;
    T requestedValue;
    virtual uint32_t getTimeInMs()=0;
public:
    CFilter_Debounce(uint32_t aDebounceTime):debounceTimeUp(aDebounceTime),debounceTimeDown(aDebounceTime){};
    CFilter_Debounce(uint32_t aDebounceTimeUp,uint32_t aDebounceTimeDown):debounceTimeUp(aDebounceTimeUp),debounceTimeDown(aDebounceTimeDown){};
    //wait setting new walue;
    //aTimeout - im ms how long wait to setting
    //ret 0 - ok
    bool waitSetting(const uint32_t aTimeout=0){
    	uint32_t waitTill=getTimeInMs()+aTimeout;
        while(Subject<T>::getValue()!= requestedValue){
        	if(aTimeout &&(waitTill<getTimeInMs())){
        		return 1;//time out pass, vallue is not setted
        	}
            loop();
        }
        return 0;
    }
    virtual void loop(){
        if(Subject<T>::getValue()== requestedValue){
            return;
        }
        if(getTimeInMs()>=debounceTimeout)
            Subject<T>::setValue(requestedValue);
    }
    virtual void setValue(T value) {
        if(requestedValue == value)//continue to wait
            return;
        if(Subject<T>::getValue()> value){
            debounceTimeout=getTimeInMs()+debounceTimeDown;
        }else{
            debounceTimeout=getTimeInMs()+debounceTimeUp;
        }
        requestedValue=value;
        loop();
    }
    virtual void update(const  Subject<T> &subject){
        setValue(subject.getValue());
    }
    virtual ~CFilter_Debounce(){};
};

} //STD

#endif /* CFILTER_H_ */
