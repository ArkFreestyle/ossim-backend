#include "stip-mongo.h"
#include <glib.h>

mongoc_uri_t * stip_get_mongo_uri(const char* uri_string, bson_error_t *error)
{

    // const char *uri_string = "mongodb://localhost:27017";
    mongoc_uri_t *uri;

    mongoc_init ();
    uri = mongoc_uri_new_with_error (uri_string, &(*error));
    if (!uri) 
    {
        g_message("%s: Failed to parse URI:%s, err:%s", __func__, uri_string, error->message);
    }

    return uri;
}

mongoc_client_t * stip_get_mongo_client(mongoc_uri_t *uri, bson_error_t *error)
{
    mongoc_client_t *client;
    client = mongoc_client_new_from_uri (uri);
    if (!client)
    {
        g_message("%s: Failed to get client, err:%s", __func__, error->message);
    }

    /*
    * Register the application name so we can track it in the profile logs
    * on the server. This can also be done from the URI (see other examples).
    */
    mongoc_client_set_appname (client, "alienvault-mongo-client");
    
    return client;
}

void stip_insert_alarm_into_mongodb(const char *database_name, const char *collection_name, SimEvent *event)
{
    mongoc_uri_t *uri;
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
    bson_t *alarm_insert_bson;
    bson_error_t error;
    uri = stip_get_mongo_uri("mongodb://localhost:27017", &error);
    client = stip_get_mongo_client(uri, &error);
    database = mongoc_client_get_database (client, database_name);
    collection = mongoc_client_get_collection (client, database_name, collection_name);
    alarm_insert_bson = stip_get_alarm_bson(event);
    
    if (!mongoc_collection_insert_one (collection, alarm_insert_bson, NULL, NULL, &error)) 
    {
        g_message("%s: Failed to insert, err:%s", __func__, error.message);
    }

    // Clean up
    bson_destroy (alarm_insert_bson);
    mongoc_collection_destroy (collection);
    mongoc_database_destroy (database);
    mongoc_uri_destroy (uri);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
}

