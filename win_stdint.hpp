/*
 * win_stdint.hpp
 *
 *  Created on: Sep 28, 2009
 *      Author: chuck
 */

#ifndef IPPC_WINSTDINT_HPP_
#define IPPC_WINSTDINT_HPP_

typedef char       int8_t;
typedef short int  int16_t;
typedef int        int32_t;

typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned int        uint32_t;

// Carefull, need to check for this if __WORDSIZE == 6

#endif /* !IPPC_WINSTDINT_HPP_ */
