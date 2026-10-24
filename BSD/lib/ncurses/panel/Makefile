# $Id: Makefile.in,v 1.81 2025/10/25 18:00:30 tom Exp $
##############################################################################
# Copyright 2020-2024,2025 Thomas E. Dickey                                  #
# Copyright 1998-2015,2018 Free Software Foundation, Inc.                    #
#                                                                            #
# Permission is hereby granted, free of charge, to any person obtaining a    #
# copy of this software and associated documentation files (the "Software"), #
# to deal in the Software without restriction, including without limitation  #
# the rights to use, copy, modify, merge, publish, distribute, distribute    #
# with modifications, sublicense, and/or sell copies of the Software, and to #
# permit persons to whom the Software is furnished to do so, subject to the  #
# following conditions:                                                      #
#                                                                            #
# The above copyright notice and this permission notice shall be included in #
# all copies or substantial portions of the Software.                        #
#                                                                            #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    #
# THE ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    #
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        #
# DEALINGS IN THE SOFTWARE.                                                  #
#                                                                            #
# Except as contained in this notice, the name(s) of the above copyright     #
# holders shall not be used in advertising or otherwise to promote the sale, #
# use or other dealings in this Software without prior written               #
# authorization.                                                             #
##############################################################################
#
# Author: Thomas E. Dickey 1996-on
#
# Makefile for panels source code.
#
# This makes the following:
#	libraries (normal/debug/profile/shared)
#
# The variable 'srcdir' refers to the source-distribution, and can be set with
# the configure script by "--srcdir=DIR".
#
# The rules are organized to produce the libraries for the configured models,

# turn off _all_ suffix rules; we'll generate our own
.SUFFIXES:

SHELL		= /bin/sh

THIS		= Makefile



TOP_MFLAGS	= DESTDIR="$(DESTDIR)" RPATH_LIST="$(RPATH_LIST)"

x		= 
o		= .o

MODEL		= shared
top_srcdir	= ..
srcdir		= .
prefix		= /usr
exec_prefix	= ${prefix}
bindir		= ${exec_prefix}/bin
libdir		= ${exec_prefix}/lib
includedir	= ${prefix}/include
datarootdir	= ${prefix}/share
datadir		= ${datarootdir}
includesubdir	= 

INCLUDEDIR	= $(DESTDIR)$(includedir)$(includesubdir)
BINDIR		= $(DESTDIR)$(bindir)
LIBDIR		= $(DESTDIR)$(libdir)

PACKAGE		= ncurses

LIBTOOL		= 
LIBTOOL_OPTS	=  
LIBTOOL_CLEAN	= 
LIBTOOL_COMPILE	= 
LIBTOOL_LINK	= ${CC}
LIBTOOL_INSTALL	= 
LIBTOOL_UNINSTALL = 
LT_UNDEF	= 

INSTALL		= /usr/bin/install -c
INSTALL_LIB	= /usr/bin/install -c -m 644
INSTALL_PROG	= ${INSTALL} -s
INSTALL_DATA	= ${INSTALL} -m 644

AR		= ar
ARFLAGS		= -curvU
AWK		= mawk
LD		= ld
LN_S		= ln -s -f

CTAGS		= ctags
ETAGS		= etags

CC		= /Library/Developer/Toolchains/Default.xctoolchain/usr/bin/clang
CPP		= /lib/cpp
CFLAGS		= --sysroot=/Library/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk -mmacos-version-min=10.15 -Qunused-arguments -Wno-error=implicit-function-declaration -no-cpp-precomp 

INCDIR		= $(top_srcdir)/include
BASE_DIR 	= $(top_srcdir)/ncurses
CPPFLAGS	= -I$(BASE_DIR) -DHAVE_CONFIG_H -DBUILDING_PANEL -I. -I../include -D_DARWIN_C_SOURCE -DNDEBUG

CCFLAGS		= $(CPPFLAGS) $(CFLAGS)

CFLAGS_LIBTOOL	= $(CCFLAGS)
CFLAGS_NORMAL	= $(CCFLAGS) -DNCURSES_STATIC
CFLAGS_DEBUG	= $(CCFLAGS) -DNCURSES_STATIC -g -DTRACE
CFLAGS_PROFILE	= $(CCFLAGS) -DNCURSES_STATIC -pg
CFLAGS_SHARED	= $(CCFLAGS) -dynamic

