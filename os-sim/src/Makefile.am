AM_CPPFLAGS =  $(LIBGDA_CFLAGS) \
  $(GNET_CFLAGS) \
  $(OPENSSL_CFLAGS) \
  $(LIBXML_CFLAGS) \
  $(GLIB_CFLAGS) \
  $(GIO_CFLAGS) \
  $(JSON_GLIB_CFLAGS) \
  $(LIBGEOIP_CFLAGS) \
  $(LIBSOUP_CFLAGS) \
  $(LIBBSON_CFLAGS) \
  $(LIBMONGOC_CFLAGS) \
  $(LIBZLIB_CFLAGS) \
  -DG_LOG_DOMAIN=\"OSSIM\" \
  -DOS_SIM_GLOBAL_CONFIG_FILE=\""$(sysconfdir)/ossim/server/config.xml"\" \
  -DOS_SIM_GLOBAL_DIRECTIVE_FILE=\""$(sysconfdir)/ossim/server/directives.xml"\" \
  -DOS_SIM_GLOBAL_WSD_FILE="\"$(sysconfdir)/ossim/server/directives.xss"\" \
  -DOS_SIM_LOG_DIR=\""$(localstatedir)/log/alienvault/server/"\" \
  -DOS_SIM_RUN_FILE=\""$(localstatedir)/run/alienvault/ossim-server.pid"\" \
  -DOS_SIM_VERSION=\""$(OS_SIM_VERSION)"\"

#
#
noinst_LIBRARIES = libparser.a
libparser_a_SOURCES = sim-parser.c sim-parser-connect.c sim-parser-event.c sim-parser-pong.c sim-parser-plugins.c \
        sim-parser-agent-date.c sim-parser-append-plugins.c sim-parser-idm-event.c
libparser_a_CFLAGS = $(LIBGDA_CFLAGS) $(GLIB_CFLAGS) $(GNET_CFLAGS) $(LIBBSON_CFLAGS) $(LIBMONGOC_CFLAGS)
#

ossim_server_CFLAGS = $(AM_CFLAGS)

bin_PROGRAMS = ossim-server


ossim_server_SOURCES = \
		sim-geoip.c \
		sim-alarm-stats.c \
		sim-idm.c \
		sim-idm-entry.c \
		sim-idm-anomalies.c \
		sim-util.c \
		sim-unittesting.c \
		sim-radix.c \
		sim-reputation.c \
		sim-inet.c \
		sim-network.c \
		sim-config.c \
		sim-database.c \
		sim-event.c \
		sim-plugin.c \
		sim-plugin-sid.c \
		sim-sensor.c \
		sim-host.c \
		sim-net.c \
		sim-timezone.c \
		sim-policy.c \
		sim-rule.c \
		sim-directive-group.c \
		sim-directive.c \
		sim-container.c \
		sim-xml-config.c \
		sim-xml-directive.c \
		sim-command.c \
		sim-plugin-state.c \
		sim-session.c \
		sim-scheduler.c \
		sim-organizer.c \
		sim-connect.c \
		sim-log.c \
		sim-server.c \
		sim-timetable.c \
		sim-text-fields.c \
		sim-debug.c \
		sim-groupalarm.c \
		sim-context.c \
		sim-engine.c \
		sim-mini-object.c \
		sim-db-command.c \
		sim-db-insert.c \
		sim-correlation.c \
		sim-role.c \
		sim-uuid.c	\
		sim-ssl.c \
		sim-server-api.c \
		sim-user-auth.c \
		stip-fileutils.c \
		stip-mongo.c \
		main.c

ossim_server_LDADD = $(LIBGDA_LIBS) $(GNET_LIBS) $(GLIB_LIBS) $(GIO_LIBS) $(OPENSSL_LIBS) $(LIBUUID_LIBS) $(LIBPCRE_LIBS) $(LIBTIDY_LIBS) $(JSON_GLIB_LIBS) $(LIBGEOIP_LIBS) $(LIBSOUP_LIBS) $(LIBBSON_LIBS) $(LIBMONGOC_LIBS) $(LIBZLIB_LIBS) -lm libparser.a

SUBDIRS = tests


DISTCLEANFILES = Makefile.in
