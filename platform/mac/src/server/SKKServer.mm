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
#include "SKKCommonDictionary.h"
#include "SKKAutoUpdateDictionary.h"
#include "SKKProxyDictionary.h"
#include "SKKGadgetDictionary.h"
#include "SKKDictionaryFactory.h"
#include "SKKConstVars.h"
#include "MacKotoeriDictionary.h"
#include "skkserv.h"
#include "InputModeWindow.h"

#include <InputMethodKit/InputMethodKit.h>
#include <signal.h>

namespace {
    // 順番の入れ替えは禁止(追加のみ)
    struct DictionaryTypes {
        enum {
            Common,
            AutoUpdate,
            Proxy,
            Kotoeri,
            Gadget,
            CommonUTF8,
        };
    };

    NSString* DictionaryNames[] = {
        @"SKK 辞書",
        @"SKK 辞書(自動更新)",
        @"skkserv 辞書",
        @"ことえり辞書",
        @"プログラム辞書",
        @"SKK 辞書(UTF-8)"
    };

    const struct {
        int mode;
        NSString* name;
    } InputModeIcons[] = {
        { HirakanaInputMode,		@"AquaSKK-Hirakana.png" },
        { KatakanaInputMode,		@"AquaSKK-Katakana.png" },
        { Jisx0201KanaInputMode,	@"AquaSKK-Jisx0201Kana.png" },
        { AsciiInputMode,		@"AquaSKK-Ascii.png" },
        { Jisx0208LatinInputMode,	@"AquaSKK-Jisx0208Latin.png" },
        { 0,				0 }
    };
}

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

- (void)initializeInputModeIcons;
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

    NSLog(@"loading UserDefaults ...");

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

    flag = [defaults boolForKey:SKKUserDefaultKeys::enable_private_mode] == YES;
    SKKBackEnd::theInstance().EnablePrivateMode(flag);

    int length = [defaults integerForKey:SKKUserDefaultKeys::minimum_completion_length];
    SKKBackEnd::theInstance().SetMinimumCompletionLength(length);
}

- (void)reloadDictionarySet {
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];

    NSLog(@"loading DictionarySet ...");

    NSArray* array = [NSArray arrayWithContentsOfFile:SKKFilePaths::DictionarySet];
    if(array == nil) {
        NSLog(@"can't read DictionarySet.plist");
    }

    SKKDictionaryKeyContainer keys;
    NSEnumerator* enumerator = [array objectEnumerator];
    
    while(NSDictionary* entry = [enumerator nextObject]) {
        NSNumber* active = [entry valueForKey:SKKDictionarySetKeys::active];

        if([active boolValue] == YES) {
            NSNumber* type = [entry valueForKey:SKKDictionarySetKeys::type];
            NSString* location = [entry valueForKey:SKKDictionarySetKeys::location];

            if(location) {
                location = [location stringByExpandingTildeInPath];

                // 自動更新辞書の場合
                if([type intValue] == DictionaryTypes::AutoUpdate) {
                    NSString* file = [location lastPathComponent];
                    NSString* path = [NSString stringWithFormat:@"%@ %@/%@ %@",
                                               [defaults stringForKey:SKKUserDefaultKeys::openlab_host],
                                               [defaults stringForKey:SKKUserDefaultKeys::openlab_path],
                                               location,
                                               [self pathForUserResource:file]];
                    location = path;
                }
            } else {
                location = @"[location was not specified]";
            }

            NSLog(@"loading %@", location);
            keys.push_back(SKKDictionaryKey([type intValue], [location UTF8String]));
        }
    }

    NSString* userDictionary = [defaults stringForKey:SKKUserDefaultKeys::user_dictionary_path];
    userDictionary = [userDictionary stringByExpandingTildeInPath];

    SKKBackEnd::theInstance().Initialize([userDictionary UTF8String], keys);
}

- (void)reloadComponents {
    NSString* tmp;

    NSLog(@"loading Components ...");

    tmp = [self pathForResource:@"keymap.conf"];
    SKKPreProcessor::theInstance().Initialize([tmp UTF8String]);

    tmp = [self pathForResource:@"kana-rule.conf"];
    SKKRomanKanaConverter::theInstance().Initialize([tmp UTF8String]);

    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    NSArray* subRules = [defaults arrayForKey:SKKUserDefaultKeys::sub_rules];

    for(NSString* path in subRules) {
        SKKRomanKanaConverter::theInstance().Patch([path UTF8String]);
    }

    [self initializeInputModeIcons];
}

- (NSArray*)createDictionaryTypes {
    int order[] = {
        DictionaryTypes::Common,
        DictionaryTypes::CommonUTF8,
        DictionaryTypes::AutoUpdate,
        DictionaryTypes::Proxy,
        DictionaryTypes::Kotoeri,
        DictionaryTypes::Gadget,
        0xff
    };
    NSMutableArray* types = [[NSMutableArray alloc] init];

    for(int index = 0; order[index] != 0xff; ++ index) {
        NSNumber* type = [NSNumber numberWithInt:order[index]];
        NSString* name = DictionaryNames[order[index]];
        NSDictionary* entity = [NSDictionary dictionaryWithObjectsAndKeys:
                                             type, SKKDictionaryTypeKeys::type,
                                             name, SKKDictionaryTypeKeys::name,
                                             nil];
        [types addObject:entity];
    }

    return [types autorelease];
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

    SKKRegisterFactoryMethod<SKKCommonDictionary>(DictionaryTypes::Common);
    SKKRegisterFactoryMethod<SKKCommonDictionaryUTF8>(DictionaryTypes::CommonUTF8);
    SKKRegisterFactoryMethod<SKKAutoUpdateDictionary>(DictionaryTypes::AutoUpdate);
    SKKRegisterFactoryMethod<SKKProxyDictionary>(DictionaryTypes::Proxy);
    SKKRegisterFactoryMethod<MacKotoeriDictionary>(DictionaryTypes::Kotoeri);
    SKKRegisterFactoryMethod<SKKGadgetDictionary>(DictionaryTypes::Gadget);
}

- (void)prepareInputMethodKit {
    NSDictionary* info = [[NSBundle mainBundle] infoDictionary];
    NSString* connection = [info objectForKey:@"InputMethodConnectionName"];
    NSString* identifier = [[NSBundle mainBundle] bundleIdentifier];

    [[IMKServer alloc] initWithName:connection bundleIdentifier:identifier];
}

- (void)initializeInputModeIcons {
    NSMutableDictionary* icons = [[NSMutableDictionary alloc] initWithCapacity:0];

    for(int i = 0; InputModeIcons[i].name != 0; ++ i) {
        NSString* path = [self pathForResource:InputModeIcons[i].name];
        NSImage* image = [[NSImage alloc] initWithContentsOfFile:path];
        [icons setObject:image forKey:[NSNumber numberWithInt:InputModeIcons[i].mode]];
        [image release];
    }

    [[InputModeWindow sharedWindow] setModeIcons:icons];

    [icons release];
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
    return [NSString stringWithFormat:@"%@/%@", SKKFilePaths::SystemResourceFolder, path];
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
