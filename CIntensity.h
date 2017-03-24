/*
 * CIntensity.h
 *
 *  Created on: Feb 17, 2017
 *      Author: ominenko
 */

#ifndef CLOCK_CINTENSITY_H_
#define CLOCK_CINTENSITY_H_
//#define TEST
#ifndef TEST
#include <Arduino.h>
#else
#include <stddef.h>
#include <iostream>
#endif
typedef int (*tGetEnviropment)();
typedef void (*tSetIntensity)(int);
typedef int tIntensityRation[2];

class CIntensity {
  tGetEnviropment getEnviropmentPtr = NULL;
  tSetIntensity setIntensityPtr = NULL;
  tIntensityRation *pIntensityRation=NULL;
  int count=0;
public:
  CIntensity(tIntensityRation *aIntensityRation,int count);
  void handle();
  void setGetEnviropment(tGetEnviropment func);
  void setSetIntensity(tSetIntensity func);
  int getIntensity(int Enviropment);
};

#endif /* CLOCK_CINTENSITY_H_ */
