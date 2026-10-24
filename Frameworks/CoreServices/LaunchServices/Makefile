MK_WERROR = no
FRAMEWORK = LaunchServices
FMWK_VERSION = A

SRCS = LaunchServices.mm LSAppRecord.mm UTTypes.mm LSIPC.mm
HEADERS = LaunchServices.h UTCoreTypes.h UTTypes.h
RESOURCES= Info.plist InitDB.sql DBSchema_3_4.sql

CFLAGS = -fobjc-arc -framework Foundation
LDFLAGS = -lsqlite3 -llaunch

fmwk-install-hook:

.include <rvn.framework.mk>

