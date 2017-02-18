//
//  gpio.hpp
//  quadcopter
//
//  Created by Muhsin Fatih Yorulmaz on 18/02/2017.
//  Copyright © 2017 mekatrotekno. All rights reserved.
//

#ifndef gpio_hpp
#define gpio_hpp

#include <stdio.h>
#include <stm32f4xx.h>

void gpio(GPIO_TypeDef* GPIOx, uint32_t pin, GPIOMode_TypeDef mode, GPIOPuPd_TypeDef PuPd);














#endif /* gpio_hpp */