bson_t * stip_get_alarm_bson(SimEvent *event)
{
    /*
    MySql alarm table columns:
    1)  backlog_id: binary(16) --- 
    2)  event_id: binary(16)
    3)  corr_engine_ctx: binary(16) 
    4)  timestamp: timestamp
    5)  status: enum('open', 'closed')
    6)  plugin_id: int(11)
    7)  plugin_sid: int(11)
    8)  procotol: int(11)
    9)  src_ip: varbinary(16)
    10) dst_ip: varbinary(16)
    11) src_port: int(11)
    12) dst_port: int(11)
    13) risk: int(11)
    14) efr: int(11)
    15) similar: varchar(40)
    16) stats: mediumtext
    17) removable: tinyint(1)
    18) in_file: tinyint(1)

    MySql insert statement:
    REPLACE INTO alarm (event_id, backlog_id, corr_engine_ctx, timestamp, plugin_id, plugin_sid, protocol, src_ip, dst_ip, src_port, dst_port, risk, efr, similar, removable, stats) 
    VALUES (0x27e0a330a6e411ed81f40800f0e5688c,0x27e0a330a6e411ed81f40800effd5ca4,0x8ab53e6e96ef11ed8df8080027e0a330,'2023-02-07 12:42:54',1505,500001,6,0xac1010e6,0xac10100f,0,0,1,48,IF(''<>'','',SHA1('0x27e0a330a6e411ed81f40800f0e5688c')),0,'{"events":6,"src":{"ip":{"172.16.16.230":{"count":6,"rep":0,"country":"--","uuid":"0x27e0a33096f111eda65d0800c5fe9830"}},"port":{"0":6},"rep":0,"country":{"--":6}},"dst":{"ip":{"172.16.16.15":{"count":6,"rep":0,"country":"--","uuid":"0xc488665896ef11ed8df8080027e0a330"}},"port":{"0":6},"rep":0,"country":{"--":6}}}')
   
    Hardcoded alarm table equivalent BSON format:
    stats:
    {
    "events": 6,
    "src": {
    "ip": {
        "172.16.16.230": {
        "count": 6,
        "rep": 0,
        "country": "--",
        "uuid": "0x27e0a33096f111eda65d0800c5fe9830"
        }
    },
    "port": {
        "0": 6
    },
    "rep": 0,
    "country": {
        "--": 6
    }
    },
    "dst": {
    "ip": {
        "172.16.16.15": {
        "count": 6,
        "rep": 0,
        "country": "--",
        "uuid": "0xc488665896ef11ed8df8080027e0a330"
        }
    },
    "port": {
        "0": 6
    },
    "rep": 0,
    "country": {
        "--": 6
    }
    }
    }

    Note: I'm going to replace the . (dots) in ips to d. This is because the version of mongodb I installed does not support dots inside keys.
   
    insert = BCON_NEW (
        "event_id", BCON_UTF8 ("0x27e0a330a6e411ed81f40800f0e5688c"),
        "backlog_id", BCON_UTF8 ("0x27e0a330a6e411ed81f40800effd5ca4"),
        "corr_engine_ctx", BCON_UTF8("0x8ab53e6e96ef11ed8df8080027e0a330"),
        "timestamp", BCON_UTF8("2023-02-07 12:42:54"),
        "plugin_id", BCON_UTF8("1505"),
        "plugin_sid", BCON_UTF8("500001"),
        "protocol", BCON_UTF8("6"),
        "src_ip", BCON_UTF8("0xac1010e6"),
        "dst_ip", BCON_UTF8("0xac10100f"),
        "src_port", BCON_UTF8("0"),
        "dst_port", BCON_UTF8("0"),
        "risk", BCON_UTF8("1"),
        "efr", BCON_UTF8("48"),
        "similar", BCON_UTF8("IF(''<>'','',SHA1('0x27e0a330a6e411ed81f40800f0e5688c'))"),
        "removable", BCON_UTF8("0"),
        "stats", "{",
        "events", "6",
        "src", "{",
        "ip", "{",
        "172d16d16d230", "{",
        "count", "6",
        "rep", "0",
        "country", "--",
        "uuid", "0x27e0a33096f111eda65d0800c5fe9830",
        "}",
        "}",
        "port", "{",
        "0", "6",
        "}",
        "rep", "0",
        "country", "{",
        "--", "6",
        "}",
        "}",
        "dst", "{",
        "ip", "{",
        "172d16d16d15", "{",
        "count", "6",
        "rep", "0",
        "country", "--",
        "uuid", "0xc488665896ef11ed8df8080027e0a330",
        "}",
        "}",
        "port", "{",
        "0", "6",
        "}",
        "rep", "0",
        "country", "{",
        "--", "6",
        "}",
        "}",
        "}");
    */
    bson_t *insert;
    
    GString *event_id = g_string_new ("");
    g_string_printf(event_id, "%s", sim_uuid_get_db_string(event->id));

    GString *backlog_id = g_string_new ("");
    g_string_printf(backlog_id, "%s", sim_uuid_get_db_string(event->backlog_id));

    GString *corr_engine_ctx = g_string_new ("");
    g_string_printf(corr_engine_ctx, "%s", sim_uuid_get_db_string(sim_engine_get_id(event->engine)));

    GString *timestamp = g_string_new ("");
    g_string_printf(timestamp, "%ld", event->time);

    GString *plugin_id = g_string_new ("");
    g_string_printf(plugin_id, "%d", event->plugin_id);

    GString *plugin_sid = g_string_new ("");
    g_string_printf(plugin_sid, "%d", event->plugin_sid);

    GString *protocol = g_string_new ("");
    g_string_printf(protocol, "%d", event->protocol);

    GString *src_ip = g_string_new ("");
    g_string_printf(src_ip, "%s", sim_inet_get_db_string(event->src_ia));

    GString *dst_ip = g_string_new ("");
    g_string_printf(dst_ip, "%s", sim_inet_get_db_string(event->dst_ia));

    GString *src_port = g_string_new ("");
    g_string_printf(src_port, "%d", event->src_port);

    GString *dst_port = g_string_new ("");
    g_string_printf(dst_port, "%d", event->dst_port);

    GString *risk = g_string_new ("");
    g_string_printf(risk, "%d", ((gint)event->risk_a > (gint)event->risk_c) ? (gint)event->risk_a : (gint)event->risk_c);

    GString *efr = g_string_new ("");
    g_string_printf(efr, "%d", event->priority * event->reliability * 2);

    GString *similar = g_string_new ("");
    g_string_printf(similar, "IF('%s'<>'','%s',SHA1('%s'))", (event->groupalarmsha1 != NULL ? event->groupalarmsha1 : ""), (event->groupalarmsha1 != NULL ? event->groupalarmsha1 : ""), sim_uuid_get_db_string (event->id));

    GString *removable = g_string_new ("");
    bool _removable = TRUE; 
    if (event->plugin_id == 1505)
    {
    _removable = FALSE;
    }
    g_string_printf(removable, "%d", _removable);

    GString *stats = g_string_new ("");
    g_string_printf(stats, "%s", event->alarm_stats);

    insert = BCON_NEW (
        "event_id", BCON_UTF8 (event_id->str),
        "backlog_id", BCON_UTF8 (backlog_id->str),
        "corr_engine_ctx", BCON_UTF8(corr_engine_ctx->str),
        "timestamp", BCON_UTF8(timestamp->str),
        "plugin_id", BCON_UTF8(plugin_id->str),
        "plugin_sid", BCON_UTF8(plugin_sid->str),
        "protocol", BCON_UTF8(protocol->str),
        "src_ip", BCON_UTF8(src_ip->str),
        "dst_ip", BCON_UTF8(dst_ip->str),
        "src_port", BCON_UTF8(src_port->str),
        "dst_port", BCON_UTF8(dst_port->str),
        "risk", BCON_UTF8(risk->str),
        "efr", BCON_UTF8(efr->str),
        "similar", BCON_UTF8(similar->str),
        "removable", BCON_UTF8(removable->str),
        "stats", BCON_UTF8(stats->str)
        );

    g_string_free(event_id, TRUE);
    g_string_free(backlog_id, TRUE);
    g_string_free(corr_engine_ctx, TRUE);
    g_string_free(timestamp, TRUE);
    g_string_free(plugin_id, TRUE);
    g_string_free(plugin_sid, TRUE);
    g_string_free(protocol, TRUE);
    g_string_free(src_ip, TRUE);
    g_string_free(dst_ip, TRUE);
    g_string_free(src_port, TRUE);
    g_string_free(dst_port, TRUE);
    g_string_free(risk, TRUE);
    g_string_free(efr, TRUE);
    g_string_free(similar, TRUE);
    g_string_free(removable, TRUE);
    g_string_free(stats, TRUE);

    return insert;

    
}

