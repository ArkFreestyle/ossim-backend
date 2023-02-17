#include "stip-mongo.h"
#include <glib.h>

mongoc_uri_t * stip_mongo_get_uri(const char* uri_string, bson_error_t *error)
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

mongoc_client_t * stip_mongo_get_client(mongoc_uri_t *uri, bson_error_t *error)
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

void stip_mongo_insert_alarm(const char *database_name, const char *collection_name, SimEvent *event)
{
    mongoc_uri_t *uri;
    mongoc_client_t *client;
    mongoc_database_t *database;
    mongoc_collection_t *collection;
    bson_t *alarm_insert_bson;
    bson_error_t error;
    uri = stip_mongo_get_uri("mongodb://localhost:27017", &error);
    client = stip_mongo_get_client(uri, &error);
    database = mongoc_client_get_database (client, database_name);
    collection = mongoc_client_get_collection (client, database_name, collection_name);
    alarm_insert_bson = stip_mongo_get_alarm_bson(event);
    
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

bson_t * stip_mongo_get_alarm_bson(SimEvent *event)
{
    /*
    MySql alarm table columns:
    1)  backlog_id: binary(16)
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

    For MongoDB, the document will look like:
    {
        "_id" : ObjectId("63ef5129c40e9e23466f4112"),
        "event_id" : "0x27e0a330aeaa11ed994d08007556c36e",
        "backlog_id" : "0x27e0a330aeaa11ed994d080073f71082",
        "corr_engine_ctx" : "0x8ab53e6e96ef11ed8df8080027e0a330",
        "timestamp" : ISODate("2023-02-17T10:04:25Z"),
        "plugin_id" : 1505,
        "plugin_sid" : 500001,
        "protocol" : 6,
        "src_ip" : "172.16.16.230",
        "dst_ip" : "172.16.16.15",
        "src_port" : 0,
        "dst_port" : 0,
        "risk" : 1,
        "efr" : 48,
        "similar" : "IF(''<>'','',SHA1('0x27e0a330aeaa11ed994d08007556c36e'))",
        "removable" : false,
        "stats" : "{\"events\":6,\"src\":{\"ip\":{\"172.16.16.230\":{\"count\":6,\"rep\":0,\"country\":\"--\",\"uuid\":\"0x27e0a33096f111eda65d0800c5fe9830\"}},\"port\":{\"0\":6},\"rep\":0,\"country\":{\"--\":6}},\"dst\":{\"ip\":{\"172.16.16.15\":{\"count\":6,\"rep\":0,\"country\":\"--\",\"uuid\":\"0xc488665896ef11ed8df8080027e0a330\"}},\"port\":{\"0\":6},\"rep\":0,\"country\":{\"--\":6}}}"
    }
    */

    bson_t *alarm_bson = bson_new();

    GString *similar = g_string_new ("");
    g_string_printf(similar, "IF('%s'<>'','%s',SHA1('%s'))", (event->groupalarmsha1 != NULL ? event->groupalarmsha1 : ""), (event->groupalarmsha1 != NULL ? event->groupalarmsha1 : ""), sim_uuid_get_db_string (event->id));
    
    bool _removable = TRUE; 
    if (event->plugin_id == 1505)
    {
        _removable = FALSE;
    }
    
    BSON_APPEND_UTF8(alarm_bson, "event_id", sim_uuid_get_db_string(event->id));
    BSON_APPEND_UTF8(alarm_bson, "backlog_id", sim_uuid_get_db_string(event->backlog_id));
    BSON_APPEND_UTF8(alarm_bson, "corr_engine_ctx", sim_uuid_get_db_string(sim_engine_get_id(event->engine)));
    BSON_APPEND_TIME_T(alarm_bson, "timestamp", event->time);
    BSON_APPEND_INT32(alarm_bson, "plugin_id", event->plugin_id);
    BSON_APPEND_INT32(alarm_bson, "plugin_sid", event->plugin_sid);
    BSON_APPEND_INT32(alarm_bson, "protocol", event->protocol);
    BSON_APPEND_UTF8(alarm_bson, "src_ip", sim_inet_get_canonical_name(event->src_ia));
    BSON_APPEND_UTF8(alarm_bson, "dst_ip", sim_inet_get_canonical_name (event->dst_ia));
    BSON_APPEND_INT32(alarm_bson, "src_port", event->src_port);
    BSON_APPEND_INT32(alarm_bson, "dst_port", event->dst_port);
    BSON_APPEND_INT32(alarm_bson, "risk", ((gint)event->risk_a > (gint)event->risk_c) ? (gint)event->risk_a : (gint)event->risk_c);
    BSON_APPEND_INT32(alarm_bson, "efr", event->priority * event->reliability * 2);
    BSON_APPEND_UTF8(alarm_bson, "similar", similar->str);
    BSON_APPEND_BOOL(alarm_bson, "removable", _removable);
    BSON_APPEND_UTF8(alarm_bson, "stats", event->alarm_stats);

    // alarm_bson_json = bson_as_canonical_extended_json (alarm_bson, NULL);
    // printf("\nalarm_bson_json: %s\n", alarm_bson_json);

    g_string_free(similar, TRUE);
    return alarm_bson;
}
