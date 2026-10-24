SUBDIR =
MK_WERROR = no
.PATH: ${.CURDIR}

LIB = z
SRCDIR = ${.CURDIR}/
OBJZ = adler32.o crc32.o deflate.o infback.o inffast.o inflate.o inftrees.o \
        trees.o zutil.o
OBJG = compress.o uncompr.o gzclose.o gzlib.o gzread.o gzwrite.o
OBJS = ${OBJZ} ${OBJG}

CFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fPIC -arch x86_64 -m64 -DNO_FSEEKO -DNO_STRERROR \
        -DHAVE_vsnprintf -DHAVE_HIDDEN
ASFLAGS = --sysroot=${RAVYN_SDKROOT} -mmacos-version-min=${MACOS_VERSION_MIN} \
        -fPIC -arch x86_64 -m64 -DNO_FSEEKO -DNO_STRERROR \
        -DHAVE_vsnprintf -DHAVE_HIDDEN
LDFLAGS = -fuse-ld=${LD} -arch x86_64
ARFLAGS = rcs

MK_MANCOMPRESS = no
MAN = ${.CURDIR}/zlib.3

ZINC=
ZINCOUT=-I.

all: lib${LIB}.a 
	$(AR) $(ARFLAGS) $@ $(OBJS)
	-@ ($(RANLIB) $@ || true) >/dev/null 2>&1
	mkdir -p ${RAVYN_SDKROOT}/usr/lib ${RAVYN_SDKROOT}/usr/share/man/man3
	cp -f ${.OBJDIR}/lib${LIB}.a ${RAVYN_SDKROOT}/usr/lib/
	cp -f ${MAN} ${RAVYN_SDKROOT}/usr/share/man/man3/

match.o: match.S
	$(CPP) match.S > _match.s
	$(CC) -c _match.s
	mv _match.o match.o
	rm -f _match.s

match.lo: match.S
	$(CPP) match.S > _match.s
	$(CC) -c -fPIC _match.s
	mv _match.o match.lo
	rm -f _match.s

adler32.o: $(SRCDIR)adler32.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)adler32.c

crc32.o: $(SRCDIR)crc32.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)crc32.c

crc32_vx.o: $(SRCDIR)contrib/crc32vx/crc32_vx.c
	$(CC) $(CFLAGS) $(VGFMAFLAG) $(ZINC) -c -o $@ $(SRCDIR)contrib/crc32vx/crc32_vx.c

deflate.o: $(SRCDIR)deflate.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)deflate.c

infback.o: $(SRCDIR)infback.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)infback.c

inffast.o: $(SRCDIR)inffast.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)inffast.c

inflate.o: $(SRCDIR)inflate.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)inflate.c

inftrees.o: $(SRCDIR)inftrees.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)inftrees.c

trees.o: $(SRCDIR)trees.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)trees.c

zutil.o: $(SRCDIR)zutil.c $(SRCDIR)gzguts.h
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)zutil.c

compress.o: $(SRCDIR)compress.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)compress.c

uncompr.o: $(SRCDIR)uncompr.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)uncompr.c

gzclose.o: $(SRCDIR)gzclose.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)gzclose.c

gzlib.o: $(SRCDIR)gzlib.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)gzlib.c

gzread.o: $(SRCDIR)gzread.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)gzread.c

gzwrite.o: $(SRCDIR)gzwrite.c
	$(CC) $(CFLAGS) $(ZINC) -c -o $@ $(SRCDIR)gzwrite.c


adler32.lo: $(SRCDIR)adler32.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/adler32.o $(SRCDIR)adler32.c
	-@mv objs/adler32.o $@

crc32.lo: $(SRCDIR)crc32.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/crc32.o $(SRCDIR)crc32.c
	-@mv objs/crc32.o $@

crc32_vx.lo: $(SRCDIR)contrib/crc32vx/crc32_vx.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(VGFMAFLAG) $(ZINC) -DPIC -c -o objs/crc32_vx.o $(SRCDIR)contrib/crc32vx/crc32_vx.c
	-@mv objs/crc32_vx.o $@

deflate.lo: $(SRCDIR)deflate.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/deflate.o $(SRCDIR)deflate.c
	-@mv objs/deflate.o $@

infback.lo: $(SRCDIR)infback.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/infback.o $(SRCDIR)infback.c
	-@mv objs/infback.o $@

inffast.lo: $(SRCDIR)inffast.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/inffast.o $(SRCDIR)inffast.c
	-@mv objs/inffast.o $@

