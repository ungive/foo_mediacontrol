//
//  fooTimeFormatter.h
//  foo_abx
//
//  Created by P on 06/09/2023.
//

#import <Foundation/Foundation.h>
#import "foobar2000-mac-helpers.h"

#define fooTimeFormatter FB2K_OBJC_CLASS(fooTimeFormatter)

@interface fooTimeFormatter : NSFormatter
@property (nonatomic) NSNumber * digits;
@end