CFLAGS_DEFAULT	= $(CFLAGS_SHARED)

LINK		= $(LIBTOOL_LINK)
LDFLAGS		= --sysroot=/Library/Developer/Platforms/ravynOS.platform/Developer/SDKs/ravynOS.sdk -mmacos-version-min=10.15 -fuse-ld=/Library/Developer/Toolchains/Default.xctoolchain/usr/bin/ld64 -Wl,-search_paths_first  -lpthread 

SHLIB_DIRS	= -L../lib
SHLIB_LIST	= $(SHLIB_DIRS) -lncurses 

RPATH_LIST	= ${libdir}
RESULTING_SYMS	= 
VERSIONED_SYMS	= 
MK_SHARED_LIB	= ${CC} ${LDFLAGS} ${CFLAGS} -dynamiclib -install_name ${libdir}/`basename $@` -compatibility_version ${ABI_VERSION} -current_version ${ABI_VERSION} -o $@

NCURSES_MAJOR	= 6
NCURSES_MINOR	= 6
REL_VERSION	= 6.6
ABI_VERSION	= 6

RANLIB		= ranlib

LIBRARIES	=  ../lib/libpanel.${ABI_VERSION}.dylib ../lib/libpanel.a ../lib/libpanel_g.a

LINT		= 
LINT_OPTS	= 
LINT_LIBS	= -lpanel -lncurses -lpthread 

AUTO_SRC	= \
		../include/panel.h

################################################################################

.PHONY :	all
.PHONY :	clean
.PHONY :	distclean
.PHONY :	install
.PHONY :	install.libs
.PHONY :	libs
.PHONY :	mostlyclean
.PHONY :	realclean
.PHONY :	sources
.PHONY :	uninstall
.PHONY :	uninstall.libs

all \
libs \
install ::	$(AUTO_SRC) $(LIBRARIES)

check ::
	@echo "no unit-test implemented"

sources:	$(AUTO_SRC)

$(BINDIR) \
$(LIBDIR) :
	mkdir -p $@

# make a copy to simplify include-paths while still keeping panel's include
# file in this directory.
../include/panel.h : $(srcdir)/panel.h
	-rm -f $@
	cp $(srcdir)/panel.h $@

HEADER_DEPS = \
	../include/curses.h \
	../include/ncurses_cfg.h \
	../include/ncurses_def.h \
	../include/ncurses_dll.h \
	../include/term.h \
	../include/unctrl.h \
	$(BASE_DIR)/curses.priv.h \
	$(BASE_DIR)/new_pair.h \
	$(INCDIR)/nc_alloc.h \
	$(INCDIR)/nc_panel.h \
	$(INCDIR)/nc_string.h \
	$(INCDIR)/nc_termios.h \
	$(INCDIR)/nc_tparm.h \
	$(INCDIR)/term_entry.h \
	$(srcdir)/panel.priv.h \
	$(srcdir)/panel.h

tags:
	$(CTAGS) *.[ch]

TAGS:
	$(ETAGS) *.[ch]

mostlyclean ::
	-rm -f core tags TAGS *~ *.bak *.i *.ln *.atac trace

clean :: mostlyclean
	-rm -f $(AUTO_SRC)
	-rm -rf .libs *.dSYM

distclean :: clean
	-rm -f Makefile

realclean :: distclean

# These rules are used to allow "make -n" to work on a clean directory-tree
../include/curses.h \
../include/ncurses_def.h \
../include/term.h :
	( cd ../include && $(MAKE) $(TOP_MFLAGS) )

###############################################################################
# The remainder of this file is automatically generated during configuration
###############################################################################

# Generated by CF_LIB_RULES
resulting.map: 
	mawk 'BEGIN { skip = 1; last=""; } /deprecated in ABI6/ { next; }{ sub("NCURSES([WT]+)?", "&6"); } { if ( last != "" && ( skip == 0 || $$0 !~ /}/ ) ) { print last; } skip = 0; last = $$0; } END { print last; }' < "" >$@

distclean::
	rm -f resulting.map

# generated by mk-0th.awk
#   libname:    panel
#   subsets:    ticlib+termlib+ext_tinfo+base+widechar+ext_funcs

.SUFFIXES: .c .cc .h .i .ii
.c.i :
	$(CPP) $(CPPFLAGS) $< >$@
.cc.ii :
	$(CPP) $(CPPFLAGS) $< >$@