inflate.lo: $(SRCDIR)inflate.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/inflate.o $(SRCDIR)inflate.c
	-@mv objs/inflate.o $@

inftrees.lo: $(SRCDIR)inftrees.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/inftrees.o $(SRCDIR)inftrees.c
	-@mv objs/inftrees.o $@

trees.lo: $(SRCDIR)trees.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/trees.o $(SRCDIR)trees.c
	-@mv objs/trees.o $@

zutil.lo: $(SRCDIR)zutil.c $(SRCDIR)gzguts.h
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/zutil.o $(SRCDIR)zutil.c
	-@mv objs/zutil.o $@

compress.lo: $(SRCDIR)compress.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/compress.o $(SRCDIR)compress.c
	-@mv objs/compress.o $@

uncompr.lo: $(SRCDIR)uncompr.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/uncompr.o $(SRCDIR)uncompr.c
	-@mv objs/uncompr.o $@

gzclose.lo: $(SRCDIR)gzclose.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/gzclose.o $(SRCDIR)gzclose.c
	-@mv objs/gzclose.o $@

gzlib.lo: $(SRCDIR)gzlib.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/gzlib.o $(SRCDIR)gzlib.c
	-@mv objs/gzlib.o $@

gzread.lo: $(SRCDIR)gzread.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/gzread.o $(SRCDIR)gzread.c
	-@mv objs/gzread.o $@

gzwrite.lo: $(SRCDIR)gzwrite.c
	-@mkdir objs 2>/dev/null || test -d objs
	$(CC) $(SFLAGS) $(ZINC) -DPIC -c -o objs/gzwrite.o $(SRCDIR)gzwrite.c
	-@mv objs/gzwrite.o $@


placebo $(SHAREDLIBV): $(PIC_OBJS) libz.a $(SRCDIR)zlib.map
	$(LDSHARED) $(SFLAGS) -o $@ $(PIC_OBJS) $(LDSHAREDLIBC) $(LDFLAGS)
	rm -f $(SHAREDLIB) $(SHAREDLIBM)
	ln -s $@ $(SHAREDLIB)
	ln -s $@ $(SHAREDLIBM)
	-@rmdir objs

install-libs: $(LIBS)
	-@if [ ! -d $(DESTDIR)$(exec_prefix)  ]; then mkdir -p $(DESTDIR)$(exec_prefix); fi
	-@if [ ! -d $(DESTDIR)$(libdir)       ]; then mkdir -p $(DESTDIR)$(libdir); fi
	-@if [ ! -d $(DESTDIR)$(sharedlibdir) ]; then mkdir -p $(DESTDIR)$(sharedlibdir); fi
	-@if [ ! -d $(DESTDIR)$(man3dir)      ]; then mkdir -p $(DESTDIR)$(man3dir); fi
	-@if [ ! -d $(DESTDIR)$(pkgconfigdir) ]; then mkdir -p $(DESTDIR)$(pkgconfigdir); fi
	rm -f $(DESTDIR)$(libdir)/$(STATICLIB)
	cp $(STATICLIB) $(DESTDIR)$(libdir)
	chmod 644 $(DESTDIR)$(libdir)/$(STATICLIB)
	-@($(RANLIB) $(DESTDIR)$(libdir)/libz.a || true) >/dev/null 2>&1
	-@if test -n "$(SHAREDLIBV)"; then \
	  rm -f $(DESTDIR)$(sharedlibdir)/$(SHAREDLIBV); \
	  cp $(SHAREDLIBV) $(DESTDIR)$(sharedlibdir); \
	  echo "cp $(SHAREDLIBV) $(DESTDIR)$(sharedlibdir)"; \
	  chmod 755 $(DESTDIR)$(sharedlibdir)/$(SHAREDLIBV); \
	  echo "chmod 755 $(DESTDIR)$(sharedlibdir)/$(SHAREDLIBV)"; \
	  rm -f $(DESTDIR)$(sharedlibdir)/$(SHAREDLIB) $(DESTDIR)$(sharedlibdir)/$(SHAREDLIBM); \
	  ln -s $(SHAREDLIBV) $(DESTDIR)$(sharedlibdir)/$(SHAREDLIB); \
	  ln -s $(SHAREDLIBV) $(DESTDIR)$(sharedlibdir)/$(SHAREDLIBM); \
	  ($(LDCONFIG) || true)  >/dev/null 2>&1; \
	fi
	rm -f $(DESTDIR)$(man3dir)/zlib.3
	cp $(SRCDIR)zlib.3 $(DESTDIR)$(man3dir)
	chmod 644 $(DESTDIR)$(man3dir)/zlib.3
	rm -f $(DESTDIR)$(pkgconfigdir)/zlib.pc
	cp zlib.pc $(DESTDIR)$(pkgconfigdir)
	chmod 644 $(DESTDIR)$(pkgconfigdir)/zlib.pc
