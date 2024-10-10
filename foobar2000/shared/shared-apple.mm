#include "shared.h"
#include "shared-apple.h"

#import <Cocoa/Cocoa.h>

bool uSetClipboardString(const char * str) {
    @autoreleasepool {
        @try {
            NSPasteboard * pb = [NSPasteboard generalPasteboard];
            [pb clearContents];
            [pb setString: [NSString stringWithUTF8String: str] forType:NSPasteboardTypeString];
            return true;
        } @catch (NSException *) {
            
        }
    }
    return false;
}

bool uGetClipboardString(pfc::string_base & out) {
    bool rv = false;
    @autoreleasepool {
        NSPasteboard * pb = [NSPasteboard generalPasteboard];
        NSString * str = [pb stringForType: NSPasteboardTypeString];
        if ( str != nil ) {
            out = str.UTF8String;
            rv = true;
        }
    }
    return rv;
}

void fb2k::crashOnException(std::function<void()> f, const char * context) {
    auto fail = [context] ( const char * msg ) {
        if (context) {
            fb2k::crashWithMessage(pfc::format(context, ": ", msg));
        } else {
            fb2k::crashWithMessage(msg);
        }
    };
    try {
        @autoreleasepool {
            @try {
                f();
            } @catch(NSException * e) {
                auto header = pfc::format("NSException: ", e.name.UTF8String, " reason: ", e.reason.UTF8String );
                uAddDebugEvent( header );
                uAddDebugEvent("Stack:");
                for(NSString * str in e.callStackSymbols ) {
                    uAddDebugEvent(str.UTF8String);
                }
                fail(header);
            }
        }
    } catch(std::exception const & e) {
        fail(pfc::format("C++ exception: ", e.what()));
    } catch(...) {
        fail("Invalid exception");
    }
}