.h.i :
	$(CPP) $(CPPFLAGS) $< >$@

C_SRC = \
	$(srcdir)/panel.c \
	$(srcdir)/p_above.c \
	$(srcdir)/p_below.c \
	$(srcdir)/p_bottom.c \
	$(srcdir)/p_delete.c \
	$(srcdir)/p_hide.c \
	$(srcdir)/p_hidden.c \
	$(srcdir)/p_move.c \
	$(srcdir)/p_new.c \
	$(srcdir)/p_replace.c \
	$(srcdir)/p_show.c \
	$(srcdir)/p_top.c \
	$(srcdir)/p_update.c \
	$(srcdir)/p_user.c \
	$(srcdir)/p_win.c

# Producing llib-lpanel is time-consuming, so there's no direct-dependency for
# it in the lintlib rule.  We'll only remove in the cleanest setup.

PANEL_SRC = \
	$(srcdir)/panel.c \
	$(srcdir)/p_above.c \
	$(srcdir)/p_below.c \
	$(srcdir)/p_bottom.c \
	$(srcdir)/p_delete.c \
	$(srcdir)/p_hide.c \
	$(srcdir)/p_hidden.c \
	$(srcdir)/p_move.c \
	$(srcdir)/p_new.c \
	$(srcdir)/p_replace.c \
	$(srcdir)/p_show.c \
	$(srcdir)/p_top.c \
	$(srcdir)/p_update.c \
	$(srcdir)/p_user.c \
	$(srcdir)/p_win.c

clean ::
	rm -f llib-lpanel.*

realclean ::
	rm -f llib-lpanel

llib-lpanel : $(PANEL_SRC)
	cproto -a -l -DNCURSES_ENABLE_STDBOOL_H=0 -DLINT $(CPPFLAGS) $(PANEL_SRC) >$@

lintlib ::
	sh $(srcdir)/../misc/makellib panel $(CPPFLAGS)

lint ::
	$(LINT) $(LINT_OPTS) $(CPPFLAGS) $(PANEL_SRC) $(LINT_LIBS)

# generated by mk-1st.awk (subset=ticlib+termlib+ext_tinfo+base+widechar+ext_funcs)
#  name:            panel
#  traces:          DEBUG
#  MODEL:           SHARED
#  CXX_MODEL:       SHARED
#  LIB_SUFFIX:      
#  ABI_SUFFIX:      
#  model:           obj_s
#  prefix:          lib
#  suffix:          .dylib
#  subset:          ticlib+termlib+ext_tinfo+base+widechar+ext_funcs
#  driver:          no
#  ShlibVer:        abi
#  ShlibVerInfix:   yes
#  SymLink:         ln -s -f
#  TermlibRoot:     ncurses
#  TermlibSuffix:   .dylib
#  ReLink:          yes
#  ReRanlib:        yes
#  DoLinks:         yes
#  rmSoLocs:        no
#  ldconfig:        :
#  make_phony:      yes
#  overwrite:       yes
#  depend:          ../include/ncurses_cfg.h ./panel.priv.h
#  host:            x86_64-apple-darwin19
#  libtool_version: -version-number

SHARED_OBJS = \
	../obj_s/panel$o \
	../obj_s/p_above$o \
	../obj_s/p_below$o \
	../obj_s/p_bottom$o \
	../obj_s/p_delete$o \
	../obj_s/p_hide$o \
	../obj_s/p_hidden$o \
	../obj_s/p_move$o \
	../obj_s/p_new$o \
	../obj_s/p_replace$o \
	../obj_s/p_show$o \
	../obj_s/p_top$o \
	../obj_s/p_update$o \
	../obj_s/p_user$o \
	../obj_s/p_win$o

$(SHARED_OBJS) : ../include/ncurses_cfg.h ./panel.priv.h

../lib/libpanel.$(ABI_VERSION).dylib : \
		$(RESULTING_SYMS) $(SHARED_OBJS)
	@echo linking $@
	@mkdir -p ../lib
	$(MK_SHARED_LIB) $(SHARED_OBJS) $(SHLIB_LIST)
	cd ../lib && ($(LN_S) libpanel.$(ABI_VERSION).dylib libpanel.dylib; )

install \
install.libs \
install.panel :: $(LIBDIR)/libpanel.$(ABI_VERSION).dylib

