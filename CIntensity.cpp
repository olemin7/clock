/*
 * CIntensity.cpp
 *
 *  Created on: Feb 17, 2017
 *      Author: ominenko
 */

#include "CIntensity.h"

CIntensity::CIntensity(tIntensityRation *aIntensityRation, int count) {
  this->pIntensityRation = aIntensityRation;
  this->count = count;
}

void CIntensity::setGetEnviropment(tGetEnviropment func) {
  getEnviropmentPtr = func;
}

void CIntensity::handle() {
  if (NULL == getEnviropmentPtr || NULL == setIntensityPtr)
    return;

  int env = getEnviropmentPtr();
  int intens = getIntensity(env);
  setIntensityPtr(intens);
}

void CIntensity::setSetIntensity(tSetIntensity func) { setIntensityPtr = func; }

int CIntensity::getIntensity(int Enviropment) {
  if (NULL == pIntensityRation)
    return 0;
  int result = 0;
  for (int n = 0; n != count; n++) {
    if (pIntensityRation[n][0] > Enviropment)
      break;
    result = pIntensityRation[n][1];
  }
  return result;
}

#ifdef TEST

int test1() {

  int aIntensityRation[][2] = {{0, 0}, {20, 2}, {40, 3}};
  CIntensity tmp(aIntensityRation, 3);
  EXPECT_EQ(tmp.getIntensity(0), 0);
  EXPECT_EQ(tmp.getIntensity(10), 0);
  EXPECT_EQ(tmp.getIntensity(20), 2);
  EXPECT_EQ(tmp.getIntensity(30), 2);
  EXPECT_EQ(tmp.getIntensity(40), 3);
  EXPECT_EQ(tmp.getIntensity(50), 3);
  return 0;
}

int main() {
  std::cout << "start test ne ok\n";
  if (test1())
    return 1;

  std::cout << "All done ok\n";
  return 0;
}
#endif
