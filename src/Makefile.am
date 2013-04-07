BSRC=$(top_srcdir)/bstrlib

AM_CPPFLAGS = -I$(YAML_CFLAGS) -I$(ARGTABLE_CFLAGS) -I$(BSRC)
bin_PROGRAMS = chokus

chokus_SOURCES = $(BSRC)/bstrlib.c $(BSRC)/bstraux.c ck_log.c ck_config.c main.c
chokus_LDADD = $(YAML_LIBS) $(ARGTABLE_LIBS)