$(LIBDIR)/libpanel.$(ABI_VERSION).dylib : \
		$(RESULTING_SYMS) $(SHARED_OBJS)
	@echo linking $@
	@mkdir -p $(LIBDIR)
	$(MK_SHARED_LIB) $(SHARED_OBJS) $(SHLIB_LIST)
	cd $(LIBDIR) && ($(LN_S) libpanel.$(ABI_VERSION).dylib libpanel.dylib; )

uninstall \
uninstall.libs \
uninstall.panel ::
	@echo uninstalling $(LIBDIR)/libpanel.$(ABI_VERSION).dylib
	-rm -f $(LIBDIR)/libpanel.$(ABI_VERSION).dylib
	-rm -f $(LIBDIR)/libpanel.dylib

clean ::
	-rm -f ../lib/libpanel.$(ABI_VERSION).dylib
	-rm -f ../lib/libpanel.dylib

mostlyclean::
	-rm -f $(SHARED_OBJS)

# generated by mk-2nd.awk
#   model:      obj_s
#   MODEL:      SHARED
#   echo:       yes
#   subset:     ticlib+termlib+ext_tinfo+base+widechar+ext_funcs
#   crenames:   yes
#   cxxrenames: 
#   traces:     DEBUG
#   srcdir:     .

../obj_s/panel$o :	$(srcdir)/panel.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/panel.c -o ../obj_s/panel$o

../obj_s/p_above$o :	$(srcdir)/p_above.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_above.c -o ../obj_s/p_above$o

../obj_s/p_below$o :	$(srcdir)/p_below.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_below.c -o ../obj_s/p_below$o

../obj_s/p_bottom$o :	$(srcdir)/p_bottom.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_bottom.c -o ../obj_s/p_bottom$o

../obj_s/p_delete$o :	$(srcdir)/p_delete.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_delete.c -o ../obj_s/p_delete$o

../obj_s/p_hide$o :	$(srcdir)/p_hide.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_hide.c -o ../obj_s/p_hide$o

../obj_s/p_hidden$o :	$(srcdir)/p_hidden.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_hidden.c -o ../obj_s/p_hidden$o

../obj_s/p_move$o :	$(srcdir)/p_move.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_move.c -o ../obj_s/p_move$o

../obj_s/p_new$o :	$(srcdir)/p_new.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_new.c -o ../obj_s/p_new$o

../obj_s/p_replace$o :	$(srcdir)/p_replace.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_replace.c -o ../obj_s/p_replace$o

../obj_s/p_show$o :	$(srcdir)/p_show.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_show.c -o ../obj_s/p_show$o

../obj_s/p_top$o :	$(srcdir)/p_top.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_top.c -o ../obj_s/p_top$o

../obj_s/p_update$o :	$(srcdir)/p_update.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_update.c -o ../obj_s/p_update$o

../obj_s/p_user$o :	$(srcdir)/p_user.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_user.c -o ../obj_s/p_user$o

../obj_s/p_win$o :	$(srcdir)/p_win.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_SHARED) -c ../panel/p_win.c -o ../obj_s/p_win$o


# generated by mk-1st.awk (subset=ticlib+termlib+ext_tinfo+base+widechar+ext_funcs)
#  name:            panel
#  traces:          DEBUG
#  MODEL:           NORMAL
#  CXX_MODEL:       NORMAL
#  LIB_SUFFIX:      
#  ABI_SUFFIX:      
#  model:           objects
#  prefix:          lib
#  suffix:          .a
#  subset:          ticlib+termlib+ext_tinfo+base+widechar+ext_funcs
#  driver:          no
#  ShlibVer:        abi
#  ShlibVerInfix:   yes
#  SymLink:         ln -s -f
#  TermlibRoot:     ncurses
#  TermlibSuffix:   .dylib
#  ReLink:          yes
#  ReRanlib:        yes
#  DoLinks:         yes
#  rmSoLocs:        no
#  ldconfig:        :
#  make_phony:      yes
#  overwrite:       yes
#  depend:          ../include/ncurses_cfg.h ./panel.priv.h
#  host:            x86_64-apple-darwin19
#  libtool_version: -version-number

NORMAL_OBJS = \
	../objects/panel$o \
	../objects/p_above$o \
	../objects/p_below$o \
	../objects/p_bottom$o \
	../objects/p_delete$o \
	../objects/p_hide$o \
	../objects/p_hidden$o \
	../objects/p_move$o \
	../objects/p_new$o \
	../objects/p_replace$o \
	../objects/p_show$o \
	../objects/p_top$o \
	../objects/p_update$o \
	../objects/p_user$o \
	../objects/p_win$o

