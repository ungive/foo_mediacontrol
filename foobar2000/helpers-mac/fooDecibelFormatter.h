#import <Foundation/Foundation.h>

#import "foobar2000-mac-helpers.h"

#define fooDecibelFormatter FB2K_OBJC_CLASS(fooDecibelFormatter)

@interface fooDecibelFormatter : NSFormatter

@property (nonatomic) NSNumber * minValue;
@property (nonatomic) NSNumber * maxValue;
@property (nonatomic) BOOL showSignAlways;
@end
