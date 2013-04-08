BSRC=$(top_srcdir)/bstrlib
SSRC=$(top_srcdir)/src

AM_CPPFLAGS = -I$(YAML_CFLAGS) -I$(ARGTABLE_CFLAGS) -I$(BSRC) -I$(SSRC)
check_PROGRAMS = cktest

cktest_SOURCES = $(BSRC)/bstrlib.c $(BSRC)/bstraux.c $(SSRC)/ck_log.c $(SSRC)/ck_config.c $(SSRC)/ck_ops.c ckTest_main.c ckTest_ops.c
cktest_LDADD = $(YAML_LIBS) $(ARGTABLE_LIBS)

TESTS=cktest