$(NORMAL_OBJS) : ../include/ncurses_cfg.h ./panel.priv.h

../lib/libpanel.a : $(NORMAL_OBJS)
	$(AR) $(ARFLAGS) $@ $?
	$(RANLIB) $@

install \
install.libs \
install.panel :: $(LIBDIR) ../lib/libpanel.a
	@echo installing ../lib/libpanel.a as $(LIBDIR)/libpanel.a
	$(INSTALL_DATA) ../lib/libpanel.a $(LIBDIR)/libpanel.a
	$(RANLIB) $(LIBDIR)/libpanel.a

uninstall \
uninstall.libs \
uninstall.panel ::
	@echo uninstalling $(LIBDIR)/libpanel.a
	-@rm -f $(LIBDIR)/libpanel.a

clean ::
	-rm -f ../lib/libpanel.a

mostlyclean::
	-rm -f $(NORMAL_OBJS)

# generated by mk-2nd.awk
#   model:      objects
#   MODEL:      NORMAL
#   echo:       yes
#   subset:     ticlib+termlib+ext_tinfo+base+widechar+ext_funcs
#   crenames:   yes
#   cxxrenames: 
#   traces:     DEBUG
#   srcdir:     .

../objects/panel$o :	$(srcdir)/panel.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/panel.c -o ../objects/panel$o

../objects/p_above$o :	$(srcdir)/p_above.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_above.c -o ../objects/p_above$o

../objects/p_below$o :	$(srcdir)/p_below.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_below.c -o ../objects/p_below$o

../objects/p_bottom$o :	$(srcdir)/p_bottom.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_bottom.c -o ../objects/p_bottom$o

../objects/p_delete$o :	$(srcdir)/p_delete.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_delete.c -o ../objects/p_delete$o

../objects/p_hide$o :	$(srcdir)/p_hide.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_hide.c -o ../objects/p_hide$o

../objects/p_hidden$o :	$(srcdir)/p_hidden.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_hidden.c -o ../objects/p_hidden$o

../objects/p_move$o :	$(srcdir)/p_move.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_move.c -o ../objects/p_move$o

../objects/p_new$o :	$(srcdir)/p_new.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_new.c -o ../objects/p_new$o

../objects/p_replace$o :	$(srcdir)/p_replace.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_replace.c -o ../objects/p_replace$o

../objects/p_show$o :	$(srcdir)/p_show.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_show.c -o ../objects/p_show$o

../objects/p_top$o :	$(srcdir)/p_top.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_top.c -o ../objects/p_top$o

../objects/p_update$o :	$(srcdir)/p_update.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_update.c -o ../objects/p_update$o

../objects/p_user$o :	$(srcdir)/p_user.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_user.c -o ../objects/p_user$o

../objects/p_win$o :	$(srcdir)/p_win.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_NORMAL) -c ../panel/p_win.c -o ../objects/p_win$o


# generated by mk-1st.awk (subset=ticlib+termlib+ext_tinfo+base+widechar+ext_funcs)
#  name:            panel
#  traces:          DEBUG
#  MODEL:           DEBUG
#  CXX_MODEL:       DEBUG
#  LIB_SUFFIX:      
#  ABI_SUFFIX:      
#  model:           obj_g
#  prefix:          lib
#  suffix:          _g.a
#  subset:          ticlib+termlib+ext_tinfo+base+widechar+ext_funcs
#  driver:          no
#  ShlibVer:        abi
#  ShlibVerInfix:   yes
#  SymLink:         ln -s -f
#  TermlibRoot:     ncurses
#  TermlibSuffix:   .dylib
#  ReLink:          yes
#  ReRanlib:        yes
#  DoLinks:         yes
#  rmSoLocs:        no
#  ldconfig:        :
#  make_phony:      yes
#  overwrite:       yes
#  depend:          ../include/ncurses_cfg.h ./panel.priv.h
#  host:            x86_64-apple-darwin19
#  libtool_version: -version-number