# The ranlib in install is needed on NeXTSTEP which checks file times
# ldconfig is for Linux

install: install-libs
	-@if [ ! -d $(DESTDIR)$(includedir)   ]; then mkdir -p $(DESTDIR)$(includedir); fi
	rm -f $(DESTDIR)$(includedir)/zlib.h $(DESTDIR)$(includedir)/zconf.h
	cp $(SRCDIR)zlib.h zconf.h $(DESTDIR)$(includedir)
	chmod 644 $(DESTDIR)$(includedir)/zlib.h $(DESTDIR)$(includedir)/zconf.h

uninstall:
	cd $(DESTDIR)$(includedir) && rm -f zlib.h zconf.h
	cd $(DESTDIR)$(libdir) && rm -f libz.a; \
	if test -n "$(SHAREDLIBV)" -a -f $(SHAREDLIBV); then \
	  rm -f $(SHAREDLIBV) $(SHAREDLIB) $(SHAREDLIBM); \
	fi
	cd $(DESTDIR)$(man3dir) && rm -f zlib.3
	cd $(DESTDIR)$(pkgconfigdir) && rm -f zlib.pc

# zconf.h.cmakein: $(SRCDIR)zconf.h.in
# 	-@ TEMPFILE=zconfh_$$; \
# 	echo "/#define ZCONF_H/ a\\\\\n#cmakedefine Z_PREFIX\\\\\n#cmakedefine Z_HAVE_UNISTD_H\n" >> $$TEMPFILE &&\
# 	sed -f $$TEMPFILE $(SRCDIR)zconf.h.in > $@ &&\
# 	touch -r $(SRCDIR)zconf.h.in $@ &&\
# 	rm $$TEMPFILE
# 

zconf: $(SRCDIR)zconf.h.in
	cp -p $(SRCDIR)zconf.h.in zconf.h

mostlyclean: clean
clean:
	rm -f *.o *.lo *~ \
	   infcover \
	   libz.* foo.gz so_locations \
	   _match.s maketree contrib/infback9/*.o
	rm -rf objs
	rm -f *.gcda *.gcno *.gcov

adler32.o: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h
zutil.o: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)gzguts.h
gzclose.o gzlib.o gzread.o gzwrite.o: $(SRCDIR)zlib.h zconf.h $(SRCDIR)gzguts.h
compress.o example.o minigzip.o uncompr.o: $(SRCDIR)zlib.h zconf.h
crc32.o: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)crc32.h
deflate.o: $(SRCDIR)deflate.h $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h
infback.o inflate.o: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)inftrees.h $(SRCDIR)inflate.h $(SRCDIR)inffast.h $(SRCDIR)inffixed.h
inffast.o: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)inftrees.h $(SRCDIR)inflate.h $(SRCDIR)inffast.h
inftrees.o: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)inftrees.h
trees.o: $(SRCDIR)deflate.h $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)trees.h
crc32_vx.o: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)contrib/crc32vx/crc32_vx_hooks.h

adler32.lo: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h
zutil.lo: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)gzguts.h
gzclose.lo gzlib.lo gzread.lo gzwrite.lo: $(SRCDIR)zlib.h zconf.h $(SRCDIR)gzguts.h
compress.lo example.lo minigzip.lo uncompr.lo: $(SRCDIR)zlib.h zconf.h
crc32.lo: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)crc32.h
deflate.lo: $(SRCDIR)deflate.h $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h
infback.lo inflate.lo: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)inftrees.h $(SRCDIR)inflate.h $(SRCDIR)inffast.h $(SRCDIR)inffixed.h
inffast.lo: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)inftrees.h $(SRCDIR)inflate.h $(SRCDIR)inffast.h
inftrees.lo: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)inftrees.h
trees.lo: $(SRCDIR)deflate.h $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)trees.h
crc32_vx.lo: $(SRCDIR)zutil.h $(SRCDIR)zlib.h zconf.h $(SRCDIR)contrib/crc32vx/crc32_vx_hooks.h

.include <bsd.lib.mk>

