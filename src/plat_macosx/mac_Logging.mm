// ------------------------------------------------------------------
// mac_Logging - stardazed
// (c) 2014 by Arthur Langereis
// ------------------------------------------------------------------

#include "system/Logging.hpp"
#import <Foundation/NSString.h>

namespace stardazed {


void log(const std::string& msg) {
	NSLog(@"%s", msg.c_str());
}


} // ns stardazed