DEBUG_OBJS = \
	../obj_g/panel$o \
	../obj_g/p_above$o \
	../obj_g/p_below$o \
	../obj_g/p_bottom$o \
	../obj_g/p_delete$o \
	../obj_g/p_hide$o \
	../obj_g/p_hidden$o \
	../obj_g/p_move$o \
	../obj_g/p_new$o \
	../obj_g/p_replace$o \
	../obj_g/p_show$o \
	../obj_g/p_top$o \
	../obj_g/p_update$o \
	../obj_g/p_user$o \
	../obj_g/p_win$o

$(DEBUG_OBJS) : ../include/ncurses_cfg.h ./panel.priv.h

../lib/libpanel_g.a : $(DEBUG_OBJS)
	$(AR) $(ARFLAGS) $@ $?
	$(RANLIB) $@

install \
install.libs \
install.panel :: $(LIBDIR) ../lib/libpanel_g.a
	@echo installing ../lib/libpanel_g.a as $(LIBDIR)/libpanel_g.a
	$(INSTALL_DATA) ../lib/libpanel_g.a $(LIBDIR)/libpanel_g.a
	$(RANLIB) $(LIBDIR)/libpanel_g.a

uninstall \
uninstall.libs \
uninstall.panel ::
	@echo uninstalling $(LIBDIR)/libpanel_g.a
	-@rm -f $(LIBDIR)/libpanel_g.a

clean ::
	-rm -f ../lib/libpanel_g.a

mostlyclean::
	-rm -f $(DEBUG_OBJS)

# generated by mk-2nd.awk
#   model:      obj_g
#   MODEL:      DEBUG
#   echo:       yes
#   subset:     ticlib+termlib+ext_tinfo+base+widechar+ext_funcs
#   crenames:   yes
#   cxxrenames: 
#   traces:     DEBUG
#   srcdir:     .

../obj_g/panel$o :	$(srcdir)/panel.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/panel.c -o ../obj_g/panel$o

../obj_g/p_above$o :	$(srcdir)/p_above.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_above.c -o ../obj_g/p_above$o

../obj_g/p_below$o :	$(srcdir)/p_below.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_below.c -o ../obj_g/p_below$o

../obj_g/p_bottom$o :	$(srcdir)/p_bottom.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_bottom.c -o ../obj_g/p_bottom$o

../obj_g/p_delete$o :	$(srcdir)/p_delete.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_delete.c -o ../obj_g/p_delete$o

../obj_g/p_hide$o :	$(srcdir)/p_hide.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_hide.c -o ../obj_g/p_hide$o

../obj_g/p_hidden$o :	$(srcdir)/p_hidden.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_hidden.c -o ../obj_g/p_hidden$o

../obj_g/p_move$o :	$(srcdir)/p_move.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_move.c -o ../obj_g/p_move$o

../obj_g/p_new$o :	$(srcdir)/p_new.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_new.c -o ../obj_g/p_new$o

../obj_g/p_replace$o :	$(srcdir)/p_replace.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_replace.c -o ../obj_g/p_replace$o

../obj_g/p_show$o :	$(srcdir)/p_show.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_show.c -o ../obj_g/p_show$o

../obj_g/p_top$o :	$(srcdir)/p_top.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_top.c -o ../obj_g/p_top$o

../obj_g/p_update$o :	$(srcdir)/p_update.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_update.c -o ../obj_g/p_update$o

../obj_g/p_user$o :	$(srcdir)/p_user.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_user.c -o ../obj_g/p_user$o

../obj_g/p_win$o :	$(srcdir)/p_win.c \
			$(HEADER_DEPS)
	$(LIBTOOL_COMPILE) $(CC) $(CFLAGS_DEBUG) -c ../panel/p_win.c -o ../obj_g/p_win$o


# generated by mk-hdr.awk
#  subset:     ticlib+termlib+ext_tinfo+base+widechar+ext_funcs
#  compat:     yes

${INCLUDEDIR} :
	mkdir -p $@

install \
install.libs \
install.includes :: ${AUTO_SRC} ${INCLUDEDIR} \
		$(srcdir)/panel.h
	@ (cd ${INCLUDEDIR} && rm -f panel.h) ; ../headers.sh ${INSTALL_DATA} ${INCLUDEDIR} ${srcdir} $(srcdir)/panel.h

uninstall \
uninstall.libs \
uninstall.includes ::
	-@ (cd ${INCLUDEDIR} && rm -f panel.h)

.PHONY :	depend
depend : ${AUTO_SRC}
	makedepend -- ${CPPFLAGS} -- ${C_SRC}

# DO NOT DELETE THIS LINE -- make depend depends on it.