int stip_write_alarm_to_mongodb(gchar* insert_statement, SimEvent *event)
{
  const char *uri_string = "mongodb://localhost:27017";
   mongoc_uri_t *uri;
   mongoc_client_t *client;
   mongoc_database_t *database;
   mongoc_collection_t *collection;
   bson_t *command, reply, *insert;
   bson_error_t error;
   char *str;
   bool retval;

   /*
    * Required to initialize libmongoc's internals
    */
   mongoc_init ();

   /*
    * Safely create a MongoDB URI object from the given string
    */
   uri = mongoc_uri_new_with_error (uri_string, &error);
   if (!uri) {
      fprintf (stderr,
               "failed to parse URI: %s\n"
               "error message:       %s\n",
               uri_string,
               error.message);
      return EXIT_FAILURE;
   }

   /*
    * Create a new client instance
    */
   client = mongoc_client_new_from_uri (uri);
   if (!client) {
      return EXIT_FAILURE;
   }

   /*
    * Register the application name so we can track it in the profile logs
    * on the server. This can also be done from the URI (see other examples).
    */
   mongoc_client_set_appname (client, "connect-example");

   /*
    * Get a handle on the database "ark_database" and collection "ark_column"
    */
   database = mongoc_client_get_database (client, "ark_database");
   collection = mongoc_client_get_collection (client, "ark_database", "ark_column");

   /*
    * Do work. This example pings the database, prints the result as JSON and
    * performs an insert
    */
   command = BCON_NEW ("ping", BCON_INT32 (1));

   retval = mongoc_client_command_simple (
      client, "admin", command, NULL, &reply, &error);

   if (!retval) {
      fprintf (stderr, "%s\n", error.message);
      return EXIT_FAILURE;
   }

   str = bson_as_json (&reply, NULL);
   printf ("%s\n", str);

   insert = BCON_NEW ("hello", BCON_UTF8 ("from inside alienvault!!!"));

   if (!mongoc_collection_insert_one (collection, insert, NULL, NULL, &error)) {
      fprintf (stderr, "%s\n", error.message);

      fprintf (stderr, "%s\n", insert_statement);
   }

   /*
   Attempting to create an alarm table equivalent BSON format:
   Table columns:
   1)  backlog_id: binary(16) --- 
   2)  event_id: binary(16)
   3)  corr_engine_ctx: binary(16) 
   4)  timestamp: timestamp
   5)  status: enum('open', 'closed')
   6)  plugin_id: int(11)
   7)  plugin_sid: int(11)
   8)  procotol: int(11)
   9)  src_ip: varbinary(16)
   10) dst_ip: varbinary(16)
   11) src_port: int(11)
   12) dst_port: int(11)
   13) risk: int(11)
   14) efr: int(11)
   15) similar: varchar(40)
   16) stats: mediumtext
   17) removable: tinyint(1)
   18) in_file: tinyint(1)

   REPLACE INTO alarm (event_id, backlog_id, corr_engine_ctx, timestamp, plugin_id, plugin_sid, protocol, src_ip, dst_ip, src_port, dst_port, risk, efr, similar, removable, stats) 
   VALUES (0x27e0a330a6e411ed81f40800f0e5688c,0x27e0a330a6e411ed81f40800effd5ca4,0x8ab53e6e96ef11ed8df8080027e0a330,'2023-02-07 12:42:54',1505,500001,6,0xac1010e6,0xac10100f,0,0,1,48,IF(''<>'','',SHA1('0x27e0a330a6e411ed81f40800f0e5688c')),0,'{"events":6,"src":{"ip":{"172.16.16.230":{"count":6,"rep":0,"country":"--","uuid":"0x27e0a33096f111eda65d0800c5fe9830"}},"port":{"0":6},"rep":0,"country":{"--":6}},"dst":{"ip":{"172.16.16.15":{"count":6,"rep":0,"country":"--","uuid":"0xc488665896ef11ed8df8080027e0a330"}},"port":{"0":6},"rep":0,"country":{"--":6}}}')
   
   stats:
   {
  "events": 6,
  "src": {
    "ip": {
      "172.16.16.230": {
        "count": 6,
        "rep": 0,
        "country": "--",
        "uuid": "0x27e0a33096f111eda65d0800c5fe9830"
      }
    },
    "port": {
      "0": 6
    },
    "rep": 0,
    "country": {
      "--": 6
    }
  },
  "dst": {
    "ip": {
      "172.16.16.15": {
        "count": 6,
        "rep": 0,
        "country": "--",
        "uuid": "0xc488665896ef11ed8df8080027e0a330"
      }
    },
    "port": {
      "0": 6
    },
    "rep": 0,
    "country": {
      "--": 6
    }
  }
}

Note: I'm going to replace the . (dots) in ips to d. This is because the version of mongodb I installed does not support dots inside keys.
   */

  //  insert = BCON_NEW (
  //     "event_id", BCON_UTF8 ("0x27e0a330a6e411ed81f40800f0e5688c"),
  //     "backlog_id", BCON_UTF8 ("0x27e0a330a6e411ed81f40800effd5ca4"),
  //     "corr_engine_ctx", BCON_UTF8("0x8ab53e6e96ef11ed8df8080027e0a330"),
  //     "timestamp", BCON_UTF8("2023-02-07 12:42:54"),
  //     "plugin_id", BCON_UTF8("1505"),
  //     "plugin_sid", BCON_UTF8("500001"),
  //     "protocol", BCON_UTF8("6"),
  //     "src_ip", BCON_UTF8("0xac1010e6"),
  //     "dst_ip", BCON_UTF8("0xac10100f"),
  //     "src_port", BCON_UTF8("0"),
  //     "dst_port", BCON_UTF8("0"),
  //     "risk", BCON_UTF8("1"),
  //     "efr", BCON_UTF8("48"),
  //     "similar", BCON_UTF8("IF(''<>'','',SHA1('0x27e0a330a6e411ed81f40800f0e5688c'))"),
  //     "removable", BCON_UTF8("0"),
  //     "stats", "{",
  //     "events", "6",
  //     "src", "{",
  //     "ip", "{",
  //     "172d16d16d230", "{",
  //     "count", "6",
  //     "rep", "0",
  //     "country", "--",
  //     "uuid", "0x27e0a33096f111eda65d0800c5fe9830",
  //     "}",
  //     "}",
  //     "port", "{",
  //     "0", "6",
  //     "}",
  //     "rep", "0",
  //     "country", "{",
  //     "--", "6",
  //     "}",
  //     "}",
  //     "dst", "{",
  //     "ip", "{",
  //     "172d16d16d15", "{",
  //     "count", "6",
  //     "rep", "0",
  //     "country", "--",
  //     "uuid", "0xc488665896ef11ed8df8080027e0a330",
  //     "}",
  //     "}",
  //     "port", "{",
  //     "0", "6",
  //     "}",
  //     "rep", "0",
  //     "country", "{",
  //     "--", "6",
  //     "}",
  //     "}",
  //     "}");

//   void         g_string_printf            (GString         *string,
//                                          const gchar     *format,
//                                          ...) G_GNUC_PRINTF (2, 3);
// GLIB_AVAILABLE_IN_ALL
// void         g_string_append_vprintf    (GString         *string,
//                                          const gchar     *format,
//                                          va_list          args)
//                                          G_GNUC_PRINTF(2, 0);
// GLIB_AVAILABLE_IN_ALL
// void         g_string_append_printf     (GString         *string,
//                                          const gchar     *format,
//                                          ...) G_GNUC_PRINTF (2, 3);
  // g_string_printf (key, "plugin_id=%u|plugin_sid=%u", event->plugin_id, event->plugin_sid);
  
  GString *event_id = g_string_new ("");
  g_string_printf(event_id, "%s", sim_uuid_get_db_string(event->id));
  
  GString *backlog_id = g_string_new ("");
  g_string_printf(backlog_id, "%s", sim_uuid_get_db_string(event->backlog_id));

  GString *corr_engine_ctx = g_string_new ("");
  g_string_printf(corr_engine_ctx, "%s", sim_uuid_get_db_string(sim_engine_get_id(event->engine)));

  GString *timestamp = g_string_new ("");
  g_string_printf(timestamp, "%ld", event->time);

  GString *plugin_id = g_string_new ("");
  g_string_printf(plugin_id, "%d", event->plugin_id);

  GString *plugin_sid = g_string_new ("");
  g_string_printf(plugin_sid, "%d", event->plugin_sid);

  GString *protocol = g_string_new ("");
  g_string_printf(protocol, "%d", event->protocol);

  GString *src_ip = g_string_new ("");
  g_string_printf(src_ip, "%s", sim_inet_get_db_string(event->src_ia));

  GString *dst_ip = g_string_new ("");
  g_string_printf(dst_ip, "%s", sim_inet_get_db_string(event->dst_ia));

  GString *src_port = g_string_new ("");
  g_string_printf(src_port, "%d", event->src_port);

  GString *dst_port = g_string_new ("");
  g_string_printf(dst_port, "%d", event->dst_port);

  GString *risk = g_string_new ("");
  g_string_printf(risk, "%d", ((gint)event->risk_a > (gint)event->risk_c) ? (gint)event->risk_a : (gint)event->risk_c);

  GString *efr = g_string_new ("");
  g_string_printf(efr, "%d", event->priority * event->reliability * 2);

  GString *similar = g_string_new ("");
  g_string_printf(similar, "IF('%s'<>'','%s',SHA1('%s'))", (event->groupalarmsha1 != NULL ? event->groupalarmsha1 : ""), (event->groupalarmsha1 != NULL ? event->groupalarmsha1 : ""), sim_uuid_get_db_string (event->id));

  GString *removable = g_string_new ("");
  bool _removable = TRUE; 
  if (event->plugin_id == 1505)
  {
    _removable = FALSE;
  }
  g_string_printf(removable, "%d", _removable);
  
  GString *stats = g_string_new ("");
  g_string_printf(stats, "%s", event->alarm_stats);

  insert = BCON_NEW (
      "event_id", BCON_UTF8 (event_id->str),
      "backlog_id", BCON_UTF8 (backlog_id->str),
      "corr_engine_ctx", BCON_UTF8(corr_engine_ctx->str),
      "timestamp", BCON_UTF8(timestamp->str),
      "plugin_id", BCON_UTF8(plugin_id->str),
      "plugin_sid", BCON_UTF8(plugin_sid->str),
      "protocol", BCON_UTF8(protocol->str),
      "src_ip", BCON_UTF8(src_ip->str),
      "dst_ip", BCON_UTF8(dst_ip->str),
      "src_port", BCON_UTF8(src_port->str),
      "dst_port", BCON_UTF8(dst_port->str),
      "risk", BCON_UTF8(risk->str),
      "efr", BCON_UTF8(efr->str),
      "similar", BCON_UTF8(similar->str),
      "removable", BCON_UTF8(removable->str),
      "stats", BCON_UTF8(stats->str)
      );

   if (!mongoc_collection_insert_one (collection, insert, NULL, NULL, &error)) {
      fprintf (stderr, "%s\n", error.message);
   }

   g_string_free(event_id, TRUE);
   g_string_free(backlog_id, TRUE);
   g_string_free(corr_engine_ctx, TRUE);
   g_string_free(timestamp, TRUE);
   g_string_free(plugin_id, TRUE);
   g_string_free(plugin_sid, TRUE);
   g_string_free(protocol, TRUE);
   g_string_free(src_ip, TRUE);
   g_string_free(dst_ip, TRUE);
   g_string_free(src_port, TRUE);
   g_string_free(dst_port, TRUE);
   g_string_free(risk, TRUE);
   g_string_free(efr, TRUE);
   g_string_free(similar, TRUE);
   g_string_free(removable, TRUE);
   g_string_free(stats, TRUE);

   bson_destroy (insert);
   bson_destroy (&reply);
   bson_destroy (command);
   bson_free (str);

   /*
    * Release our handles and clean up libmongoc
    */
   mongoc_collection_destroy (collection);
   mongoc_database_destroy (database);
   mongoc_uri_destroy (uri);
   mongoc_client_destroy (client);
   mongoc_cleanup ();

   return 0;
}
