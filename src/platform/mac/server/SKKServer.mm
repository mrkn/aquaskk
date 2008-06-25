/* -*- ObjC -*-

  MacOS X implementation of the SKK input method.

  Copyright (C) 2008 Tomotaka SUWA <t.suwa@mac.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "SKKServer.h"
#include "SKKPreProcessor.h"
#include "SKKRomanKanaConverter.h"
#include "SKKBackEnd.h"
#include "SKKConstVars.h"
#include "skkserv.h"

#include <InputMethodKit/InputMethodKit.h>
#include <signal.h>

static void terminate(int) {
    [NSApp terminate:nil];
}

@interface SKKServer (Local)
- (void)prepareSignalHandler;
- (void)prepareDirectory;
- (void)prepareConnection;
- (void)prepareUserDefaults;
- (void)prepareDictionary;
- (void)prepareInputMethodKit;

- (BOOL)fileExistsAtPath:(NSString*)path;
- (void)createDirectory:(NSString*)path;
- (NSString*)pathForSystemResource:(NSString*)path;
- (NSString*)pathForUserResource:(NSString*)path;
- (NSString*)pathForResource:(NSString*)path;
@end

@implementation SKKServer

- (void)awakeFromNib {
    skkserv_ = 0;

    [self prepareSignalHandler];
    [self prepareDirectory];
    [self prepareConnection];
    [self prepareUserDefaults];
    [self prepareDictionary];
    [self prepareInputMethodKit];

    [self reloadUserDefaults];
    [self reloadDictionarySet];
    [self reloadComponents];
}

- (void)reloadUserDefaults {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    bool flag;

    [defaults synchronize];

    delete skkserv_;

    if([defaults boolForKey:SKKUserDefaultKeys::enable_skkserv] == YES) {
        skkserv_ = new skkserv([defaults integerForKey:SKKUserDefaultKeys::skkserv_port],
                               [defaults boolForKey:SKKUserDefaultKeys::skkserv_localonly] == YES);
    }

    flag = [defaults boolForKey:SKKUserDefaultKeys::use_numeric_conversion] == YES;
    SKKBackEnd::theInstance().UseNumericConversion(flag);

    flag = [defaults boolForKey:SKKUserDefaultKeys::enable_extended_completion] == YES;
    SKKBackEnd::theInstance().EnableExtendedCompletion(flag);

    NSLog(@"UserDefaults has been reloaded");
}

- (void)reloadDictionarySet {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    NSArray* array = [NSArray arrayWithContentsOfFile:SKKFilePaths::DictionarySet];
    if(array == nil) {
        NSLog(@"DictionarySet.plist can't read.");
    }

    SKKDictionaryKeyContainer keys;
    NSEnumerator* enumerator = [array objectEnumerator];
    
    while(NSDictionary* entry = [enumerator nextObject]) {
        NSNumber* active = [entry valueForKey:@"active"];

        if([active boolValue] == YES) {
            NSNumber* type = [entry valueForKey:@"type"];
            NSString* location = [entry valueForKey:@"location"];

            location = [location stringByExpandingTildeInPath];

            // 自動更新辞書の場合
            if([type intValue] == SKKAutoUpdateDictionaryType) {
                NSString* file = [location lastPathComponent];
                NSString* path = [NSString stringWithFormat:@"%@ %@/%@ %@",
                                           [defaults stringForKey:SKKUserDefaultKeys::openlab_host],
                                           [defaults stringForKey:SKKUserDefaultKeys::openlab_path],
                                           location,
                                           [self pathForUserResource:file]];
                location = path;
            }

            NSLog(@"loading %@", location);

            keys.push_back(SKKDictionaryKey([type intValue], [location UTF8String]));
        }
    }

    NSString* userDictionary = [defaults stringForKey:SKKUserDefaultKeys::user_dictionary_path];
    userDictionary = [userDictionary stringByExpandingTildeInPath];

    SKKBackEnd::theInstance().Initialize([userDictionary UTF8String], keys);

    NSLog(@"DictionarySet has been reloaded");
}

- (void)reloadComponents {
    NSString* tmp;

    tmp = [self pathForResource:@"keymap.conf"];
    SKKPreProcessor::theInstance().Initialize([tmp UTF8String]);

    tmp = [self pathForResource:@"kana-rule.conf"];
    SKKRomanKanaConverter::theInstance().Initialize([tmp UTF8String]);

    NSLog(@"Components has been reloaded");
}

@end

@implementation SKKServer (Local)

- (void)prepareSignalHandler {
    signal(SIGHUP, terminate);
    signal(SIGINT, terminate);
    signal(SIGTERM, terminate);
    signal(SIGPIPE, SIG_IGN);
}

- (void)prepareDirectory {
    NSString* dir = SKKFilePaths::ApplicationSupportFolder;

    if([self fileExistsAtPath:dir] != YES) {
        [self createDirectory:dir];
    }
}

- (void)prepareConnection {
    connection_ = [[NSConnection alloc] init];

    [connection_ registerName:SKKSupervisorConnectionName];
    [connection_ setRootObject:self];
    [connection_ runInNewThread];
}

- (void)prepareUserDefaults {
    NSString* factoryDefaults = [self pathForSystemResource:@"UserDefaults.plist"];
    NSString* userDefaults = SKKFilePaths::UserDefaults;

    NSMutableDictionary* defaults = [NSMutableDictionary dictionaryWithContentsOfFile:factoryDefaults];
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];

    [defaults addEntriesFromDictionary:[NSDictionary dictionaryWithContentsOfFile:userDefaults]];
    [defaults writeToFile:userDefaults atomically:YES];

    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)prepareDictionary {
    NSString* factoryDictionarySet = [self pathForSystemResource:@"DictionarySet.plist"];
    NSString* userDictionarySet = SKKFilePaths::DictionarySet;

    if([self fileExistsAtPath:userDictionarySet] != YES) {
        NSData* data = [NSData dataWithContentsOfFile:factoryDictionarySet];
        [data writeToFile:userDictionarySet atomically:YES];
    }
}

- (void)prepareInputMethodKit {
    NSDictionary* info = [[NSBundle mainBundle] infoDictionary];
    NSString* connection = [info objectForKey:@"InputMethodConnectionName"];
    NSString* identifier = [[NSBundle mainBundle] bundleIdentifier];

    [[IMKServer alloc] initWithName:connection bundleIdentifier:identifier];
}

- (BOOL)fileExistsAtPath:(NSString*)path {
    NSFileManager* fileManager = [NSFileManager defaultManager];

    return [fileManager fileExistsAtPath:path];
}

- (void)createDirectory:(NSString*)path {
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSError* error;

    if([fileManager createDirectoryAtPath:path
                    withIntermediateDirectories:YES
                    attributes:nil error:&error] != YES) {
        NSLog(@"create directory[%@] failed: %@", path, [error localizedDescription]);
    }
}

- (NSString*)pathForSystemResource:(NSString*)path {
    return [NSString stringWithFormat:@"%@/%@", [[NSBundle mainBundle] resourcePath], path];
}

- (NSString*)pathForUserResource:(NSString*)path {
    return [NSString stringWithFormat:@"%@/%@", SKKFilePaths::ApplicationSupportFolder, path];
}

- (NSString*)pathForResource:(NSString*)path {
    NSString* tmp = [self pathForUserResource:path];

    if([self fileExistsAtPath:tmp] == YES) {
        return tmp;
    } else {
        return [self pathForSystemResource:path];
    }
}

@end
