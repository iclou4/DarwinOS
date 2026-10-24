
MK_WERROR = no

FRAMEWORK = CFNetwork
FMWK_VERSION = A
VERSION = 7
INFO_PLIST = Info.plist

HEADERS = CFFTPStream.h CFHost.h CFHTTPAuthentication.h CFHTTPMessage.h \
	CFHTTPStream.h CFNetDiagnostics.h CFNetServices.h CFNetwork.h \
	CFNetworkDefs.h CFSocketStream.h

PRIVATE_HEADERS = CFFTPStreamPriv.h CFHostPriv.h CFHTTPConnectionPriv.h \
	CFHTTPMessagePriv.h CFHTTPServerPriv.h CFHTTPStreamPriv.h \
	CFNetDiagnosticsPriv.h CFNetServicesPriv.h CFNetworkPriv.h \
	CFServerPriv.h CFSocketStreamPriv.h

SLF = /System/Library/Frameworks
INSTALL_NAME_DIR = ${SLF}/${FRAMEWORK}.framework/Versions/${FMWK_VERSION}

SRCS = CFNetwork.c SharedCode/CFServer.c SharedCode/CFNetConnection.c \
	SharedCode/CFNetworkSchedule.c SharedCode/CFNetworkThreadSupport.c \
	FTP/CFFTPStream.c Host/CFHost.c \
	HTTP/CFHTTPAuthentication.c HTTP/CFHTTPConnection.c \
	HTTP/CFHTTPFilter.c HTTP/CFHTTPMessage.c HTTP/CFHTTPServer.c \
	HTTP/CFHTTPStream.c NetDiagnostics/CFNetDiagnosticPing.c \
	NetDiagnostics/CFNetDiagnostics.c \
	NetDiagnostics/CFNetDiagnosticsProtocolUser.c \
	NetServices/CFNetServices.c NetServices/CFNetServiceBrowser.c \
	NetServices/CFNetServiceMonitor.c \
	NetServices/DeprecatedDNSServiceDiscovery.c Proxies/ProxySupport.c \
	Stream/CFSocketStream.c URL/_CFURLAccess.c JavaScriptGlue.c \
	libresolv.c HTTP/SPNEGO/spnegoBlob.cpp HTTP/SPNEGO/spnegoDER.cpp \
	HTTP/SPNEGO/spnegoKrb.cpp HTTP/NTLM/ntlmBlobPriv.cpp \
	HTTP/NTLM/NtlmGenerator.cpp

LDFLAGS = -framework CoreFoundation -Wl,-compatibility_version,1
#-framework Security -framework SystemConfiguration \
	-F/usr/local/SecurityPieces/Frameworks -framework security_cdsa_utils

INCLUDES = -I${.CURDIR:H} -I${ROOT_SOURCE_DIR}/Libraries/Libsystem/liblaunch \
	-I${.CURDIR}/SharedCode -I${.CURDIR}/Proxies -I${.CURDIR}/HTTP \
	-I${.CURDIR} -I${ROOT_SOURCE_DIR}/Kernel/kext_tools/FILES/3rd
DEFS = -DCF_ASSUME_NONNULL_BEGIN="" -DCF_ASSUME_NONNULL_END=""

CFLAGS += -F${RAVYN_SDKROOT}${SLF}/CoreServices.framework/Frameworks \
	-Wmissing-prototypes -Wpointer-arith -Wcast-align \
	-Wno-unknown-pragmas -Wmissing-declarations -Wbad-function-cast \
	-Wno-incompatible-function-pointer-types ${DEFS} ${INCLUDES}
CPPFLAGS += -F${RAVYN_SDKROOT}${SLF}/CoreServices.framework/Frameworks \
	-Wmissing-prototypes -Wpointer-arith -Wcast-align -Wno-unknown-pragmas \
	-Wno-incompatible-function-pointer-types ${DEFS} ${INCLUDES}


.include <rvn.framework.mk>
