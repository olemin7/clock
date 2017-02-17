/*
 * CIntensity.h
 *
 *  Created on: Feb 17, 2017
 *      Author: ominenko
 */

#ifndef CLOCK_CINTENSITY_H_
#define CLOCK_CINTENSITY_H_
#include <Arduino.h>
typedef int (*tGetEnviropment)();
typedef void (*tSetIntensity)(int);

class CIntensity {
  tGetEnviropment getEnviropmentPtr = NULL;
  tSetIntensity setIntensityPtr = NULL;

public:
  CIntensity();
  void handle();
  void setGetEnviropment(tGetEnviropment func, int min, int max);
  void setSetIntensity(tSetIntensity func, int min, int max);
};

#endif /* CLOCK_CINTENSITY_H_ */
